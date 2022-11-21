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
#define STEPPERS_STEPS_COUNT 200 * STEPPERS_MICROSTEPS

enum Mode {mdMenu, mdVarEdit, mdRun} mode;                // режим установки значения; работает подпрограмма автонамотки 

#define TRANSFORMER_COUNT 3
#define WINDING_COUNT 3

Winding params[TRANSFORMER_COUNT][WINDING_COUNT];

byte currentTransformer = -1;
byte currentWinding = -1;

volatile int Encoder_Dir;                                 // Направление вращения энкодера
volatile bool Push_Button;                                // Нажатие кнопки

volatile bool Pause;                                      // Флаг паузы в режиме автонамотка   
Winding current;                                          // Текущий виток и слой при автонамотке
int Shaft_Pos = 0, Lay_Pos = 0;                           // Переменные изменяемые на экране

volatile uint16_t OCR1A_NOM;
uint8_t run_btn; 
volatile int Set_Speed_INT;



Settings settings;

enum menu_states {Autowinding1, Autowinding2, Autowinding3, PosControl, miSettings, Winding1, Winding2, Winding3, WindingBack, TurnsSet, StepSet, SpeedSet, LaySet, Direction, Start, Cancel, ShaftPos, ShaftStepMul, LayerPos, LayerStepMul, PosCancel, miSettingsStopPerLevel, miSettingsBack}; // Нумерованный список строк экрана



struct MenuType Menu[] = {              // Объявляем переменную Menu пользовательского типа MenuType и доступную только для чтения

  {0,  0,  ' ', "Setup 1            ", ""      ,NULL,        0,      0,      0,        0},    // "> AUTOWINDING   "
  {0,  1,  ' ', "Setup 2            ", ""      ,NULL,        0,      0,      0,        0},    // "> AUTOWINDING   "
  {0,  2,  ' ', "Setup 3            ", ""      ,NULL,        0,      0,      0,        0},    // "> AUTOWINDING   "
  {0,  3,  ' ', "Pos control        ", ""      ,NULL,        0,      0,      0,        0},    // "> POS CONTROL   "
  {0,  4,  ' ', "Settings           ", ""      ,NULL,        0,      0,      0,        0},    // "> POS CONTROL   "

  {1,  0,  'i', "Winding 1  % 3dT   ", ""      ,NULL,        0,      0,      1,        0},    // "> AUTOWINDING   "
  {1,  1,  'i', "Winding 2  % 3dT   ", ""      ,NULL,        0,      0,      1,        0},    // "> AUTOWINDING   "
  {1,  2,  'i', "Winding 3  % 3dT   ", ""      ,NULL,        0,      0,      1,        0},    // "> AUTOWINDING   "
  {1,  3,  ' ', "Back               ", ""      ,NULL,        0,      0,      0,        0},    // "> CANCEL        "  
  
  {2,  0,  'i', "Turns:  %03d       ", "%03d"  ,NULL,        1,      999,    1,        1},    // "> TURNS: >000<  "
  {2,  1,  'i', "Step: 0.%04d       ", "%04d"  ,NULL,        1,      200,    ShaftStep,1},    // "> STEP:>0.0000<↓"  
  {2,  2,  'i', "Speed:  %03d       ", "%03d"  ,NULL,        1,      300,    1,        1},    // "> SPEED: >000< ↑"
  {2,  3,  'i', "Layers: %02d       ", "%02d"  ,NULL,        1,      99,     1,        1},    // "> LAYERS:>00<  ↓" 
  {2,  4,  'd', "Direction >$>      ", ""      ,NULL,        0,      1,      1,        0},    // "> DIRECTION >>>↑"
  {2,  5,  ' ', "Start              ", ""      ,NULL,        0,      0,      0,        0},    // "> START        ↓" 
  {2,  6,  ' ', "Back               ", ""      ,NULL,        0,      0,      0,        0},    // "> CANCEL       ↑" 

  {10, 0,  'i', "SH pos: %+04d      ", "%+04d" ,&Shaft_Pos,  -999,   999,    1,        1},    // "> SH POS:>±000< "
  {10, 1,  'i', "StpMul: %03d       ", "%03d"  ,&settings.shaftStep,  1,      100,    1,        1},    // "> STPMUL:>000< ↑"
  {10, 2,  'i', "LA pos: %+04d      ", "%+04d" ,&Lay_Pos,    -999,   999,    1,        1},    // "> LA POS:>±000<↓" 
  {10, 3,  'i', "StpMul: %03d       ", "%03d"  ,&settings.layerStep,  1,      100,    1,        1},    // "> STPMUL:>000< ↑"
  {10, 4,  ' ', "Back               ", ""      ,NULL,        0,      0,      0,        0},    // "> CANCEL        "  

