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
#include <LiquidCrystalCyr.h>
//#include <LiquidCrystal_I2C.h>
//#include <Wire.h>
#include <HardwareSerial.h>
#include "winding.h"

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


enum Mode {mdMenu, mdVarEdit, mdRun} mode;                // режим установки значения; работает подпрограмма автонамотки 

#define TRANSFORMER_COUNT 3
#define WINDING_COUNT 3

Winding params[TRANSFORMER_COUNT][WINDING_COUNT];

byte currentTransformer = -1;
byte currentWinding = -1;

volatile int Encoder_Dir;                                 // Направление вращения энкодера
volatile boolean Push_Button, DC;                         // Нажатие кнопки; формирование сигнала STEP
volatile boolean Pause;                                   // Флаг паузы в режиме автонамотка   
volatile int i;                                           // Счетчик кол-ва заходов в прерывание таймера
char Str_Buffer[22];                                      // Буфер для функции sprintf 

byte Motor_Num;                                           // номер шагового двигателя
int32_t ActualShaftPos, ActualLayerPos;                   // Текущие позиции двигателей вала и укладчика
Winding current;                                          // Текущий виток и слой при автонамотке
int Shaft_Pos, Lay_Pos, Step_Mult=1;                      // Переменные изменяемые на экране
byte Menu_Index = 0;                                      // Переменная хранит номер текущей строки меню
int32_t Steps, Step_Accel, Step_Decel;

volatile uint16_t OCR1A_NOM;
volatile uint32_t OCR1A_TEMP;
volatile uint32_t INCR;
volatile uint32_t NSteps;
volatile uint32_t Temp;
int V;
volatile int NTurn;
uint8_t run_btn;
 
uint8_t MicroSteps = 16;
long SteppersPositions[2];
int Pitch = 1;

int16_t SpeedIncrease, SpeedDecrease;

volatile int X,Y;
volatile int Set_Speed_INT;

enum menu_states {Autowinding1, Autowinding2, Autowinding3, PosControl, Winding1, Winding2, Winding3, WindingBack, TurnsSet, StepSet, SpeedSet, LaySet, Direction, Start, Cancel, ShaftPos, LayPos, StepMul, PosCancel}; // Нумерованный список строк экрана

struct MenuType {                       // Структура описывающая меню
  byte Screen;                          // Индекс экрана
  byte string_number;                   // Номер строки на экране
  char type;
  char format[22];                      // Формат строки
  char format_Set_var[6];               // Формат значения при вводе переменной
  int  *param;                          // Указатель на адрес текущей переменной изменяемой на экране
  int  var_Min;                         // Ограничение значения переменной снизу
  int  var_Max;                         // Ограничение значения переменной сверху
  byte param_coef;};                    // Размерный коэффициент значения переменной

struct MenuType Menu[] = {              // Объявляем переменную Menu пользовательского типа MenuType и доступную только для чтения
  {0,  0,  ' ', "Setup 1            ", ""      ,NULL,        0,      0,      0        },    // "> AUTOWINDING   "
  {0,  1,  ' ', "Setup 2            ", ""      ,NULL,        0,      0,      0        },    // "> AUTOWINDING   "
  {0,  2,  ' ', "Setup 3            ", ""      ,NULL,        0,      0,      0        },    // "> AUTOWINDING   "
  {0,  3,  ' ', "Pos control        ", ""      ,NULL,        0,      0,      0        },    // "> POS CONTROL   "

  {1,  0,  'i', "Winding 1  % 3dT   ", ""      ,NULL,        0,      0,      1        },    // "> AUTOWINDING   "
  {1,  1,  'i', "Winding 2  % 3dT   ", ""      ,NULL,        0,      0,      1        },    // "> AUTOWINDING   "
  {1,  2,  'i', "Winding 3  % 3dT   ", ""      ,NULL,        0,      0,      1        },    // "> AUTOWINDING   "
  {1,  3,  ' ', "Back               ", ""      ,NULL,        0,      0,      0        },    // "> CANCEL        "  
  
