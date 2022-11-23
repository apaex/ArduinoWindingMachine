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
    
#define ShaftStep 50 // ShaftStep = Шаг резьбы*50

//**************************************************************

#include <avr/io.h>
#include <avr/interrupt.h>
//#include <LiquidCrystal.h>
//#include <LiquidCrystal_I2C.h>
#include <GyverPlanner2.h>
#include <GyverStepper2.h>
#include <HardwareSerial.h>
#include "Menu.h"
#include "Winding.h"
#include "LiquidCrystalCyr.h"

#define ENC_CLK   2 // Даем имена номерам пинов
#define ENC_SW    3
#define STEP_Z    4 
#define ENC_DT    5 
#define DIR_Z     7
#define EN_STEP   8
#define BUZZ_OUT  10
#define STOP_BT   11
#define STEP_A    12
#define DIR_A     13
#define RS        14
#define EN        15
#define D4        16
#define D5        17
#define D6        18
#define D7        19

#define NCOL 20
#define NROW 4 

#define STEPPERS_MICROSTEPS 16
#define STEPPERS_STEPS_COUNT (200L * STEPPERS_MICROSTEPS)


#define TRANSFORMER_COUNT 3
#define WINDING_COUNT 3

Winding params[TRANSFORMER_COUNT][WINDING_COUNT];

int8_t currentTransformer = -1;
int8_t currentWinding = -1;

volatile int8_t Encoder_Dir = 0;                          // Направление вращения энкодера
volatile bool Push_Button = false;                        // Нажатие кнопки

Winding current;                                          // Текущий виток и слой при автонамотке
int Shaft_Pos = 0, Lay_Pos = 0;                           // Переменные, изменяемые на экране

volatile uint32_t NSteps;
volatile int NTurn;
volatile int i_;                                          // Счетчик кол-ва заходов в прерывание таймера
enum Mode {mdMenu, mdVarEdit, mdRun} _mode;                // режим установки значения; работает подпрограмма автонамотки 

Settings settings;

enum menu_states {Autowinding1, Autowinding2, Autowinding3, PosControl, miSettings, Winding1, Winding2, Winding3, WindingBack, TurnsSet, StepSet, SpeedSet, LaySet, Direction, Start, Cancel, ShaftPos, ShaftStepMul, LayerPos, LayerStepMul, PosCancel, miSettingsStopPerLevel, miSettingsBack}; // Нумерованный список строк экрана

const char * boolSet[] = {"OFF", "ON "};
const char * dirSet[] = {"<<<", ">>>"};

MenuItem* Menu[] = {              // Объявляем переменную Menu пользовательского типа MenuType и доступную только для чтения

  new MenuItem(0,  0,  "Setup 1"),
  new MenuItem(0,  1,  "Setup 2"),
  new MenuItem(0,  2,  "Setup 3"),
  new MenuItem(0,  3,  "Pos control"),
  new MenuItem(0,  4,  "Settings"),

  new MenuItem(1,  0,  "Winding 1   0000T"),
  new MenuItem(1,  1,  "Winding 2   0000T"),
  new MenuItem(1,  2,  "Winding 3   0000T"),
  new MenuItem(1,  3,  "Back"),
  
  new UIntMenuItem(2,  0,  "Turns:", "%03d", NULL, 1, 999),
  new ByteMenuItem(2,  1,  "Step:", "0.%04d", NULL, 1, 199,    ShaftStep),
  new ByteMenuItem(2,  2,  "Speed:", "%03d", NULL, 1, 255),
  new ByteMenuItem(2,  3,  "Layers:", "%02d", NULL, 1, 99),
  new BoolMenuItem(2,  4,  "Direction", NULL, dirSet),
  new MenuItem(2,  5,  "Start"),
  new MenuItem(2,  6,  "Back"),

  new IntMenuItem(10, 0,  "SH pos:", "%+04d" ,&Shaft_Pos,  -999,   999),
  new SetMenuItem(10, 1,  "StpMul:", "%03d", &settings.shaftStep),
  new IntMenuItem(10, 2,  "LA pos:", "%+04d" ,&Lay_Pos,    -999,   999),
  new SetMenuItem(10, 3,  "StpMul:", "%03d", &settings.layerStep),
  new MenuItem(10, 4,  "Back"),

  new BoolMenuItem(11, 0,  "LayerStop", &settings.stopPerLayer, boolSet),
  new MenuItem(11, 1,  "Back"),
}; 