  {11, 0,  'b', "LayerStop          ", "%1d"  ,&settings.stopPerLayer,  0,   0,    1,        0},    // "> SH POS:>±000< "
  {11, 1,  ' ', "Back               ", ""      ,NULL,        0,      0,      0,        0},    // "> CANCEL        "  

//  {14, 0,  "T%03d/%03d L%02d/%02d", ""      ,NULL,        0,      0,      0        },    // "T000/000 L00/00 "
//  {14, 1,  "SP%03d ST0.%04d      ", ""      ,NULL,        0,      0,      0        },    // "SP000 ST0.0000  " 
//  {16, 0,  "AUTOWINDING DONE     ", ""      ,NULL,        0,      0,      0        },    // "AUTOWINDING DONE" 
//  {16, 1,  "PRESS CONTINUE       ", ""      ,NULL,        0,      0,      0        },    // "PRESS CONTINUE  "
}; 
const int MENU_COUNT = sizeof(Menu)/sizeof(*Menu);

MainMenu menu(Menu, MENU_COUNT);


const char *LINE1_FORMAT = "T%03d/%03d L%02d/%02d";
const char *LINE2_FORMAT = "Sp%03d St0.%04d";
const char *LINE4_FORMAT = "%03d";
const char *LINE5_FORMAT = "%02d";
const char *LINE6_FORMAT = "%03d";

const char *LINE3_FORMAT = "Winding %d  % 4dT";

const char *STRING_1 = "AUTOWINDING DONE";
const char *STRING_2 = "PRESS CONTINUE  ";

LiquidCrystalCyr lcd(RS,EN,D4,D5,D6,D7);                  // Назначаем пины для управления LCD 
//LiquidCrystal_I2C lcd(0x27, NCOL, NROW);                // 0x3F I2C адрес для PCF8574AT


GStepper2<STEPPER2WIRE> shaftStepper(STEPPERS_STEPS_COUNT, STEP_Z, DIR_Z, EN_STEP);
GStepper2<STEPPER2WIRE> layerStepper(STEPPERS_STEPS_COUNT, STEP_A, DIR_A, EN_STEP);


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

  cli();                                                                        // Глобальный запрет прерываний
  EICRA = (1<<ISC11)|(0<<ISC10)|(0<<ISC01)|(1<<ISC00);                          // Настройка срабатывания прерываний: INT0 по изменению сигнала, INT1 по спаду сигнала; ATmega328/P DATASHEET стр.89
  EIMSK = (1<<INT0)|(1<<INT1);                                                  // Разрешение прерываний INT0 и INT1; ATmega328/P DATASHEET стр.90 
  EIFR = 0x00;                                                                  // Сбрасываем флаги внешних прерываний; ATmega328/P DATASHEET стр.91
  TCCR1A=(0<<COM1A1)|(0<<COM1B1)|(0<<COM1A0)|(0<<COM1B0)|(0<<WGM11)|(0<<WGM10); // Настройка таймера/счетчика 1: нормальный режим работы порта, OC1A/OC1B отключены; ATmega328/P DATASHEET стр.170-172
  TCCR1B=(0<<WGM13)|(1<<WGM12)|(0<<CS12)|(0<<CS11)|(1<<CS10);                   // Режим работы таймера/счетчика - CTC (очистить таймер при достижении значения в регистре сравнения OCR1A)
  OCR1A = 20000;                                                                // Значение в регистре OCR1A определяет частоту входа в прерывание таймера и устанавливает скрость вращения двигателей

  lcd.begin(NCOL, NROW);                                                        // Инициализация LCD Дисплей 20 символов 4 строки   
  
  menu.InitRender(lcd, NCOL, NROW);
  menu.Update();
  sei();
} 


