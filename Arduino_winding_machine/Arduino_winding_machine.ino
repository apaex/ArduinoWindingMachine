/* Name: Winding machine
   Description: Arduino ATmega 328P + Stepper motor control CNC Shield v3 + 2004 LCD + Encoder KY-040

       Arduino pinout diagram:
          _______________
         |      USB      |
         |           AREF|
         |            GND|
         |             13| DIR A
         |RESET        12| STEP A
         |3V3         #11| STOP BT
         |5V          #10| BUZ OUT
         |GND          #9|
         |VIN           8| EN STEP
         |               |
         |              7| DIR Z
         |             #6|
  LCD RS |A0 14        #5| ENCODER CLK
  LCD EN |A1 15         4| STEP Z
  LCD D4 |A2 16   INT1 #3| ENCODER SW
  LCD D5 |A3 17   INT0  2| ENCODER DT
  LCD D6 |A4 18      TX 1|
  LCD D7 |A5 19      RX 0|
         |__A6_A7________|


https://cxem.net/arduino/arduino235.php
https://cxem.net/arduino/arduino245.php

*/

#include "config.h"  // все настройки железа здесь
#include "debug.h"

#define FPSTR(pstr) (const __FlashStringHelper *)(pstr)
#define LENGTH(a) (sizeof(a) / sizeof(*a))

#if DISPLAY_I2C == 1
#include <LiquidCrystal_I2C.h>
#else
#include <LiquidCrystal.h>
#endif
#include <EncButton.h>
#include <GyverPlanner.h>
#include <GyverStepper2.h>
#include "LiquidCrystalCyr.h"
#include "Menu.h"
#include "timer.h"
#include "buzzer.h"

#include "Screen.h"
#include "Winding.h"
#include "strings.h"

#define STEPPER_STEPS_COUNT (int32_t(STEPPER_STEPS) * STEPPER_MICROSTEPS)
#ifdef GS_FAST_PROFILE
#define SPEED_LIMIT 600
#else
#define SPEED_LIMIT 260
#endif
#define SPEED_INC 10
#define EEPROM_SETTINGS_VERSION 2
#define EEPROM_WINDINGS_VERSION 2
#define EEPROM_SETTINGS_ADDR 0x00
#define EEPROM_WINDINGS_ADDR 0x10
#define EEPROM_WINDINGS_CLASTER (sizeof(Winding) * WINDING_COUNT + 1)
#ifndef TRANSFORMER_COUNT
#define TRANSFORMER_COUNT 3
#endif
#define WINDING_COUNT 3
#ifndef STEPPER_Z_REVERSE
#define STEPPER_Z_REVERSE 0
#endif
#ifndef STEPPER_A_REVERSE
#define STEPPER_A_REVERSE 0
#endif

Winding params[WINDING_COUNT];

int8_t currentWinding = 0;

Settings settings;

enum menu_states {
  Autowinding1,
  CurrentTrans,
  PosControl,
  miSettings,
  Winding1,
  Winding2,
  Winding3,
  StartAll,
  WindingBack,
  TurnsSet,
  LaySet,
  StepSet,
  SpeedSet,
  Direction,
  Start,
  Cancel,
  ShaftPos,
  ShaftStepMul,
  LayerPos,
  LayerStepMul,
  PosCancel,
  miSettingsStopPerLevel,
  AccelSet,
  miSettingsBack
};  // Нумерованный список строк экрана

const char *boolSet[] = { STRING_OFF, STRING_ON };
const char *dirSet[] = { "<<<", ">>>" };
const uint8_t *stepSet[] = { 1, 10, 100 };