const byte MENU_COUNT = sizeof(Menu)/sizeof(*Menu);

MainMenu menu(Menu, MENU_COUNT);


const char PROGMEM LINE1_FORMAT[] = "T%03d/%03d L%02d/%02d";
const char PROGMEM LINE2_FORMAT[] = "Sp%03d St0.%04d";
const char PROGMEM LINE4_FORMAT[] = "%03d";
const char PROGMEM LINE5_FORMAT[] = "%02d";
const char PROGMEM LINE6_FORMAT[] = "%03d";

const char PROGMEM LINE3_FORMAT[] = "Winding %d  % 4dT";

const char PROGMEM STRING_1[] = "AUTOWINDING DONE";
const char PROGMEM STRING_2[] = "PRESS CONTINUE  ";

LiquidCrystalCyr lcd(RS,EN,D4,D5,D6,D7);                  // Назначаем пины для управления LCD 
//LiquidCrystal_I2C lcd(0x27, NCOL, NROW);                // 0x3F I2C адрес для PCF8574AT


GStepper2<STEPPER2WIRE> shaftStepper(STEPPERS_STEPS_COUNT, STEP_Z, DIR_Z, EN_STEP);
GStepper2<STEPPER2WIRE> layerStepper(STEPPERS_STEPS_COUNT, STEP_A, DIR_A, EN_STEP);
GPlanner2< STEPPER2WIRE, 2, 6 > planner;

void setup() 
{
  Serial.begin(9600);

  LoadSettings();

  pinMode(ENC_CLK, INPUT);    // Инициализация входов/выходов  
  pinMode(ENC_SW,  INPUT);
  pinMode(ENC_DT,  INPUT);
  pinMode(STOP_BT, INPUT);
  pinMode(EN_STEP, OUTPUT);
  pinMode(BUZZ_OUT,OUTPUT);


  digitalWrite(EN_STEP, HIGH); // Запрет управления двигателями  

  //digitalWrite(ENC_CLK,HIGH);  // Вкл. подтягивающие резисторы к VDD 
  //digitalWrite(ENC_SW, HIGH);   
  //digitalWrite(ENC_DT, HIGH);    
  digitalWrite(STOP_BT, HIGH);   

 // lcd.init(); 
  
  byte up[8] =   {0b00100,0b01110,0b11111,0b00000,0b00000,0b00000,0b00000,0b00000};   // Создаем свой символ ⯅ для LCD
  byte down[8] = {0b00000,0b00000,0b00000,0b00000,0b00000,0b11111,0b01110,0b00100};   // Создаем свой символ ⯆ для LCD

  lcd.createChar(0, up);       // Записываем символ ⯅ в память LCD
  lcd.createChar(1, down);     // Записываем символ ⯆ в память LCD

  planner.addStepper(0, shaftStepper);
  planner.addStepper(1, layerStepper);

  cli();                                                                        // Глобальный запрет прерываний
  EICRA = (1<<ISC11)|(0<<ISC10)|(0<<ISC01)|(1<<ISC00);                          // Настройка срабатывания прерываний: INT0 по изменению сигнала, INT1 по спаду сигнала; ATmega328/P DATASHEET стр.89
  EIMSK = (1<<INT0)|(1<<INT1);                                                  // Разрешение прерываний INT0 и INT1; ATmega328/P DATASHEET стр.90 
  EIFR = 0x00;                                                                  // Сбрасываем флаги внешних прерываний; ATmega328/P DATASHEET стр.91
  //TCCR1A=(0<<COM1A1)|(0<<COM1B1)|(0<<COM1A0)|(0<<COM1B0)|(0<<WGM11)|(0<<WGM10); // Настройка таймера/счетчика 1: нормальный режим работы порта, OC1A/OC1B отключены; ATmega328/P DATASHEET стр.170-172
  //TCCR1B=(0<<WGM13)|(1<<WGM12)|(0<<CS12)|(0<<CS11)|(1<<CS10);                   // Режим работы таймера/счетчика - CTC (очистить таймер при достижении значения в регистре сравнения OCR1A)
  //OCR1A = 20000;                                                                // Значение в регистре OCR1A определяет частоту входа в прерывание таймера и устанавливает скрость вращения двигателей

  lcd.begin(NCOL, NROW);                                                        // Инициализация LCD Дисплей 20 символов 4 строки   
  
  menu.InitRender(lcd, NCOL, NROW);
  menu.Update();
  sei();
} 