void loop() 
{
  if (mode == mdMenu && Encoder_Dir != 0)                               // Проверяем изменение позиции энкодера   
  {                                                                               
    menu.index = constrain(menu.index + Encoder_Dir, menu.GetFirstIndex(), menu.GetLastIndex()); // Если позиция энкодера изменена то меняем menu.index и выводим экран
    Encoder_Dir = 0; 
    menu.Update();   
  }

  if (mode == mdMenu && Push_Button)                                    // Проверяем нажатие кнопки
  {  
    switch (menu.index)                                                 // Если было нажатие то выполняем действие соответствующее текущей позиции курсора
    {  
      case Autowinding1:  
      case Autowinding2: 
      case Autowinding3: 
              currentTransformer = menu.index - Autowinding1; 
              menu.index = Winding1;   

              for (int i=0; i<WINDING_COUNT; ++i)
              {
                Menu[Winding1 + i].param = (int*)&params[currentTransformer][i].turns;   
                Menu[Winding1 + i].type = ' ';
                sprintf(Menu[Winding1 + i].format, LINE3_FORMAT, i+1, params[currentTransformer][i].turns * params[currentTransformer][i].layers); 
              }                                
              break;
      case Winding1:     
      case Winding2: 
      case Winding3:     
              currentWinding = menu.index - Winding1; 
              menu.index = TurnsSet;                                                          
              Menu[TurnsSet].param = (int*)&params[currentTransformer][currentWinding].turns;
              Menu[StepSet].param = (int*)&params[currentTransformer][currentWinding].step;
              Menu[SpeedSet].param = (int*)&params[currentTransformer][currentWinding].speed;
              Menu[LaySet].param = (int*)&params[currentTransformer][currentWinding].layers;              
              Menu[Direction].param = (int*)&params[currentTransformer][currentWinding].dir;
              break;
      case WindingBack:  menu.index = Autowinding1 + currentTransformer;                                                                             break;
      case PosControl:   menu.index = ShaftPos;                                                                                                      break;
      case TurnsSet:     menu.SetQuote(9,13); Push_Button=false; mode = mdVarEdit; while(!Push_Button){menu.LCD_Print_Var();} mode = mdMenu; menu.ClearQuote(9,13); break;
      case StepSet:      menu.SetQuote(7,14); Push_Button=false; mode = mdVarEdit; while(!Push_Button){menu.LCD_Print_Var();} mode = mdMenu; menu.ClearQuote(7,14); break;  
      case SpeedSet:     menu.SetQuote(9,13); Push_Button=false; mode = mdVarEdit; while(!Push_Button){menu.LCD_Print_Var();} mode = mdMenu; menu.ClearQuote(9,13); break;
      case LaySet:       menu.SetQuote(9,12); Push_Button=false; mode = mdVarEdit; while(!Push_Button){menu.LCD_Print_Var();} mode = mdMenu; menu.ClearQuote(9,12); break;   
      case Direction:
              {
                bool &Steppers_Dir = *(bool*)Menu[Direction].param;
                Push_Button = false; 
                Steppers_Dir = !Steppers_Dir;
                menu.PrintDirection(Steppers_Dir);
              }
              break;                          
      case Start:        SaveSettings(); Push_Button = false; mode = mdRun; AutoWindingPrg(); mode = mdMenu; lcd.clear();   menu.index = Winding1 + currentWinding;      break; 
      case Cancel:       SaveSettings(); menu.index = Winding1 + currentWinding;                                                                     break;

      case ShaftPos:
      case LayerPos:    { 
                          GStepper2<STEPPER2WIRE> &stepper = (menu.index == LayerPos) ? layerStepper : shaftStepper;
                          
                          menu.SetQuote(9,14); 
                          Push_Button=false; 
                          mode = mdVarEdit; 
                          digitalWrite(EN_STEP, LOW); 

                          stepper.setAcceleration(STEPPERS_STEPS_COUNT/2);
                          stepper.setMaxSpeed(STEPPERS_STEPS_COUNT/2);
    
                          int oldPos = -*(int*)Menu[menu.index].param * STEPPERS_MICROSTEPS * 2;
                          stepper.setCurrent(oldPos);
                          stepper.setTarget(oldPos);
                      
                          while(!Push_Button || stepper.getStatus() != 0)
                          {
                            stepper.tick();

                            int newPos = -*(int*)Menu[menu.index].param * STEPPERS_MICROSTEPS * 2;
                            if (newPos != oldPos)
                            {                              
                              stepper.setTarget(newPos);
                              oldPos = newPos;
                            }    

                            menu.LCD_Print_Var(); 
                          } 
                          mode = mdMenu; 
                          digitalWrite(EN_STEP, HIGH); 
                          menu.ClearQuote(9,14);
                        }
                        break;

      case ShaftStepMul:                                                                         
      case LayerStepMul:    
                        {
                            Push_Button=false;

                            int values[] = {1,10,100};
                            // переделать на индексы

                            int &value =  *(int*)Menu[menu.index].param;
                            switch (value)
                            {
                            case 1: value = 10;  break;
                            case 10: value = 100;  break;
                            case 100: value = 1;  break;
                            }

                            Menu[menu.index-1].increment = value;
                            menu.LCD_Print_Var();
                        }
                        break;  
                        /*
                            menu.SetQuote(9,13);Push_Button=false; mode = mdVarEdit; while(!Push_Button){menu.LCD_Print_Var();} mode = mdMenu; menu.ClearQuote(9,13);  
                            Menu[menu.index-1].increment = *(byte*)Menu[menu.index].param;
                            break;    
                            */
      case PosCancel:    menu.index = PosControl; Shaft_Pos = 0; Lay_Pos = 0; break;
      
      case miSettings:   menu.index = miSettingsStopPerLevel; break;
      case miSettingsStopPerLevel: 
              Push_Button = false; 
              settings.stopPerLayer = !settings.stopPerLayer;
              menu.PrintBool(settings.stopPerLayer);
              break;
      case miSettingsBack: menu.index = miSettings; break;
    }
    Push_Button = false; 
    menu.Update();
  }
}