MenuItem *menuItems[] = {
  new MenuItem(0, 0, MENU_01),
  new ByteMenuItem(0, 1, MENU_02, MENU_FORMAT_02, &settings.currentTransformer, 1, TRANSFORMER_COUNT),
  new MenuItem(0, 2, MENU_04),
  new MenuItem(0, 3, MENU_05),

  new ValMenuItem(1, 0, MENU_06, MENU_FORMAT_06),
  new ValMenuItem(1, 1, MENU_07, MENU_FORMAT_06),
  new ValMenuItem(1, 2, MENU_08, MENU_FORMAT_06),
  new MenuItem(1, 3, MENU_15),
  new MenuItem(1, 4, MENU_09),

  new IntMenuItem(2, 0, MENU_10, MENU_FORMAT_10, NULL, 1, 999),
  new IntMenuItem(2, 1, MENU_13, MENU_FORMAT_13, NULL, 1, 99),
  new FloatMenuItem(2, 2, MENU_11, MENU_FORMAT_11, NULL, 5, 9995, 5),
  new IntMenuItem(2, 3, MENU_12, MENU_FORMAT_10, NULL, SPEED_INC, SPEED_LIMIT, SPEED_INC),
  new BoolMenuItem(2, 4, MENU_14, NULL, dirSet),
  new MenuItem(2, 5, MENU_15),
  new MenuItem(2, 6, MENU_09),

  new IntMenuItem(10, 0, MENU_17, MENU_FORMAT_17, &settings.shaftPos, -999, 999),
  new SetMenuItem(10, 1, MENU_18, MENU_FORMAT_10, &settings.shaftStep, stepSet, 3),
  new IntMenuItem(10, 2, MENU_19, MENU_FORMAT_17, &settings.layerPos, -999, 999),
  new SetMenuItem(10, 3, MENU_18, MENU_FORMAT_10, &settings.layerStep, stepSet, 3),
  new MenuItem(10, 4, MENU_09),

  new BoolMenuItem(11, 0, MENU_22, &settings.stopPerLayer, boolSet),
  new IntMenuItem(11, 1, MENU_23, MENU_FORMAT_10, &settings.acceleration, 0, 600, 1),
  new MenuItem(11, 2, MENU_09),
};

byte up[8] = { 0b00100, 0b01110, 0b11111, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000 };    // Создаем свой символ ⯅ для LCD
byte down[8] = { 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b11111, 0b01110, 0b00100 };  // Создаем свой символ ⯆ для LCD

#if DISPLAY_I2C == 0
LiquidCrystalCyr lcd(DISPLAY_RS, DISPLAY_EN, DISPLAY_D4, DISPLAY_D5, DISPLAY_D6, DISPLAY_D7);  // Назначаем пины для управления LCD
#else
LiquidCrystalCyr lcd(DISPLAY_ADDRESS, DISPLAY_NCOL, DISPLAY_NROW);
#endif

MainMenu menu(menuItems, LENGTH(menuItems), lcd);
MainScreen screen(lcd);


GStepper2<STEPPER2WIRE> shaftStepper(STEPPER_STEPS_COUNT, STEPPER_STEP_Z, STEPPER_DIR_Z, STEPPER_EN);
GStepper2<STEPPER2WIRE> layerStepper(STEPPER_STEPS_COUNT, STEPPER_STEP_A, STEPPER_DIR_A, STEPPER_EN);
GPlanner<STEPPER2WIRE, 2> planner;

EncButton<EB_TICK, ENCODER_CLK, ENCODER_DT, ENCODER_SW> encoder(ENCODER_INPUT);
EncButton<EB_TICK, BUTTON_STOP> pedal;

Buzzer buzzer(BUZZER);

void setup() {
  Serial.begin(9600);
  LoadSettings();

  layerStepper.disable();
  shaftStepper.disable();
  layerStepper.reverse(STEPPER_Z_REVERSE);
  shaftStepper.reverse(STEPPER_A_REVERSE);
  planner.addStepper(0, shaftStepper);
  planner.addStepper(1, layerStepper);

  lcd.createChar(0, up);                  // Записываем символ ⯅ в память LCD
  lcd.createChar(1, down);                // Записываем символ ⯆ в память LCD
  lcd.begin(DISPLAY_NCOL, DISPLAY_NROW);  // Инициализация LCD Дисплей
  menu.Draw();

  encoder.setEncType(ENCODER_TYPE);
}