void loop() 
{
  if (Encoder_Dir != 0)                               // Проверяем изменение позиции энкодера   
  {                                                                               
    menu.index = constrain(menu.index + Encoder_Dir, menu.GetFirstIndex(), menu.GetLastIndex()); // Если позиция энкодера изменена то меняем menu.index и выводим экран
    Encoder_Dir = 0; 
    menu.Update();   
  }

  if (Push_Button)                                    // Проверяем нажатие кнопки
  {  
    switch (menu.index)                                                 // Если было нажатие то выполняем действие соответствующее текущей позиции курсора
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
              ((UIntMenuItem*)Menu[TurnsSet])->value = &params[currentTransformer][currentWinding].turns;
              ((ByteMenuItem*)Menu[StepSet])->value = &params[currentTransformer][currentWinding].step;
              ((ByteMenuItem*)Menu[SpeedSet])->value = &params[currentTransformer][currentWinding].speed;
              ((ByteMenuItem*)Menu[LaySet])->value = &params[currentTransformer][currentWinding].layers;              
              ((BoolMenuItem*)Menu[Direction])->value = &params[currentTransformer][currentWinding].dir;
              break;
      case WindingBack:  menu.index = Autowinding1 + currentTransformer; break;
      case PosControl:   menu.index = ShaftPos; break;
      case TurnsSet:     menu.SetQuote(9,13); Push_Button=false; while(!Push_Button) { ValEditTick(); } menu.ClearQuote(9,13); break;
      case StepSet:      menu.SetQuote(9,16); Push_Button=false; while(!Push_Button) { ValEditTick(); } menu.ClearQuote(9,16); break;  
      case SpeedSet:     menu.SetQuote(9,13); Push_Button=false; while(!Push_Button) { ValEditTick(); } menu.ClearQuote(9,13); break;
      case LaySet:       menu.SetQuote(9,12); Push_Button=false; while(!Push_Button) { ValEditTick(); } menu.ClearQuote(9,12); break;   
      case Direction:    menu.IncCurrent(1); break;                          
      case Start:        SaveSettings(); Push_Button = false; AutoWindingPrg(); menu.index = Winding1 + currentWinding; UpdateMenuItemText(currentWinding); break; 
      case Cancel:       SaveSettings(); menu.index = Winding1 + currentWinding; UpdateMenuItemText(currentWinding); break;

      case ShaftPos:
      case LayerPos:    
              { 
                menu.SetQuote(9,14); 
                IntMenuItem* m = (IntMenuItem*)Menu[menu.index];                          
                MoveTo((menu.index == LayerPos) ? layerStepper : shaftStepper, *m->value);                         
                menu.ClearQuote(9,14);
              }
              break;

      case ShaftStepMul:                                                                         
      case LayerStepMul:    
              menu.IncCurrent(1);
              ((IntMenuItem*)Menu[menu.index-1])->increment = *((SetMenuItem*)Menu[menu.index])->value;
              break;  
      case PosCancel:    menu.index = PosControl; Shaft_Pos = 0; Lay_Pos = 0; break;
      
      case miSettings:   menu.index = miSettingsStopPerLevel; break;
      case miSettingsStopPerLevel: 
              menu.IncCurrent(1);
              break;
      case miSettingsBack: menu.index = miSettings; break;
    }
    Push_Button = false; 
    menu.Update();
  }
}


void ValEditTick()
{
  if (Encoder_Dir != 0) 
  {    
    menu.IncCurrent(Encoder_Dir);         
    Encoder_Dir = 0;                                                                
  } 
}

void UpdateMenuItemText(byte i)
{
  sprintf_P(Menu[Winding1 + i]->format, LINE3_FORMAT, i+1, params[currentTransformer][i].turns * params[currentTransformer][i].layers); 
}