  {2,  0,  'i', "Turns:  %03d       ", "%03d"  ,NULL,        1,      999,    1        },    // "> TURNS: >000<  "
  {2,  1,  'i', "Step: 0.%04d       ", "%04d"  ,NULL,        1,      200,    ShaftStep},    // "> STEP:>0.0000<↓"  
  {2,  2,  'i', "Speed:  %03d       ", "%03d"  ,NULL,        1,      300,    1        },    // "> SPEED: >000< ↑"
  {2,  3,  'i', "Layers: %02d       ", "%02d"  ,NULL,        1,      99,     1        },    // "> LAYERS:>00<  ↓" 
  {2,  4,  'b', "Direction >$>      ", ""      ,NULL,        0,      1,      1        },    // "> DIRECTION >>>↑"
  {2,  5,  ' ', "Start              ", ""      ,NULL,        0,      0,      0        },    // "> START        ↓" 
  {2,  6,  ' ', "Back               ", ""      ,NULL,        0,      0,      0        },    // "> CANCEL       ↑" 

  {10, 0,  'i', "SH pos: %+04d      ", "%+04d" ,&Shaft_Pos,  -999,   999,    1        },    // "> SH POS:>±000< "
  {10, 1,  'i', "LA pos: %+04d      ", "%+04d" ,&Lay_Pos,    -999,   999,    1        },    // "> LA POS:>±000<↓" 
  {10, 2,  'i', "StpMul: %03d       ", "%03d"  ,&Step_Mult,  1,      100,    1        },    // "> STPMUL:>000< ↑"
  {10, 3,  ' ', "Back               ", ""      ,NULL,        0,      0,      0        },    // "> CANCEL        "  

//  {14, 0,  "T%03d/%03d L%02d/%02d", ""      ,NULL,        0,      0,      0        },    // "T000/000 L00/00 "
//  {14, 1,  "SP%03d ST0.%04d      ", ""      ,NULL,        0,      0,      0        },    // "SP000 ST0.0000  " 
//  {16, 0,  "AUTOWINDING DONE     ", ""      ,NULL,        0,      0,      0        },    // "AUTOWINDING DONE" 
//  {16, 1,  "PRESS CONTINUE       ", ""      ,NULL,        0,      0,      0        },    // "PRESS CONTINUE  "
};  


const int MENU_COUNT = sizeof(Menu)/sizeof(*Menu);

const char *LINE1_FORMAT = "T%03d/%03d L%02d/%02d";
const char *LINE2_FORMAT = "Sp%03d St0.%04d";
const char *LINE3_FORMAT = "Winding %d  % 4dT";


byte up[8] =   {0b00100,0b01110,0b11111,0b00000,0b00000,0b00000,0b00000,0b00000};   // Создаем свой символ ⯅ для LCD
byte down[8] = {0b00000,0b00000,0b00000,0b00000,0b00000,0b11111,0b01110,0b00100};   // Создаем свой символ ⯆ для LCD

//const byte CH_UP = '^';
//const byte CH_DW = 'v';
const byte CH_UP = 0;
const byte CH_DW = 1;

LiquidCrystalCyr lcd(RS,EN,D4,D5,D6,D7);                  // Назначаем пины для управления LCD 
//LiquidCrystal_I2C lcd(0x27, NCOL, NROW);                // 0x3F I2C адрес для PCF8574AT


void setup() 
{
  Serial.begin(9600);

  LoadSettings();

  pinMode(ENC_CLK, INPUT);    // Инициализация входов/выходов  
  pinMode(ENC_SW,  INPUT);
  pinMode(STEP_Z,  OUTPUT);
  pinMode(ENC_DT,  INPUT);
  pinMode(DIR_Z,   OUTPUT);
  pinMode(EN_STEP, OUTPUT);
  pinMode(STEP_A,  OUTPUT);
  pinMode(DIR_A,   OUTPUT); 
  pinMode(BUZZ_OUT,OUTPUT);
  pinMode(STOP_BT, INPUT);
  pinMode(RS,      OUTPUT);
  pinMode(EN,      OUTPUT);
  pinMode(D4,      OUTPUT);
  pinMode(D5,      OUTPUT);
  pinMode(D6,      OUTPUT);
  pinMode(D7,      OUTPUT);

  digitalWrite(EN_STEP, HIGH); // Запрет управления двигателями  

  //digitalWrite(ENC_CLK,HIGH);  // Вкл. подтягивающие резисторы к VDD 
  //digitalWrite(ENC_SW, HIGH);   
  //digitalWrite(ENC_DT, HIGH);    
  digitalWrite(STOP_BT, HIGH);   
  
 // lcd.init(); 
  
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
  
  PrintScreen();
  sei();
} 