void loop() {
  encoder.tick();

  if (encoder.turn()) {
    menu.IncIndex(encoder.dir());  // Если позиция энкодера изменена, то меняем menu.index и выводим экран
    menu.Draw();
  }

  if (encoder.click()) {
    switch (menu.index)  // Если было нажатие, то выполняем действие, соответствующее текущей позиции курсора
    {
      case Autowinding1:
        SaveSettings();
        LoadWindings();
        menu.index = Winding1;

        UpdateMenuItemText(0);
        UpdateMenuItemText(1);
        UpdateMenuItemText(2);
        break;

      case Winding1:
      case Winding2:
      case Winding3:
        currentWinding = menu.index - Winding1;
        menu.index = TurnsSet;
        ((IntMenuItem *)menu[TurnsSet])->value = &params[currentWinding].turns;
        ((IntMenuItem *)menu[StepSet])->value = &params[currentWinding].step;
        ((IntMenuItem *)menu[SpeedSet])->value = &params[currentWinding].speed;
        ((IntMenuItem *)menu[LaySet])->value = &params[currentWinding].layers;
        ((BoolMenuItem *)menu[Direction])->value = &params[currentWinding].dir;
        break;
      case WindingBack:
        menu.index = Autowinding1;
        break;
      case PosControl:
        menu.index = ShaftPos;
        break;
      case TurnsSet:
      case StepSet:
      case SpeedSet:
      case LaySet:
      case AccelSet:
        ValueEdit();
        break;
      case CurrentTrans:
      case miSettingsStopPerLevel:
      case Direction:
        menu.IncCurrent(1, true);
        break;
      case StartAll:
        AutoWindingAll(params, WINDING_COUNT);
        menu.Draw(true);
        break;
      case Start:
        SaveWindings();
        AutoWindingAll(params + currentWinding, 1);
        menu.index = Winding1 + currentWinding;
        UpdateMenuItemText(currentWinding);
        break;
      case Cancel:
        SaveWindings();
        menu.index = Winding1 + currentWinding;
        UpdateMenuItemText(currentWinding);
        break;

      case ShaftPos:
      case LayerPos:
        MoveTo((menu.index == LayerPos) ? layerStepper : shaftStepper, *((IntMenuItem *)menu[menu.index])->value);
        break;

      case ShaftStepMul:
      case LayerStepMul:
        menu.IncCurrent(1, true);
        ((IntMenuItem *)menu[menu.index - 1])->increment = *((SetMenuItem *)menu[menu.index])->value;
        break;
      case PosCancel:
        menu.index = PosControl;
        settings.shaftPos = 0;
        settings.layerPos = 0;
        break;

      case miSettings:
        menu.index = miSettingsStopPerLevel;
        break;

      case miSettingsBack:
        SaveSettings();
        menu.index = miSettings;
        break;
    }
    menu.Draw();
  }
}

void UpdateMenuItemText(byte i) {
  ((ValMenuItem *)menu[Winding1 + i])->value = params[i].turns * params[i].layers;
}

void ValueEdit() {
  menu.DrawQuotes(1);
  do {
    encoder.tick();

    if (encoder.turn() || encoder.turnH())
      menu.IncCurrent(encoder.dir() * (encoder.state() ? 10 : 1));

  } while (!encoder.click());
  menu.DrawQuotes(0);
}

void MoveTo(GStepper2<STEPPER2WIRE> &stepper, int &pos) {
  menu.DrawQuotes(1);
  stepper.enable();

  stepper.setAcceleration(STEPPER_STEPS_COUNT * settings.acceleration / 60);
  stepper.setMaxSpeed(STEPPER_STEPS_COUNT / 2);

  int oldPos = -pos * STEPPER_MICROSTEPS * 2;
  stepper.setCurrent(oldPos);
  stepper.setTarget(oldPos);

  do {
    stepper.tick();
    encoder.tick();

    int newPos = -pos * STEPPER_MICROSTEPS * 2;
    if (newPos != oldPos) {
      stepper.setTarget(newPos);
      oldPos = newPos;
    }

    if (encoder.turn())
      menu.IncCurrent(encoder.dir());

  } while (!encoder.click() || stepper.getStatus() != 0);

  stepper.disable();
  menu.DrawQuotes(0);
}

double speedMult = 1;

ISR(TIMER1_COMPA_vect) {
  if (planner.tickManual())
    setPeriod(planner.getPeriod() * speedMult);
  else
    stopTimer();
}

uint32_t getSpeed() {
  uint32_t p = planner.getPeriod() * speedMult;
  return (p == 0) ? 0 : (60000000ul / (STEPPER_STEPS_COUNT * p));
}