void MoveTo(GStepper2<STEPPER2WIRE> &stepper, int &pos)
{
  Push_Button=false; 
  digitalWrite(EN_STEP, LOW); 

  stepper.setAcceleration(STEPPERS_STEPS_COUNT/2);
  stepper.setMaxSpeed(STEPPERS_STEPS_COUNT/2);

  int oldPos = -pos * STEPPERS_MICROSTEPS * 2;
  stepper.setCurrent(oldPos);
  stepper.setTarget(oldPos);

  while(!Push_Button || stepper.getStatus() != 0)
  {
    stepper.tick();

    int newPos = -pos * STEPPERS_MICROSTEPS * 2;
    if (newPos != oldPos)
    {                              
      stepper.setTarget(newPos);
      oldPos = newPos;
    }    

    ValEditTick(); 
  } 
  digitalWrite(EN_STEP, HIGH); 
}


void PrintWindingScreen()  // Подпрограмма вывода экрана автонамотки
{  
  const Winding &w = params[currentTransformer][currentWinding];

  lcd.clear();
  lcd.printfAt_P(0,0, LINE1_FORMAT, current.turns, w.turns, current.layers, w.layers);
  lcd.printfAt_P(0,1, LINE2_FORMAT, current.speed, w.step*ShaftStep);  
  PrintWindingTurns();
  PrintWindingLayers();
  PrintWindingSpeed();
}

void PrintWindingTurns()  
{  
  lcd.printfAt_P(1, 0, LINE4_FORMAT, current.turns+1);
}

void PrintWindingLayers()  
{  
  lcd.printfAt_P(10, 0, LINE5_FORMAT, current.layers+1);
}

void PrintWindingSpeed()  
{  
  lcd.printfAt_P(2, 1, LINE6_FORMAT, current.speed);
}

void LoadSettings()
{
  int p=0;
  byte v = 0;
  EEPROM.get(p, v);          p+=1;
  if (v != Winding::version)
    return;

  for (int i=0; i<TRANSFORMER_COUNT; ++i)
    for (int j=0; j<WINDING_COUNT; ++j)
      params[i][j].Load(p);

  //settings.Load(p);
}

void SaveSettings()
{
  int p=0;
  byte v = Winding::version;
  EEPROM_save(p, v);          p+=1;   

  for (int i=0; i<TRANSFORMER_COUNT; ++i)
    for (int j=0; j<WINDING_COUNT; ++j)
      params[i][j].Save(p);

  //settings.Save(p);
}

void AutoWindingPrg()                                             // Подпрограмма автоматической намотки
{    
  int Set_Speed_INT;
  const Winding &w = params[currentTransformer][currentWinding];

  Serial.println(F("Start"));

  current.turns = 0;
  current.layers = 0;
  current.speed = w.speed;
  current.dir = w.dir;
  current.step = w.step;
   
  digitalWrite(EN_STEP, LOW);   // Разрешение управления двигателями
 
  Push_Button = false; 
 
  planner.setAcceleration(STEPPERS_STEPS_COUNT / 2);
  planner.setMaxSpeed(STEPPERS_STEPS_COUNT / 2);
  //planner.setDtA(1.0);
 
  int32_t dShaft = -STEPPERS_STEPS_COUNT * w.turns;
  int32_t dLayer = -STEPPERS_STEPS_COUNT /200L * w.turns * w.step * (w.dir ? 1 : -1); 
  int32_t p[] = {0, 0};
  
  planner.reset();
  planner.addTarget(p, 0);  // начальная точка системы должна совпадать с первой точкой маршрута

  planner.start();
  int i = 0;    // упреждающий счетчик слоёв

  PrintWindingScreen();
  
  while (!planner.ready())
  {
    if (Encoder_Dir) {                                                                    // Если повернуть энкодер во время автонамотки 
      Set_Speed_INT = constrain(Set_Speed_INT + Encoder_Dir, 1, 255);                     // то меняем значение скорости
      Encoder_Dir = 0; 
    }

    planner.tick();

    if (planner.available() && (i < w.layers)) 
    {
      p[0] = dShaft;
      p[1] = (i%2) ? -dLayer : dLayer;
      ++i;
      planner.addTarget(p, (i == w.layers), RELATIVE);    // в последней точке остановка
    }    

    static uint32_t tmr;
    if (millis() - tmr >= 2000) {
      tmr = millis();

      int total_turns = -shaftStepper.pos / STEPPERS_STEPS_COUNT;
      current.turns = total_turns % w.turns;
      current.layers = total_turns / w.turns;
      PrintWindingTurns();
      PrintWindingLayers();

      Serial.print(shaftStepper.pos);
      Serial.print(',');
      Serial.println(layerStepper.pos);        
    }
  }

  planner.stop();
  /*
  if (settings.stopPerLayer) {
    lcd.printfAt_P(0, 1, STRING_2);           // "PRESS CONTINUE  "    
    WaitButton();
  }
  */
     
  digitalWrite(EN_STEP, HIGH);

  lcd.printfAt_P(0, 1, STRING_1);             // "AUTOWINDING DONE"  
  WaitButton();
}

