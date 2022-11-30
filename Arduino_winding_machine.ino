/* Name: Winding machine    
   Description: Arduino ATmega 328P + Stepper motor control CNC Shield v3 + 2004 LCD + Encoder KY-040
   Author:      TDA
   Ver:         3.0b
   Date:        19/11/2022

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
    
#define LANGUAGE RU     // EN, RU
#define DEBUG
#include "debug.h"
#define FPSTR(pstr) (const __FlashStringHelper*)(pstr)
#define LENGTH(a) (sizeof(a) / sizeof(*a))

#include <LiquidCrystal.h>
//#include <LiquidCrystal_I2C.h>
#include <EncButton.h>
#include <GyverPlanner.h>
#include <GyverStepper2.h>
#include <HardwareSerial.h>
#include "LiquidCrystalCyr.h"
#include "Menu.h"
#include "timer.h"

#include "Screen.h"
#include "Winding.h"
#include "strings.h"
#include "config.h"   // все настройки железа здесь


#define STEPPERS_STEPS_COUNT (int32_t(STEPPERS_STEPS) * STEPPERS_MICROSTEPS)

#define EEPROM_DATA_VERSION 1

#define TRANSFORMER_COUNT 3
#define WINDING_COUNT 3

Winding params[WINDING_COUNT];

int8_t currentTransformer = -1;
int8_t currentWinding = -1;

Settings settings;

enum menu_states {Autowinding1, Autowinding2, Autowinding3, PosControl, miSettings, Winding1, Winding2, Winding3, WindingBack, TurnsSet, LaySet, StepSet, SpeedSet, Direction, Start, Cancel, ShaftPos, ShaftStepMul, LayerPos, LayerStepMul, PosCancel, miSettingsStopPerLevel, AccelSet, miSettingsBack}; // Нумерованный список строк экрана

const char *boolSet[] = {STRING_OFF, STRING_ON};
const char *dirSet[] = {"<<<", ">>>"};
const uint8_t *stepSet[] = {1, 10, 100};

MenuItem* menuItems[] = 
{              
  new MenuItem(0, 0, MENU_01),
  new MenuItem(0, 1, MENU_02),
  new MenuItem(0, 2, MENU_03),
  new MenuItem(0, 3, MENU_04),
  new MenuItem(0, 4, MENU_05),

  new ValMenuItem(1, 0, MENU_06, MENU_FORMAT_06),
  new ValMenuItem(1, 1, MENU_07, MENU_FORMAT_06),
  new ValMenuItem(1, 2, MENU_08, MENU_FORMAT_06),
  new MenuItem(1, 3, MENU_09),
  
  new UIntMenuItem(2, 0, MENU_10, MENU_FORMAT_10, NULL, 1, 999),
  new UIntMenuItem(2, 1, MENU_13, MENU_FORMAT_13, NULL, 1, 99),
  new UIntMenuItem(2, 2, MENU_11, MENU_FORMAT_11, NULL, 5, 995, 5),
  new UIntMenuItem(2, 3, MENU_12, MENU_FORMAT_10, NULL, 30, 600, 30),
  new BoolMenuItem(2, 4, MENU_14, NULL, dirSet),
  new MenuItem(2, 5, MENU_15),
  new MenuItem(2, 6, MENU_09),

  new IntMenuItem(10, 0, MENU_17, MENU_FORMAT_17, &settings.shaftPos, -999, 999),
  new SetMenuItem(10, 1, MENU_18, MENU_FORMAT_10, &settings.shaftStep, stepSet, 3),
  new IntMenuItem(10, 2, MENU_19, MENU_FORMAT_17, &settings.layerPos, -999, 999),
  new SetMenuItem(10, 3, MENU_18, MENU_FORMAT_10, &settings.layerStep, stepSet, 3),
  new MenuItem(10, 4, MENU_09),

  new BoolMenuItem(11, 0, MENU_22, &settings.stopPerLayer, boolSet),
  new UIntMenuItem(11, 1, MENU_23, MENU_FORMAT_10, &settings.acceleration, 0, 600, 10),
  new MenuItem(11, 2, MENU_09),
}; 


const byte MENU_COUNT = sizeof(menuItems)/sizeof(*menuItems);

byte up[8] =   {0b00100,0b01110,0b11111,0b00000,0b00000,0b00000,0b00000,0b00000};   // Создаем свой символ ⯅ для LCD
byte down[8] = {0b00000,0b00000,0b00000,0b00000,0b00000,0b11111,0b01110,0b00100};   // Создаем свой символ ⯆ для LCD

#ifdef LiquidCrystal_h
LiquidCrystalCyr lcd(RS,EN,D4,D5,D6,D7);                  // Назначаем пины для управления LCD 
#endif
#ifdef LiquidCrystal_I2C_h
LiquidCrystalCyr lcd(0x27, DISPLAY_NCOL, DISPLAY_NROW);                // 0x3F I2C адрес для PCF8574AT
#endif

MainMenu menu(menuItems, MENU_COUNT, lcd);

GStepper2<STEPPER2WIRE> shaftStepper(STEPPERS_STEPS_COUNT, STEP_Z, DIR_Z, EN_STEP);
GStepper2<STEPPER2WIRE> layerStepper(STEPPERS_STEPS_COUNT, STEP_A, DIR_A, EN_STEP);
GPlanner< STEPPER2WIRE, 2> planner;

EncButton<EB_TICK, ENC_CLK, ENC_DT, ENC_SW> encoder(ENCODER_INPUT);  
EncButton<EB_TICK, STOP_BT> button;


void setup() 
{
  Serial.begin(9600);
  LoadSettings();

  pinMode(EN_STEP, OUTPUT);
  pinMode(BUZZ_OUT,OUTPUT);

  EnableSteppers(false); // Запрет управления двигателями  
  planner.addStepper(0, shaftStepper);
  planner.addStepper(1, layerStepper);

  lcd.createChar(0, up);       // Записываем символ ⯅ в память LCD
  lcd.createChar(1, down);     // Записываем символ ⯆ в память LCD
  lcd.begin(DISPLAY_NCOL, DISPLAY_NROW);                                                        // Инициализация LCD Дисплей 
  menu.Draw();

  encoder.setEncType(ENCODER_TYPE);  
} 

void loop() 
{
  encoder.tick(); 

  if (encoder.turn())                               // Проверяем изменение позиции энкодера   
  {                                                                               
    menu.index = constrain(menu.index + encoder.dir(), menu.GetFirstIndex(), menu.GetLastIndex()); // Если позиция энкодера изменена, то меняем menu.index и выводим экран
    menu.Draw();   
  }

  if (encoder.click())                               // Проверяем нажатие кнопки
  {  
    switch (menu.index)                              // Если было нажатие, то выполняем действие, соответствующее текущей позиции курсора
    {  
      case Autowinding1:  
      case Autowinding2: 
      case Autowinding3: 
              currentTransformer = menu.index - Autowinding1; 
              LoadSettings();
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
              ((UIntMenuItem*)menu[TurnsSet])->value = &params[currentWinding].turns;
              ((UIntMenuItem*)menu[StepSet])->value = &params[currentWinding].step;
              ((UIntMenuItem*)menu[SpeedSet])->value = &params[currentWinding].speed;
              ((UIntMenuItem*)menu[LaySet])->value = &params[currentWinding].layers;              
              ((BoolMenuItem*)menu[Direction])->value = &params[currentWinding].dir;
              break;
      case WindingBack:  menu.index = Autowinding1 + currentTransformer; break;
      case PosControl:   menu.index = ShaftPos; break;
      case TurnsSet:     menu.SetQuote(9,13); ValueEdit(); menu.ClearQuote(9,13); break;
      case StepSet:      menu.SetQuote(9,15); ValueEdit(); menu.ClearQuote(9,15); break;  
      case SpeedSet:     menu.SetQuote(9,13); ValueEdit(); menu.ClearQuote(9,13); break;
      case LaySet:       menu.SetQuote(9,12); ValueEdit(); menu.ClearQuote(9,12); break;   
      case AccelSet:     menu.SetQuote(9,13); ValueEdit(); menu.ClearQuote(9,13); break;
      case Direction:    menu.IncCurrent(1); break;                          
      case Start:        SaveSettings(); AutoWindingPrg(); menu.index = Winding1 + currentWinding; UpdateMenuItemText(currentWinding); break; 
      case Cancel:       SaveSettings(); menu.index = Winding1 + currentWinding; UpdateMenuItemText(currentWinding); break;

      case ShaftPos:
      case LayerPos:    
              menu.SetQuote(9,14);                         
              MoveTo((menu.index == LayerPos) ? layerStepper : shaftStepper, *((IntMenuItem*)menu[menu.index])->value);                         
              menu.ClearQuote(9,14);
              break;

      case ShaftStepMul:                                                                         
      case LayerStepMul:    
              menu.IncCurrent(1);
              ((IntMenuItem*)menu[menu.index-1])->increment = *((SetMenuItem*)menu[menu.index])->value;
              break;  
      case PosCancel:    menu.index = PosControl; settings.shaftPos = 0; settings.layerPos = 0; break;
      
      case miSettings:   menu.index = miSettingsStopPerLevel; break;
      case miSettingsStopPerLevel: 
              menu.IncCurrent(1);
              break;
      case miSettingsBack: SaveSettings(); menu.index = miSettings; break;
    }
    menu.Draw();
  }
}

void UpdateMenuItemText(byte i)
{
  ((ValMenuItem*)menu[Winding1 + i])->value = params[i].turns * params[i].layers;
}

void ValueEdit()
{
  do
  {
    encoder.tick(); 

    if (encoder.turn())                               // Проверяем изменение позиции энкодера   
      menu.IncCurrent(encoder.dir());
    
  } while (!encoder.click());
}

void EnableSteppers(bool b)
{
  digitalWrite(EN_STEP, b ? LOW : HIGH); 
}

void MoveTo(GStepper2<STEPPER2WIRE> &stepper, int &pos)
{
  EnableSteppers(true);

  stepper.setAcceleration(STEPPERS_STEPS_COUNT * settings.acceleration / 60);
  stepper.setMaxSpeed(STEPPERS_STEPS_COUNT/2);

  int oldPos = -pos * STEPPERS_MICROSTEPS * 2;
  stepper.setCurrent(oldPos);
  stepper.setTarget(oldPos);

  do
  {
    stepper.tick();
    encoder.tick(); 

    int newPos = -pos * STEPPERS_MICROSTEPS * 2;
    if (newPos != oldPos)
    {                              
      stepper.setTarget(newPos);
      oldPos = newPos;
    }    

    if (encoder.turn())                               // Проверяем изменение позиции энкодера   
      menu.IncCurrent(encoder.dir());

  } while(!encoder.click() || stepper.getStatus() != 0);

  EnableSteppers(false);
}



ISR(TIMER1_COMPA_vect) 
{
  if (planner.tickManual()) 
    setPeriod(planner.getPeriod());
  else 
    stopTimer();
}


void AutoWindingPrg()                                       // Подпрограмма автоматической намотки
{  
  Winding current;                                          // Текущий виток и слой при автонамотке
  const Winding &w = params[currentWinding];
  MainScreen screen(lcd, w, current);
 
  DebugWrite("Start");

  current.turns = 0;
  current.layers = 0;
  current.speed = w.speed;
  current.dir = w.dir;
  current.step = w.step;
   
  screen.Draw();  

  bool pause = false;
  
  EnableSteppers(true);   // Разрешение управления двигателями
 
  planner.setAcceleration(STEPPERS_STEPS_COUNT * settings.acceleration / 60L);
  planner.setMaxSpeed(STEPPERS_STEPS_COUNT * current.speed / 60L);
 
  int32_t dShaft = -STEPPERS_STEPS_COUNT * w.turns;
  int32_t dLayer = -STEPPERS_STEPS_COUNT * w.turns * w.step / int32_t(THREAD_PITCH) * (w.dir ? 1 : -1); 

  planner.reset();
  initTimer();  
 //! startTimer(); 
  
  while (1)
  {
    if (planner.getStatus() == 0 && !pause) 
    {   
      DebugWrite("READY");
      if (current.layers >= w.layers)
        break;      

      if (settings.stopPerLayer && (current.layers > 0)) 
      {
        lcd.printAt_P(0, 1, STRING_2);           // "PRESS CONTINUE  "    
        WaitButton();
        screen.Draw();
      }      
      
      int32_t p[] = {dShaft, (current.layers&1) ? -dLayer : dLayer};       
      
      planner.setTarget(p, RELATIVE);
      DebugWrite("setTarget", p[0], p[1]);
      ++current.layers;   

      startTimer();                   
      setPeriod(planner.getPeriod());

      screen.UpdateLayers();            
    }

    encoder.tick();
    button.tick();
    if (encoder.click())
    {
      
      if (pause)
      {
        planner.resume();
        startTimer();                   
        setPeriod(planner.getPeriod());
      }
      else 
        planner.stop();
      pause = !pause;
    }
            
    if (encoder.turn()) 
    {                                                                                             // Если повернуть энкодер во время автонамотки, 
      current.speed = constrain(current.speed + encoder.dir() * 30, 30, 600);                     // то меняем значение скорости
      planner.setMaxSpeed(STEPPERS_STEPS_COUNT * current.speed / 60L);
      screen.UpdateSpeed();
    }

    static uint32_t tmr;
    if (millis() - tmr >= 500) 
    {
      tmr = millis();

      int total_turns = (abs(shaftStepper.pos)) / STEPPERS_STEPS_COUNT;
      current.turns = total_turns - (current.layers-1) * w.turns;
      
      screen.UpdateTurns();
      //DebugWrite("planner.getStatus", planner.getStatus());
      //DebugWrite("", shaftStepper.pos, layerStepper.pos);   

      screen.PlannerStatus(planner.getStatus());

    }
  }

  EnableSteppers(false);

  lcd.printAt_P(0, 1, STRING_1);             // "AUTOWINDING DONE"  
  WaitButton();
}


void WaitButton()
{
  do {
    encoder.tick();
  } while (!encoder.click());
}


void LoadSettings()
{
  int p=0;
  byte v = 0;
  EEPROM_load(p, v);        
  if (v != EEPROM_DATA_VERSION)
    return;

  for (int i=0; i<TRANSFORMER_COUNT; ++i)
  {
    if (i == currentTransformer)
    {
      EEPROM_load(p, v);    
      
      for (int j=0; j<WINDING_COUNT; ++j)
        if (v == EEPROM_DATA_VERSION)
          Load(params[j], p);
        else
          { params[j] = Winding(); p += sizeof(Winding); }
    }
    else
      p += sizeof(Winding) * WINDING_COUNT + 1;
  }

  Load(settings, p);
}

void SaveSettings()
{
  int p=0;
  byte v = EEPROM_DATA_VERSION;
  EEPROM_save(p, v);          

  for (int i=0; i<TRANSFORMER_COUNT; ++i)
  {
    if (i == currentTransformer)
    {
      EEPROM_save(p, v);   
      for (int j=0; j<WINDING_COUNT; ++j)
        Save(params[j], p);
    }
    else
      p += sizeof(Winding) * WINDING_COUNT + 1;
  }

  Save(settings, p);
}


