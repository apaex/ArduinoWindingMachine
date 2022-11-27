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

//**************************************************************  
    
#define THREAD_PITCH 50 // Шаг резьбы*50
#define LANGUAGE RU     // EN, RU

//**************************************************************

#include <LiquidCrystal.h>
//#include <LiquidCrystal_I2C.h>
#include <EncButton.h>
#include <GyverPlanner2.h>
#include <GyverStepper2.h>
#include <HardwareSerial.h>
#include "LiquidCrystalCyr.h"
#include "Menu.h"

#include "Screen.h"
#include "Winding.h"
#include "strings.h"

#define ENC_CLK   2 // Даем имена номерам пинов
#define ENC_DT    5 
#define ENC_SW    3

#define STEP_Z    4 
#define DIR_Z     7
#define STEP_A    12
#define DIR_A     13
#define EN_STEP   8

#define BUZZ_OUT  10
#define STOP_BT   11

#define RS        14
#define EN        15
#define D4        16
#define D5        17
#define D6        18
#define D7        19

#define DISPLAY_NCOL        20           // размер дисплея: ширина
#define DISPLAY_NROW        4            // размер дисплея: высота

#define STEPPERS_STEPS      200          // число шагов двигателя на 1 оборот
#define STEPPERS_MICROSTEPS 16           // делитель на плате драйвера двигателя

#define ENCODER_TYPE        EB_HALFSTEP  // тип энкодера: EB_FULLSTEP или EB_HALFSTEP. если энкодер делает один поворот за два щелчка, нужно изменить настройку
#define ENCODER_INPUT       INPUT        // если есть подтягивающие резисторы - ставь INPUT, если нет - INPUT_PULLUP



#define STEPPERS_STEPS_COUNT (int32_t(STEPPERS_STEPS) * STEPPERS_MICROSTEPS)

#define EEPROM_DATA_VERSION 1

#define TRANSFORMER_COUNT 3
#define WINDING_COUNT 3

Winding params[TRANSFORMER_COUNT][WINDING_COUNT];

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

  new ValMenuItem(1, 0, MENU_06, "% 4dT"),
  new ValMenuItem(1, 1, MENU_07, "% 4dT"),
  new ValMenuItem(1, 2, MENU_08, "% 4dT"),
  new MenuItem(1, 3, MENU_09),
  
  new UIntMenuItem(2, 0, MENU_10, "%03d", NULL, 1, 999),
  new ByteMenuItem(2, 1, MENU_13, "%02d", NULL, 1, 99),
  new ByteMenuItem(2, 2, MENU_11, "0.%04d", NULL, 1, 199, THREAD_PITCH),
  new UIntMenuItem(2, 3, MENU_12, "%03d", NULL, 0, 600, 1, 30),
  new BoolMenuItem(2, 4, MENU_14, NULL, dirSet),
  new MenuItem(2, 5, MENU_15),
  new MenuItem(2, 6, MENU_09),

  new IntMenuItem(10, 0, MENU_17, "%+04d", &settings.shaftPos, -999, 999),
  new SetMenuItem(10, 1, MENU_18, "%03d", &settings.shaftStep, stepSet, 3),
  new IntMenuItem(10, 2, MENU_19, "%+04d", &settings.layerPos, -999, 999),
  new SetMenuItem(10, 3, MENU_18, "%03d", &settings.layerStep, stepSet, 3),
  new MenuItem(10, 4, MENU_09),

  new BoolMenuItem(11, 0, MENU_22, &settings.stopPerLayer, boolSet),
  new UIntMenuItem(11, 1, MENU_23, "%03d", &settings.acceleration, 0, 600, 1, 10),
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

EncButton<EB_TICK, ENC_CLK, ENC_DT, ENC_SW> encoder(ENCODER_INPUT);  
EncButton<EB_TICK, STOP_BT> button;


void setup() 
{
  Serial.begin(9600);

  LoadSettings();

  pinMode(EN_STEP, OUTPUT);
  pinMode(BUZZ_OUT,OUTPUT);

  EnableSteppers(false); // Запрет управления двигателями  

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
              ((UIntMenuItem*)menu[TurnsSet])->value = &params[currentTransformer][currentWinding].turns;
              ((ByteMenuItem*)menu[StepSet])->value = &params[currentTransformer][currentWinding].step;
              ((UIntMenuItem*)menu[SpeedSet])->value = &params[currentTransformer][currentWinding].speed;
              ((ByteMenuItem*)menu[LaySet])->value = &params[currentTransformer][currentWinding].layers;              
              ((BoolMenuItem*)menu[Direction])->value = &params[currentTransformer][currentWinding].dir;
              break;
      case WindingBack:  menu.index = Autowinding1 + currentTransformer; break;
      case PosControl:   menu.index = ShaftPos; break;
      case TurnsSet:     menu.SetQuote(9,13); ValueEdit(); menu.ClearQuote(9,13); break;
      case StepSet:      menu.SetQuote(9,16); ValueEdit(); menu.ClearQuote(9,16); break;  
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
      case miSettingsBack: menu.index = miSettings; break;
    }
    menu.Draw();
  }
}