void _AutoWindingPrg()                                             // Подпрограмма автоматической намотки
{    
  NSteps = 0;
  NTurn = 0;
  i_ = 0;                                           
  int Set_Speed_INT;

  const Winding &w = params[currentTransformer][currentWinding];

  Serial.println(F("Start"));

  current.turns = 0;
  current.layers = 0;
  current.speed = w.speed;
  current.dir = w.dir;
  current.step = w.step;
   
  digitalWrite(EN_STEP, LOW);   // Разрешение управления двигателями
  digitalWrite(DIR_Z, HIGH);  
 
  Push_Button = false; 
  _mode = mdRun;
 
  Set_Speed_INT = current.speed;

  while (current.layers < w.layers)                                 // Пока текущее кол-во слоев меньше заданного проверяем сколько сейчас витков
  { 
    current.turns = 0;   
    PrintWindingScreen();

    if (current.dir) PORTB &= 0b11011111; 
    else PORTB |= 0b00100000;

    OCR1A = 65535;

    while (current.turns < w.turns)                               // Пока текущее кол-во витков меньше заданного продолжаем мотать
    {     
      while (PINB & 0b00001000)
      {
        if (Encoder_Dir != 0) {                                                               // Если повернуть энкодер во время автонамотки 
          Set_Speed_INT = constrain(Set_Speed_INT + Encoder_Dir, 1, 255);                     // то меняем значение скорости
          Encoder_Dir = 0; 
        }
              
        TIMSK1=0; 

        EIMSK = 0b00000010;
        current.speed = Set_Speed_INT;      
        EIMSK = 0b00000011;
        PrintWindingSpeed();

        if (Push_Button)
        {
          static bool EN_D;
          Push_Button = false;
          digitalWrite(EN_STEP, EN_D ? HIGH: LOW);
          EN_D = !EN_D;
        }
      }

      digitalWrite(EN_STEP, LOW);
      TIMSK1=2;                
       
      PrintWindingTurns();
      
      EIMSK = 0b00000010;
      current.speed = Set_Speed_INT;
      EIMSK = 0b00000011;
      PrintWindingSpeed();      
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
     
  digitalWrite(EN_STEP, HIGH);

  lcd.printfAt_P(0, 1, STRING_1);             // "AUTOWINDING DONE"  
  WaitButton();
  _mode = mdMenu;
}

void WaitButton()
{
  Push_Button = false;
  while (!Push_Button);
  Push_Button = false;
}


ISR(INT0_vect)   // Вектор прерывания от энкодера
{
  static byte Enc_Temp_prev;                                             // Временная переменная для хранения состояния порта

  byte Enc_Temp = PIND & 0b00100100;                                     // Маскируем все пины порта D кроме PD2 и PD5      

  if (Enc_Temp==0b00100000 && Enc_Temp_prev==0b00000100) {Encoder_Dir += -1;} // -1 - шаг против часовой
  else if (Enc_Temp==0b00000000 && Enc_Temp_prev==0b00100100) {Encoder_Dir +=  1;} // +1 - шаг по часовой
  else if (Enc_Temp==0b00100000 && Enc_Temp_prev==0b00100100) {Encoder_Dir += -1;}
  else if (Enc_Temp==0b00000000 && Enc_Temp_prev==0b00000100) {Encoder_Dir +=  1;}

  Enc_Temp_prev = Enc_Temp;         
}



ISR(INT1_vect)                               // Вектор прерывания от кнопки энкодера
{   
  static unsigned long timer = 0;
  if (millis() - timer < 300) return;
  timer = millis();

  Push_Button = true;
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








      