// для текущего меню получаем индекс первого элемента
byte GetFirstMenuIndex()
{
  return Menu_Index - Menu[Menu_Index].string_number;
}

// для текущего меню получаем индекс последнего элемента
byte GetLastMenuIndex()
{
  byte scr = Menu[Menu_Index].Screen;
  byte r = Menu_Index;
  while ((r+1 < MENU_COUNT) && (Menu[r+1].Screen == scr)) ++r;
  return r;
}

void loop() 
{
  if (mode == mdMenu && Encoder_Dir != 0)                               // Проверяем изменение позиции энкодера   
  {                                                                               
    Menu_Index = constrain(Menu_Index + Encoder_Dir, GetFirstMenuIndex(), GetLastMenuIndex()); // Если позиция энкодера изменена то меняем Menu_Index и выводим экран
    Encoder_Dir = 0; 
    PrintScreen();   
  }

  if (mode == mdMenu && Push_Button)                                    // Проверяем нажатие кнопки
  {  
    switch (Menu_Index)                                                 // Если было нажатие то выполняем действие соответствующее текущей позиции курсора
    {  
      case Autowinding1:  
      case Autowinding2: 
      case Autowinding3: 
              currentTransformer = Menu_Index - Autowinding1; 
              Menu_Index = Winding1;   

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
              currentWinding = Menu_Index - Winding1; 
              Menu_Index = TurnsSet;                                                          
              Menu[TurnsSet].param = (int*)&params[currentTransformer][currentWinding].turns;
              Menu[StepSet].param = (int*)&params[currentTransformer][currentWinding].step;
              Menu[SpeedSet].param = (int*)&params[currentTransformer][currentWinding].speed;
              Menu[LaySet].param = (int*)&params[currentTransformer][currentWinding].layers;              
              Menu[Direction].param = (int*)&params[currentTransformer][currentWinding].dir;
              break;
      case WindingBack:  Menu_Index = Autowinding1 + currentTransformer;                                                                             break;
      case PosControl:   Menu_Index = ShaftPos;                                                                                                      break;
      case TurnsSet:     SetQuote(9,13); Push_Button=false; mode = mdVarEdit; while(!Push_Button){LCD_Print_Var();} mode = mdMenu; ClearQuote(9,13); break;
      case StepSet:      SetQuote(7,14); Push_Button=false; mode = mdVarEdit; while(!Push_Button){LCD_Print_Var();} mode = mdMenu; ClearQuote(7,14); break;  
      case SpeedSet:     SetQuote(9,13); Push_Button=false; mode = mdVarEdit; while(!Push_Button){LCD_Print_Var();} mode = mdMenu; ClearQuote(9,13); break;
      case LaySet:       SetQuote(9,12); Push_Button=false; mode = mdVarEdit; while(!Push_Button){LCD_Print_Var();} mode = mdMenu; ClearQuote(9,12); break;   
      case Direction:
              {
                bool &Steppers_Dir = *(bool*)Menu[Direction].param;
                Push_Button = false; 
                Steppers_Dir = !Steppers_Dir;
                PrintDirection(12, 0, Steppers_Dir);
              }
              break;                          
      case Start:        SaveSettings(); Push_Button = false; mode = mdRun; AutoWindingPrg(); mode = mdMenu; lcd.clear();   Menu_Index = Winding1 + currentWinding;      break; 
      case Cancel:       SaveSettings(); Menu_Index = Winding1 + currentWinding;                                                                     break;

      case ShaftPos:     SetQuote(9,14); Push_Button=false; mode = mdVarEdit; digitalWrite(EN_STEP, LOW); Motor_Num = 1; OCR1A = 200000/Step_Mult;
                        while(!Push_Button){LCD_Print_Var(); ActualShaftPos=MotorMove(*Menu[Menu_Index].param * MicroSteps, ActualShaftPos);} 
                        mode = mdMenu; digitalWrite(EN_STEP, HIGH); ClearQuote(9,14);                                                                break;   
      case LayPos:       SetQuote(9,14); Push_Button=false; mode = mdVarEdit; digitalWrite(EN_STEP, LOW); Motor_Num = 2; OCR1A = 200000/Step_Mult;
                        while(!Push_Button){LCD_Print_Var(); ActualLayerPos=MotorMove(*Menu[Menu_Index].param * MicroSteps, ActualLayerPos);} 
                        mode = mdMenu; digitalWrite(EN_STEP, HIGH); ClearQuote(9,14);                                                                break;                                                               
      case StepMul:      SetQuote(9,13);Push_Button=false; mode = mdVarEdit; while(!Push_Button){LCD_Print_Var();} mode = mdMenu; ClearQuote(9,13);  break;    
      case PosCancel:    Menu_Index = Autowinding1; Shaft_Pos = 0; Lay_Pos = 0; Step_Mult = 1; ActualShaftPos = 0; ActualLayerPos = 0;               break;
    }
    Push_Button = false; 
    PrintScreen();
  }
}