void UpdateMenuItemText(byte i)
{
  ((ValMenuItem*)menu[Winding1 + i])->value = params[currentTransformer][i].turns * params[currentTransformer][i].layers;
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

void AutoWindingPrg()                                       // Подпрограмма автоматической намотки
{  
  Winding current;                                          // Текущий виток и слой при автонамотке
  GPlanner2< STEPPER2WIRE, 2, 4 > planner;
  planner.addStepper(0, shaftStepper);
  planner.addStepper(1, layerStepper);

  const Winding &w = params[currentTransformer][currentWinding];
  MainScreen screen(lcd, w, current);
 
  Serial.println(F("Start"));

  current.turns = 0;
  current.layers = 0;
  current.speed = w.speed;
  current.dir = w.dir;
  current.step = w.step;
   
  EnableSteppers(true);   // Разрешение управления двигателями
 
  planner.setAcceleration(STEPPERS_STEPS_COUNT * settings.acceleration / 60);
  planner.setMaxSpeed(STEPPERS_STEPS_COUNT * current.speed / 60);
  //planner.setDtA(0.1);
 
  int32_t dShaft = -STEPPERS_STEPS_COUNT * w.turns;
  int32_t dLayer = -STEPPERS_STEPS_COUNT /200L * w.turns * w.step * (w.dir ? 1 : -1); 
  int32_t p[] = {0, 0};
  
  planner.reset();
  planner.addTarget(p, 0);  // начальная точка системы должна совпадать с первой точкой маршрута

  planner.start();
  int i = 0;    // упреждающий счетчик слоёв

  screen.Draw();
  
  while (!planner.ready())
  {
    encoder.tick();
    if (encoder.turn()) {                                                                    // Если повернуть энкодер во время автонамотки, 
      current.speed = constrain(current.speed + encoder.dir(), 1, 255);                     // то меняем значение скорости
      planner.setMaxSpeed(STEPPERS_STEPS_COUNT * current.speed / 60);
      //planner.calculate();
      screen.UpdateSpeed();

      Serial.print("Speed: ");
      Serial.println(STEPPERS_STEPS_COUNT * current.speed / 60);
    }

    while(planner.available() && (i < w.layers)) 
    {
      p[0] = dShaft;
      p[1] = (i%2) ? -dLayer : dLayer;
      ++i;
      Serial.print(i);
      Serial.println(F(" - AddTarget"));
      planner.addTarget(p, (i == w.layers), RELATIVE);    // в последней точке остановка
      planner.calculate();
    }        
    
    planner.tick();

    static uint32_t tmr;
    if (millis() - tmr >= 500) {
      tmr = millis();

      int total_turns = -shaftStepper.pos / STEPPERS_STEPS_COUNT;
      current.turns = total_turns % w.turns;
      current.layers = total_turns / w.turns;
      screen.UpdateTurns();
      screen.UpdateLayers();

      Serial.print(planner.getStatus());
      Serial.print(',');
      Serial.print(shaftStepper.pos);
      Serial.print(',');
      Serial.println(layerStepper.pos);        
      
    }
  }

  planner.stop();
  screen.Draw();
  /*
  if (settings.stopPerLayer) {
    lcd.printfAt_P(0, 1, STRING_2);           // "PRESS CONTINUE  "    
    WaitButton();
  }
  */
     
  EnableSteppers(false);

  lcd.printfAt_P(0, 1, STRING_1);             // "AUTOWINDING DONE"  
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
    for (int j=0; j<WINDING_COUNT; ++j)
      Load(params[i][j], p);

  Load(settings, p);
}

void SaveSettings()
{
  int p=0;
  byte v = EEPROM_DATA_VERSION;
  EEPROM_save(p, v);          

  for (int i=0; i<TRANSFORMER_COUNT; ++i)
    for (int j=0; j<WINDING_COUNT; ++j)
      Save(params[i][j], p);

  Save(settings, p);
}




volatile uint32_t NSteps;
volatile int NTurn;
volatile int i_;                                          // Счетчик кол-ва заходов в прерывание таймера
enum Mode {mdMenu, mdVarEdit, mdRun} _mode;                // режим установки значения; работает подпрограмма автонамотки 
Winding current;

void _AutoWindingPrg()                                             // Подпрограмма автоматической намотки
{    
  cli();
  TCCR1A=(0<<COM1A1)|(0<<COM1B1)|(0<<COM1A0)|(0<<COM1B0)|(0<<WGM11)|(0<<WGM10); // Настройка таймера/счетчика 1: нормальный режим работы порта, OC1A/OC1B отключены; ATmega328/P DATASHEET стр.170-172
  TCCR1B=(0<<WGM13)|(1<<WGM12)|(0<<CS12)|(0<<CS11)|(1<<CS10);                   // Режим работы таймера/счетчика - CTC (очистить таймер при достижении значения в регистре сравнения OCR1A)
  OCR1A = 20000;                                                                // Значение в регистре OCR1A определяет частоту входа в прерывание таймера и устанавливает скрость вращения двигателей
  sei();
  NSteps = 0;
  NTurn = 0;
  i_ = 0;                                           
  int Set_Speed_INT;
  const Winding &w = params[currentTransformer][currentWinding];
  MainScreen screen(lcd, w, current);

  Serial.println(F("Start"));
  current.turns = 0;
  current.layers = 0;
  current.speed = w.speed;
  current.dir = w.dir;
  current.step = w.step;
   
  EnableSteppers(true);   // Разрешение управления двигателями
  digitalWrite(DIR_Z, HIGH);  
 
  _mode = mdRun;
 
  Set_Speed_INT = current.speed;
  while (current.layers < w.layers)                                 // Пока текущее кол-во слоев меньше заданного проверяем сколько сейчас витков
  { 
    current.turns = 0;   
    screen.Draw();
    if (current.dir) PORTB &= 0b11011111; 
    else PORTB |= 0b00100000;
    OCR1A = 65535;
    while (current.turns < w.turns)                               // Пока текущее кол-во витков меньше заданного продолжаем мотать
    {     
      while (PINB & 0b00001000)
      {
        TIMSK1=0; 

        encoder.tick();
        if (encoder.turn())                                                               // Если повернуть энкодер во время автонамотки 
          Set_Speed_INT = constrain(Set_Speed_INT + encoder.dir(), 1, 600);                     // то меняем значение скорости     
        EIMSK = 0b00000010;
        current.speed = Set_Speed_INT;      
        EIMSK = 0b00000011;
        screen.UpdateSpeed();

        if (encoder.click())
        {
          static bool EN_D;
          digitalWrite(EN_STEP, EN_D ? HIGH: LOW);
          EN_D = !EN_D;
        }
      }
      digitalWrite(EN_STEP, LOW);
      TIMSK1=2;                
       
      screen.UpdateTurns();
      
      encoder.tick();
      if (encoder.turn())                                                               // Если повернуть энкодер во время автонамотки 
        Set_Speed_INT = constrain(Set_Speed_INT + encoder.dir(), 1, 600);                     // то меняем значение скорости
      EIMSK = 0b00000010;
      current.speed = Set_Speed_INT;
      EIMSK = 0b00000011;
      screen.UpdateSpeed();      
    }  
    TIMSK1=0;
        
    current.layers++;    
    if (current.layers == w.layers) break; 
    
    if (settings.stopPerLayer) {
      lcd.printfAt_P(0, 1, STRING_2);           // "PRESS CONTINUE  "    
      WaitButton();
    }
    current.dir = !current.dir;
         
    TIMSK1=2;        
  }
     
  EnableSteppers(false);
  lcd.printfAt_P(0, 1, STRING_1);             // "AUTOWINDING DONE"  
  WaitButton();
  _mode = mdMenu;
}

ISR(TIMER1_COMPA_vect)                       // Вектор прерывания от таймера/счетчика 1 
{
  if (_mode == mdRun) 
  {
    if (NSteps < 200 * STEPPERS_MICROSTEPS) 
    {
      uint32_t INCR = current.speed * 5 / (STEPPERS_MICROSTEPS);
      OCR1A = min (65535, 300000 * 1000 / (NSteps * INCR));
    } 
    else
    {
      OCR1A = 4800000 / (current.speed*STEPPERS_MICROSTEPS);  // OCR1A_NOM;
    }
    PORTD |= 0b00010000;
    if (NTurn>>4 > 200 - current.step) PORTB |= 0b00010000;    
    while (i_<6) {i_++;} 
    i_=0;    
    PORTD &= 0b11101111; 
    if (NTurn>>4 > 200 - current.step) PORTB &= 0b11101111;
    NTurn++;
    if (NTurn>>4 > 200) {NTurn=0; current.turns++;}
    NSteps++;
  }
  i_++;                                        // Счетчик кол-ва заходов в прерывание
}



      