void AutoWinding(const Winding &w, bool &direction)  // Подпрограмма автоматической намотки
{
  Winding current;  // Текущий виток и слой при автонамотке

  DebugWrite("Start");

  current.turns = 0;
  current.layers = 0;
  current.speed = w.speed;
  speedMult = 1;
  current.dir = w.dir;
  current.step = w.step;

  screen.Draw();

  pedal.tick();
  bool run = pedal.state();  // педаль нажата - работаем

  shaftStepper.enable();  // Разрешение управления двигателями
  layerStepper.enable();

  planner.setAcceleration(STEPPER_STEPS_COUNT * settings.acceleration / 60L);
  planner.setMaxSpeed(STEPPER_STEPS_COUNT * w.speed / 60L);

  int32_t dShaft = -STEPPER_STEPS_COUNT * w.turns;
  int32_t dLayer = -STEPPER_STEPS_COUNT * w.turns * w.step / int32_t(THREAD_PITCH) * (direction ? 1 : -1);
  int32_t p[] = { dShaft, dLayer };

  planner.reset();
  initTimer();

  while (1) {
    if (planner.getStatus() == 0 && run) {
      DebugWrite("READY");
      if (current.layers >= w.layers)
        break;

      if (settings.stopPerLayer && (current.layers > 0)) {
        screen.Message(STRING_2);  // "PRESS CONTINUE  "
        WaitButton();
        screen.Draw();
      }

      DebugWrite("setTarget", p[0], p[1]);
      planner.setTarget(p, RELATIVE);
      ++current.layers;
      p[1] = -p[1];
      direction = !direction;

      startTimer();
      setPeriod(planner.getPeriod() * speedMult);

      screen.UpdateLayers(current.layers);
    }

    encoder.tick();
    pedal.tick();

    bool oldState = run;
    if (pedal.press() || pedal.release())
      run = pedal.state();
    else if (pedal.state() && encoder.click())
      run = !run;

    if (run != oldState) {
      if (run) {
        noInterrupts();
        planner.resume();
        interrupts();
        if (planner.getStatus())
        {
          startTimer();
          setPeriod(planner.getPeriod() * speedMult);
        }
      } else {
        noInterrupts();
        planner.stop();
        interrupts();
      }
    }

    if (encoder.turn()) {                                                                            // Если повернуть энкодер во время автонамотки,
      current.speed = constrain(current.speed + encoder.dir() * SPEED_INC, SPEED_INC, SPEED_LIMIT);  // то меняем значение скорости
      //planner.setMaxSpeed(STEPPER_STEPS_COUNT * current.speed / 60L);
      speedMult = double(w.speed) / double(current.speed);
      screen.UpdateSpeed(current.speed);
    }

    static uint32_t tmr;
    if (millis() - tmr >= 500) {
      tmr = millis();

      int total_turns = (abs(shaftStepper.pos)) / STEPPER_STEPS_COUNT;

      screen.UpdateTurns(total_turns % w.turns + 1);
      // DebugWrite("pos", shaftStepper.pos, layerStepper.pos);
      screen.PlannerStatus(planner.getStatus());
    }
  }

  layerStepper.disable();
  shaftStepper.disable();
}

void AutoWindingAll(const Winding windings[], byte n) {
  bool direction = windings[0].dir;

  for (byte i = 0; i < n; ++i) {
    const Winding &w = windings[i];
    if (!w.turns || !w.layers || !w.step || !w.speed) continue;

    screen.Init(w);

    if (n > 1) {
      screen.Draw();
      screen.Message(STRING_3, i + 1);
      buzzer.Multibeep(2, 200, 200);
      WaitButton();
    }

    AutoWinding(w, direction);
  }

  screen.Message(STRING_1);  // "AUTOWINDING DONE"
  buzzer.Multibeep(3, 200, 200);
  WaitButton();
}


void WaitButton() {
  do {
    encoder.tick();
  } while (!encoder.click());
}

void LoadSettings() {
  int p = EEPROM_SETTINGS_ADDR;
  byte v = 0;
  EEPROM_load(p, v);
  if (v != EEPROM_SETTINGS_VERSION)
    return;

  Load(settings, p);
  settings.currentTransformer = constrain(settings.currentTransformer, 1, TRANSFORMER_COUNT);
}

void SaveSettings() {
  int p = EEPROM_SETTINGS_ADDR;
  byte v = EEPROM_SETTINGS_VERSION;
  EEPROM_save(p, v);
  Save(settings, p);
}

void LoadWindings() {
  int p = EEPROM_WINDINGS_ADDR + (settings.currentTransformer - 1) * EEPROM_WINDINGS_CLASTER;

  byte v = 0;
  EEPROM_load(p, v);

  for (int j = 0; j < WINDING_COUNT; ++j)
    if (v == EEPROM_WINDINGS_VERSION)
      Load(params[j], p);
    else
      params[j] = Winding();
}

void SaveWindings() {
  int p = EEPROM_WINDINGS_ADDR + (settings.currentTransformer - 1) * EEPROM_WINDINGS_CLASTER;

  byte v = EEPROM_WINDINGS_VERSION;
  EEPROM_save(p, v);
  for (int j = 0; j < WINDING_COUNT; ++j)
    Save(params[j], p);
}