void PrintScreen() // Подпрограмма: Выводим экран на LCD
{                          
  byte scr = Menu[Menu_Index].Screen;
  byte page = Menu[Menu_Index].string_number / NROW;
  byte cur = Menu[Menu_Index].string_number % NROW;  
  byte first = Menu_Index - Menu[Menu_Index].string_number + page * NROW;

  static byte prev_screen = -1;
  static byte prev_page = -1;

  if (scr != prev_screen || page != prev_page) 
  {
    lcd.clear();

    for (int i = 0; i < NROW; ++i)
    {
      MenuType &m = Menu[first + i];

      if (m.Screen != scr)
        break;     

      lcd.setCursor(2, i); 
      switch (m.type)
      {
      case 'i':
        sprintf(Str_Buffer,m.format, *m.param * m.param_coef);
        lcd.print(Str_Buffer);
        break;
      case 'b':
        lcd.print(m.format);
        PrintDirection(12, i, *m.param);
        break;
      default:
        lcd.print(m.format);
      }
    }

    prev_screen = scr;
    prev_page = page;
  }
  
  for (int i = 0; i < NROW; ++i)
      PrintSymbol(0, i, (i == cur) ? 0x3E : 0x20); 

  if (page > 0)                                                   // Выводим стрелки ⯅⯆ на соответствующих строках меню
    PrintSymbol(NCOL-1, 0, CH_UP);
  if (Menu[first + NROW].Screen == scr)
    PrintSymbol(NCOL-1, NROW-1, CH_DW); 

}


                                                                    
void PrintSymbol(byte LCD_Column, byte LCD_Row, byte Symbol_Code) // Подпрограмма: Выводим символ на экран
{ 
  lcd.setCursor(LCD_Column, LCD_Row); 
  lcd.write(byte(Symbol_Code));
}

void PrintDirection(byte c, byte r, bool d)
{
  char ch = (d) ? 0x3E : 0x3C;  
  PrintSymbol(c, r, ch); 
  PrintSymbol(c+1, r, ch); 
  PrintSymbol(c+2, r, ch); 
}

void SetQuote   (int First_Cur, int Second_Cur)                  // Подпрограмма: Выводим выделение изменяемой переменной на LCD
{
  byte cur = Menu[Menu_Index].string_number % NROW;  
  PrintSymbol(First_Cur,  cur,0x3E);   // Выводим символ >
  PrintSymbol(Second_Cur, cur,0x3C);   // Выводим символ <
  PrintSymbol(0,          cur,0x20);   // Стираем основной курсор
}

void ClearQuote (int First_Cur, int Second_Cur)                  // Подпрограмма: Стираем выделение изменяемой переменной на LCD
{
  byte cur = Menu[Menu_Index].string_number % NROW;  
  PrintSymbol(First_Cur,  cur,0x20);   // Стираем символ >
  PrintSymbol(Second_Cur, cur,0x20);   // Стираем символ <
  PrintSymbol(0,          cur,0x3E);   // Выводим основной курсор     
}

void LCD_Print_Var()                                             // Подпрограмма: Выводим новое значение переменной на LCD
{
  static int Previous_Param;

  if (*Menu[Menu_Index].param == Previous_Param)
    return;
  
  byte cur = Menu[Menu_Index].string_number % NROW;

  lcd.setCursor(10, cur);
  sprintf(Str_Buffer, Menu[Menu_Index].format_Set_var, *Menu[Menu_Index].param * Menu[Menu_Index].param_coef);
  lcd.print(Str_Buffer);
  Previous_Param = *Menu[Menu_Index].param;  
}