void PrintWindingScreen()  // Подпрограмма вывода экрана автонамотки
{  
  const Winding &w = params[currentTransformer][currentWinding];

  lcd.clear();
  lcd.printfAt(0,0, LINE1_FORMAT, current.turns, w.turns, current.layers, w.layers);
  lcd.printfAt(0,1, LINE2_FORMAT, current.speed, w.step*ShaftStep);  
  PrintWindingTurns();
  PrintWindingLayers();
  PrintWindingSpeed();
}

void PrintWindingTurns()  
{  
  lcd.printfAt(1, 0, LINE4_FORMAT, current.turns+1);
}

void PrintWindingLayers()  
{  
  lcd.printfAt(10, 0, LINE5_FORMAT, current.layers+1);
}

void PrintWindingSpeed()  
{  
  lcd.printfAt(2, 1, LINE6_FORMAT, current.speed);
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

void __AutoWindingPrg()                                             // Подпрограмма автоматической намотки
{    
  const Winding &w = params[currentTransformer][currentWinding];

  Serial.println("Start");

  current.turns = 0;
  current.layers = 0;
  current.speed = w.speed;
  current.dir = w.dir;
  current.step = w.step;
   
  digitalWrite(EN_STEP, LOW);   // Разрешение управления двигателями
 
  Push_Button = false; 
 
  //shaftStepper.setSpeed(current.speed);
  //layerStepper.setSpeed(current.speed);

  while (current.layers < w.layers)                                 // Пока текущее кол-во слоев меньше заданного проверяем сколько сейчас витков
  { 
    current.turns = 0;   
    PrintWindingScreen();



    while (current.turns < w.turns)                               // Пока текущее кол-во витков меньше заданного продолжаем мотать
    {     
       for (int i=0; i<200; ++i) 
       {
          //shaftStepper.step(1*STEPPERS_STEPS_MULT);
          //layerStepper.step( (current.dir ? 1 : -1) *1*STEPPERS_STEPS_MULT /* current.step / ShaftStep / ShaftStep*/);       
       }

      PrintWindingTurns();
      current.turns++;
    }  

        
    current.layers++;    
    if (current.layers == w.layers) break; 
    
    if (settings.stopPerLayer) {
      lcd.printfAt(0, 1, STRING_2);           // "PRESS CONTINUE  "    
      WaitButton();
    }

    current.dir = !current.dir;
    
  }
     
  digitalWrite(EN_STEP, HIGH);

  lcd.printfAt(0, 1, STRING_1);             // "AUTOWINDING DONE"  
  WaitButton();
}

void AutoWindingPrg()                                             // Подпрограмма автоматической намотки
{    
  const Winding &w = params[currentTransformer][currentWinding];

  Serial.println("Start");

  current.turns = 0;
  current.layers = 0;
  current.speed = w.speed;
  current.dir = w.dir;
  current.step = w.step;
   
  digitalWrite(EN_STEP, LOW);   // Разрешение управления двигателями
  digitalWrite(DIR_Z, HIGH);  
 
  Push_Button = false; 
 
  Set_Speed_INT = current.speed;

  while (current.layers < w.layers)                                 // Пока текущее кол-во слоев меньше заданного проверяем сколько сейчас витков
  { 
    current.turns = 0;   
    PrintWindingScreen();

    if (current.dir) PORTB &= 0b11011111; 
    else PORTB |= 0b00100000;

    OCR1A = 65535;
    OCR1A_NOM = 4800000 / (current.speed*STEPPERS_MICROSTEPS); 

    while (current.turns < w.turns)                               // Пока текущее кол-во витков меньше заданного продолжаем мотать
    {     
      run_btn = PINB & 0b00001000;
      while (run_btn)
      {
        TIMSK1=0; 
        run_btn = PINB & 0b00001000;   
        EIMSK = 0b00000010;
        current.speed = Set_Speed_INT;      
        EIMSK = 0b00000011;
        
        PrintWindingSpeed();

        OCR1A_NOM = 4800000 / (current.speed*STEPPERS_MICROSTEPS); 
          
        if (Pause)
        {
          static boolean EN_D;
          Push_Button = false;
          Pause = false;
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
      
      if (current.turns > 1)
      {
        PrintWindingSpeed();

        OCR1A_NOM = 4800000/(current.speed*STEPPERS_MICROSTEPS);
      }
    }  

    TIMSK1=0;
        
    current.layers++;    
    if (current.layers == w.layers) break; 
    
    if (settings.stopPerLayer) {
      lcd.printfAt(0, 1, STRING_2);           // "PRESS CONTINUE  "    
      WaitButton();
    }

    current.dir = !current.dir;
         
    TIMSK1=2;        
  }
     
  digitalWrite(EN_STEP, HIGH);

  lcd.printfAt(0, 1, STRING_1);             // "AUTOWINDING DONE"  
  WaitButton();
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

  if (mode == mdRun && Encoder_Dir != 0)                                                                        // Если повернуть энкодер во время автонамотки 
  {
    Set_Speed_INT += Encoder_Dir; Encoder_Dir = 0; Set_Speed_INT = constrain(Set_Speed_INT, 1, 300);                     // то меняем значение скорости
  }
                                        
  if (mode == mdVarEdit && Encoder_Dir != 0) 
  {                                                                                                                      // Если находимся в режиме изменения переменной 
    *(int*)Menu[menu.index].param += Encoder_Dir * Menu[menu.index].increment; Encoder_Dir = 0;                                                             // то меняем ее сразу и
    *(int*)Menu[menu.index].param = constrain(*(int*)Menu[menu.index].param, Menu[menu.index].var_Min, Menu[menu.index].var_Max);    // ограничиваем в диапазоне var_Min ÷ var_Max
  } 
}



ISR(INT1_vect)                               // Вектор прерывания от кнопки энкодера
{   
  static unsigned long timer = 0;
  if (millis() - timer < 300) return;
  timer = millis();

  Push_Button = true;

  if (mode == mdRun) 
    Pause = true;  // Если нажать кнопку энкодера во время автонамотки то выставляем флаг паузы 
}




ISR(TIMER1_COMPA_vect)                       // Вектор прерывания от таймера/счетчика 1 
{
  static uint32_t OCR1A_TEMP;
  static uint32_t NSteps;
  static int NTurn;
  static uint32_t INCR;
  static uint32_t Temp;
  static int i;                                           // Счетчик кол-ва заходов в прерывание таймера

  if (mode == mdRun) 
  {
    if (NSteps < 200 * STEPPERS_MICROSTEPS) 
    {
      PORTD |= 0b00010000;
      if (NTurn>>4 > 200 - current.step) PORTB |= 0b00010000;
      while (i<6) {i++;} 
      i=0;
      PORTD &= 0b11101111; 
      if (NTurn>>4 > 200 - current.step) PORTB &= 0b11101111;
      NTurn++;
      if (NTurn>>4 > 200) {NTurn=0; current.turns++;}

      INCR = current.speed * 5 / (STEPPERS_MICROSTEPS);
      Temp = NSteps * INCR;
      OCR1A_TEMP = 300000 * 1000 / Temp;
      OCR1A = min (65535, OCR1A_TEMP);
    } 
    if (NSteps >= 200 * STEPPERS_MICROSTEPS) 
    {
      OCR1A = OCR1A_NOM;
      PORTD |= 0b00010000;
      if (NTurn>>4 > 200 - current.step) PORTB |= 0b00010000;
      while (i<6) {i++;} 
      i=0;
      PORTD &= 0b11101111; 
      if (NTurn>>4 > 200 - current.step) PORTB &= 0b11101111;
      NTurn++;
      if (NTurn>>4 > 200) {NTurn=0; current.turns++;}
    }
    NSteps++;
  }

  i++;                                        // Счетчик кол-ва заходов в прерывание
}








      