void PrintWendingScreen()  // Подпрограмма вывода экрана автонамотки
{  
  const Winding &w = params[currentTransformer][currentWinding];

  lcd.clear();
  sprintf(Str_Buffer, LINE1_FORMAT, current.turns, w.turns, current.layers, w.layers);
  lcd.print(Str_Buffer);
  lcd.setCursor(0, 1);
  sprintf(Str_Buffer, LINE2_FORMAT, current.speed, w.step*ShaftStep);
  lcd.print(Str_Buffer);  
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
}

void SaveSettings()
{
  int p=0;
  byte v = Winding::version;
  EEPROM_save(p, v);          p+=1;   

  for (int i=0; i<TRANSFORMER_COUNT; ++i)
    for (int j=0; j<WINDING_COUNT; ++j)
      params[i][j].Save(p);
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
  if (w.dir) PORTB &= 0b11011111; 
  else PORTB |= 0b00100000;

  PrintWendingScreen();
  
  Push_Button = false; 
 
  Set_Speed_INT = current.speed;

  while (current.layers < w.layers)                                 // Пока текущее кол-во слоев меньше заданного проверяем сколько сейчас витков
  {
    current.turns = 0;   
    OCR1A = 65535;
    OCR1A_NOM = 4800000 / (current.speed*MicroSteps); 

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
        
        lcd.setCursor(2, 1); 
        sprintf(Str_Buffer, "%03d", current.speed);
        lcd.print(Str_Buffer);

        OCR1A_NOM = 4800000 / (current.speed*MicroSteps); 
          
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
       
      lcd.setCursor(1, 0); 
      sprintf(Str_Buffer, "%03d", current.turns); 
      lcd.print(Str_Buffer);
      
      EIMSK = 0b00000010;
      current.speed = Set_Speed_INT;
      EIMSK = 0b00000011;
      
      if (current.turns > 1)
      {
        lcd.setCursor(2, 1); 
        sprintf(Str_Buffer, "%03d", current.speed);
        lcd.print(Str_Buffer);

        OCR1A_NOM = 4800000/(current.speed*MicroSteps);
      }
    }  

    TIMSK1=0;
    
    lcd.setCursor(1, 0); 
    sprintf(Str_Buffer, "%03d", current.turns); 
    lcd.print(Str_Buffer);
    
    current.layers++;

    lcd.setCursor(10, 0); 
    sprintf(Str_Buffer, "%02d", current.layers);
    lcd.print(Str_Buffer);
    
    if (current.layers == w.layers) continue; 
    
    lcd.setCursor(0, 1); 
    sprintf(Str_Buffer, "PRESS CONTINUE  ");                            // "PRESS CONTINUE  "
    lcd.print(Str_Buffer);
    
    WaitButton();
    
    lcd.setCursor(0, 1);
    sprintf(Str_Buffer, LINE2_FORMAT, current.speed, w.step*ShaftStep);
    lcd.print(Str_Buffer);
    
    current.dir = !current.dir;

    if (current.dir) PORTB &= 0b11011111; 
    else PORTB |= 0b00100000;
     

    lcd.setCursor(1, 0); 
    sprintf(Str_Buffer, "%03d", current.turns); 
    lcd.print(Str_Buffer);
    
    TIMSK1=2;        
  }
     
  digitalWrite(EN_STEP, HIGH);

  lcd.setCursor(0, 1); 
  sprintf(Str_Buffer, "AUTOWINDING DONE");                             // "AUTOWINDING DONE"
  lcd.print(Str_Buffer);
  
  WaitButton();
}

void WaitButton()
{
  Push_Button = false;
  while (!Push_Button);
  Push_Button = false;
}

int MotorMove(int32_t Move_Var, int32_t Actual_Rot)                     // Подпрограмма: Движение шагового двигателя до заданной координаты
{ 
  long Rotation;        
  Rotation = Move_Var * Step_Mult - Actual_Rot;
  switch(Motor_Num) {
    case 1: if      (Rotation > 0) {PORTD |= 0b10000000; TCNT1=0; TIMSK1=2; while(i<2){} TIMSK1=0; TCNT1=0; Actual_Rot++; i=0; DC=false;} 
            else if (Rotation < 0) {PORTD &= 0b01111111; TCNT1=0; TIMSK1=2; while(i<2){} TIMSK1=0; TCNT1=0; Actual_Rot--; i=0; DC=false;} 
            else     TIMSK1 = 0; i = 0; DC = false; break; 
    case 2: if      (Rotation > 0) {PORTB |= 0b00100000; TCNT1=0; TIMSK1=2; while(i<2){} TIMSK1=0; TCNT1=0; Actual_Rot++; i=0; DC=false;} 
            else if (Rotation < 0) {PORTB &= 0b11011111; TCNT1=0; TIMSK1=2; while(i<2){} TIMSK1=0; TCNT1=0; Actual_Rot--; i=0; DC=false;}
            else     TIMSK1 = 0; i = 0; DC = false; break;}                    
  return Actual_Rot;
}



ISR(INT0_vect)   // Вектор прерывания от энкодера
{
  static byte Enc_Temp_prev;                                             // Временная переменная для хранения состояния порта

  byte Enc_Temp = PIND & 0b00100100;                                     // Маскируем все пины порта D кроме PD2 и PD5      

  if (Enc_Temp==0b00100000 && Enc_Temp_prev==0b00000100) {Encoder_Dir = -1;} // -1 - шаг против часовой
  else if (Enc_Temp==0b00000000 && Enc_Temp_prev==0b00100100) {Encoder_Dir =  1;} // +1 - шаг по часовой
  else if (Enc_Temp==0b00100000 && Enc_Temp_prev==0b00100100) {Encoder_Dir = -1;}
  else if (Enc_Temp==0b00000000 && Enc_Temp_prev==0b00000100) {Encoder_Dir =  1;}

  Enc_Temp_prev = Enc_Temp;

  if (mode == mdRun && Encoder_Dir != 0)                                                                        // Если повернуть энкодер во время автонамотки 
  {
    Set_Speed_INT += Encoder_Dir; Encoder_Dir = 0; Set_Speed_INT = constrain(Set_Speed_INT, 1, 300);                     // то меняем значение скорости
  }
                                        
  if (mode == mdVarEdit && Encoder_Dir != 0) 
  {                                                                                                                      // Если находимся в режиме изменения переменной 
    *Menu[Menu_Index].param += Encoder_Dir; Encoder_Dir = 0;                                                             // то меняем ее сразу и
    *Menu[Menu_Index].param = constrain(*Menu[Menu_Index].param, Menu[Menu_Index].var_Min, Menu[Menu_Index].var_Max);    // ограничиваем в диапазоне var_Min ÷ var_Max
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
 if (mode == mdRun) 
 {
  Motor_Num = 0;
  if (NSteps < 200 * MicroSteps) 
  {
   PORTD |= 0b00010000;
   if (NTurn>>4 > 200 - current.step) PORTB |= 0b00010000;
   while (i<6) {i++;} i=0;
   PORTD &= 0b11101111; 
   if (NTurn>>4 > 200 - current.step) PORTB &= 0b11101111;
   NTurn++;
   if (NTurn>>4 > 200) {NTurn=0; current.turns++;}

  INCR = current.speed * 5 / (MicroSteps);
  Temp = NSteps * INCR;
  OCR1A_TEMP = 300000 * 1000 / Temp;
  OCR1A = min (65535, OCR1A_TEMP);
  } 
  if (NSteps >= 200 * MicroSteps) 
  {
   OCR1A = OCR1A_NOM;
   PORTD |= 0b00010000;
   if (NTurn>>4 > 200 - current.step) PORTB |= 0b00010000;
   while (i<6) {i++;} i=0;
   PORTD &= 0b11101111; 
   if (NTurn>>4 > 200 - current.step) PORTB &= 0b11101111;
   NTurn++;
   if (NTurn>>4 > 200) {NTurn=0; current.turns++;}
  }
    NSteps++;
  }
  
  i++;                                        // Счетчик кол-ва заходов в прерывание
  DC = !DC;                                   // Первое прерывание устанавливает STEP следующее - сбрасывает
    if      (Motor_Num == 1) {
      if (DC == true) {PORTD |= 0b00010000;}  // STEP_Z
      else            {PORTD &= 0b11101111;}}
    else if (Motor_Num == 2) {
      if (DC == true) {PORTB |= 0b00010000;}  // STEP_A
      else            {PORTB &= 0b11101111;}}
}








      
