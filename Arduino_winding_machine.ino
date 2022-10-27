/* Name: Winding machine    
   Description: Arduino ATmega 328P + Stepper motor control CNC Shield v3 + 2004 LCD + Encoder KY-040
   Author:      TDA
   Ver:         2.0a
   Date:        07/10/2019

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
*/

//**************************************************************  
    
#define ShaftStep 50 // ShaftStep = Шаг резьбы*50

//**************************************************************


#include <avr/io.h>
#include <avr/interrupt.h>
#include <LiquidCrystal.h>
//#include <LiquidCrystal_I2C.h>
//#include <Wire.h>

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

byte up[8] =   {0b00100,0b01110,0b11111,0b00000,0b00000,0b00000,0b00000,0b00000};   // Создаем свой символ ⯅ для LCD
byte down[8] = {0b00000,0b00000,0b00000,0b00000,0b00000,0b11111,0b01110,0b00100};   // Создаем свой символ ⯆ для LCD

volatile int Encoder_Dir;                                 // Направление вращения энкодера
volatile boolean Push_Button, Var_Set, DC, AutoWindStart; // Нажатие кнопки; режим установки значения; формирование сигнала STEP; работает подпрограмма автонамотки 
volatile boolean Pause;                                   // Флаг паузы в режиме автонамотка   
volatile int i;                                           // Счетчик кол-ва заходов в прерывание таймера
char Str_Buffer[22];                                      // Буфер для функции sprintf 
byte LCD_Column, LCD_Row, Symbol_Code, Motor_Num;         // Номер столбца и строки LCD; код символа https://i.stack.imgur.com/oZhjJ.gif; номер шагового двигателя
int32_t ActualShaftPos, ActualLayerPos;                   // Текущие позиции двигателей вала и укладчика
int Actual_Turn = 0, Actual_Layer = 0;                    // Текущий виток и слой при автонамотке
int Shaft_Pos, Lay_Pos, Set_Turns, Set_Step, Set_Speed=1, Set_Layers=1, Step_Mult=1;  // Переменные изменяемые на экране
byte Menu_Index = 0;                                      // Переменная хранит номер текущей строки меню
int32_t Steps, Step_Accel, Step_Decel;

volatile uint16_t OCR1A_NOM;
volatile uint32_t OCR1A_TEMP;
volatile uint32_t INCR;
volatile uint32_t NSteps;
volatile uint32_t Temp;
int V;
volatile int NTurn;

 
uint8_t MicroSteps = 16;
long SteppersPositions[2];
int Pitch = 1;
int8_t Steppers_Dir = 1;
int16_t SpeedIncrease, SpeedDecrease;

enum menu_states {Autowinding, PosControl, TurnsSet, StepSet, SpeedSet, LaySet, Direction, Start, Cancel, Empty, ShaftPos, LayPos, StepMul, PosCancel}; // Нумерованный список строк экрана

struct MenuType {                       // Структура описывающая меню
  byte Screen;                          // Индекс экрана
  byte string_number;                   // Номер строки на экране
  char format[22];                      // Формат строки
  char format_Set_var[6];               // Формат значения при вводе переменной
  int  *param;                          // Указатель на адрес текущей переменной изменяемой на экране
  int  var_Min;                         // Ограничение значения переменной снизу
  int  var_Max;                         // Ограничение значения переменной сверху
  byte param_coef;};                    // Размерный коэффициент значения переменной

const struct MenuType Menu[] = {        // Объявляем переменную Menu пользовательского типа MenuType и доступную только для чтения
  {0,  0,  "  AUTOWINDING        ", ""      ,NULL,        0,      0,      0        },    // "> AUTOWINDING   "
  {0,  1,  "  POS CONTROL        ", ""      ,NULL,        0,      0,      0        },    // "> POS CONTROL   "
  {2,  0,  "  TURNS:  %03d       ", "%03d"  ,&Set_Turns,  1,      999,    1        },    // "> TURNS: >000<  "
  {2,  1,  "  STEP: 0.%04d       ", "%04d"  ,&Set_Step,   1,      200,    ShaftStep},    // "> STEP:>0.0000<↓"  
  {4,  0,  "  SPEED:  %03d       ", "%03d"  ,&Set_Speed,  1,      300,    1        },    // "> SPEED: >000< ↑"
  {4,  1,  "  LAYERS: %02d       ", "%02d"  ,&Set_Layers, 1,      99,     1        },    // "> LAYERS:>00<  ↓" 
  {6,  0,  "  DIRECTION >>>      ", ""      ,NULL,        0,      0,      0        },    // "> DIRECTION >>>↑"
  {6,  1,  "  START              ", ""      ,NULL,        0,      0,      0        },    // "> START        ↓" 
  {8,  0,  "  CANCEL             ", ""      ,NULL,        0,      0,      0        },    // "> CANCEL       ↑" 
  {8,  1,  "                     ", ""      ,NULL,        0,      0,      0        },    // ">               " 
  {10, 0,  "  SH POS: %+04d      ", "%+04d" ,&Shaft_Pos,  -999,   999,    1        },    // "> SH POS:>±000< "
  {10, 1,  "  LA POS: %+04d      ", "%+04d" ,&Lay_Pos,    -999,   999,    1        },    // "> LA POS:>±000<↓" 
  {12, 0,  "  STPMUL: %03d       ", "%03d"  ,&Step_Mult,  1,      100,    1        },    // "> STPMUL:>000< ↑"
  {12, 1,  "  CANCEL             ", ""      ,NULL,        0,      0,      0        },    // "> CANCEL        "   
  {14, 0,  "T%03d/%03d L%02d/%02d", ""      ,NULL,        0,      0,      0        },    // "T000/000 L00/00 "
  {14, 1,  "SP%03d ST0.%04d      ", ""      ,NULL,        0,      0,      0        },    // "SP000 ST0.0000  " 
  {16, 0,  "AUTOWINDING DONE     ", ""      ,NULL,        0,      0,      0        },    // "AUTOWINDING DONE" 
  {16, 1,  "PRESS CONTINUE       ", ""      ,NULL,        0,      0,      0        }};   // "PRESS CONTINUE  "
  
LiquidCrystal lcd(RS,EN,D4,D5,D6,D7); // Назначаем пины для управления LCD 
//LiquidCrystal_I2C lcd(0x27,16,2); // 0x3F I2C адрес для PCF8574AT, дисплей 16 символов 2 строки 

void setup() {
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

  lcd.begin(20,4);                                                              // Инициализация LCD Дисплей 20 символов 4 строки   
// lcd.begin(16,2);                                                             // Инициализация LCD Дисплей 16 символов 2 строки                                                              
  sprintf(Str_Buffer, Menu[0].format);
  lcd.print(Str_Buffer);                                                        // Выводим первую строку на экран
  lcd.setCursor(0,1); 
  sprintf(Str_Buffer, Menu[1].format);
  lcd.print(Str_Buffer);                                                        // Выводим вторую строку на экран
  PrintSymbol(0,0,0x3E);                                                        // Выводим символ ">" на 0,0 LCD
 
  sei();} 

void loop() {
if (Encoder_Dir != 0) {                                                       // Проверяем изменение позиции энкодера
  switch (Menu_Index) {                                                       // Если позиция энкодера изменена то меняем Menu_Index и выводим экран
    case Autowinding:  Menu_Index = constrain(Menu_Index + Encoder_Dir, Autowinding, PosControl);   break;
    case PosControl:   Menu_Index = constrain(Menu_Index + Encoder_Dir, Autowinding, PosControl);   break;
    case TurnsSet:     Menu_Index = constrain(Menu_Index + Encoder_Dir, TurnsSet, Cancel);          break;
    case StepSet:      Menu_Index = constrain(Menu_Index + Encoder_Dir, TurnsSet, Cancel);          break;  
    case SpeedSet:     Menu_Index = constrain(Menu_Index + Encoder_Dir, TurnsSet, Cancel);          break;
    case LaySet:       Menu_Index = constrain(Menu_Index + Encoder_Dir, TurnsSet, Cancel);          break;        
    case Direction:    Menu_Index = constrain(Menu_Index + Encoder_Dir, TurnsSet, Cancel);          break;        
    case Start:        Menu_Index = constrain(Menu_Index + Encoder_Dir, TurnsSet, Cancel);          break;
    case Cancel:       Menu_Index = constrain(Menu_Index + Encoder_Dir, TurnsSet, Cancel);          break;
    case ShaftPos:     Menu_Index = constrain(Menu_Index + Encoder_Dir, ShaftPos, PosCancel);       break;
    case LayPos:       Menu_Index = constrain(Menu_Index + Encoder_Dir, ShaftPos, PosCancel);       break;
    case StepMul:      Menu_Index = constrain(Menu_Index + Encoder_Dir, ShaftPos, PosCancel);       break;
    case PosCancel:    Menu_Index = constrain(Menu_Index + Encoder_Dir, ShaftPos, PosCancel);       break;}
    Encoder_Dir = 0; PrintScreen();}

if (Push_Button == true) {                                                     // Проверяем нажатие кнопки
  switch (Menu_Index)    {                                                     // Если было нажатие то выполняем действие соответствующее текущей позиции курсора
    case Autowinding:  Menu_Index = TurnsSet;                                                                                                  break;
    case PosControl:   Menu_Index = ShaftPos;                                                                                                  break;
    case TurnsSet:     SetQuote(9,13); Push_Button=false; Var_Set=true; while(!Push_Button){LCD_Print_Var();} Var_Set=false; ClearQuote(9,13); break;
    case StepSet:      SetQuote(7,14); Push_Button=false; Var_Set=true; while(!Push_Button){LCD_Print_Var();} Var_Set=false; ClearQuote(7,14); break;  
    case SpeedSet:     SetQuote(9,13); Push_Button=false; Var_Set=true; while(!Push_Button){LCD_Print_Var();} Var_Set=false; ClearQuote(9,13); break;
    case LaySet:       SetQuote(9,12); Push_Button=false; Var_Set=true; while(!Push_Button){LCD_Print_Var();} Var_Set=false; ClearQuote(9,12); break;   
    case Direction:    Push_Button=false; if (Steppers_Dir == 1) Steppers_Dir = -1; else Steppers_Dir = 1;       
                       if (Steppers_Dir == 1) {PrintSymbol(12,0,0x3E); PrintSymbol(13,0,0x3E); PrintSymbol(14,0,0x3E);} 
                       else if (Steppers_Dir == -1) {PrintSymbol(12,0,0x3C); PrintSymbol(13,0,0x3C); PrintSymbol(14,0,0x3C);}                  break;                          
    case Start:        Push_Button = false; AutoWindStart = true;  AutoWindingPrg(); AutoWindStart = false; lcd.clear();                       break; 
    case Cancel:       Menu_Index = Autowinding;                                                                                               break;
    case ShaftPos:     SetQuote(9,14); Push_Button=false; Var_Set=true; digitalWrite(EN_STEP, LOW); Motor_Num = 1; OCR1A = 200000/Step_Mult;
                       while(!Push_Button){LCD_Print_Var(); ActualShaftPos=MotorMove(*Menu[Menu_Index].param * MicroSteps, ActualShaftPos);} 
                       Var_Set=false; digitalWrite(EN_STEP, HIGH); ClearQuote(9,14);                                                           break;   
    case LayPos:       SetQuote(9,14); Push_Button=false; Var_Set=true; digitalWrite(EN_STEP, LOW); Motor_Num = 2; OCR1A = 200000/Step_Mult;
                       while(!Push_Button){LCD_Print_Var(); ActualLayerPos=MotorMove(*Menu[Menu_Index].param * MicroSteps, ActualLayerPos);} 
                       Var_Set=false; digitalWrite(EN_STEP, HIGH); ClearQuote(9,14);                                                           break;                                                               
    case StepMul:      SetQuote(9,13);Push_Button=false; Var_Set=true; while(!Push_Button){LCD_Print_Var();} Var_Set=false; ClearQuote(9,13);  break;    
    case PosCancel:    Menu_Index = Autowinding; Shaft_Pos = 0; Lay_Pos = 0; Step_Mult = 1; ActualShaftPos = 0; ActualLayerPos = 0;            break;}
    Push_Button = false; PrintScreen();}}

ISR(INT0_vect) {  // Вектор прерывания от энкодера
static byte Enc_Temp, Enc_Temp_prev;    // Временная переменная для хранения состояния порта
Enc_Temp = PIND & 0b00100100;                                                                                             // Маскируем все пины порта D кроме PD2 и PD5
     if (Enc_Temp==0b00000100 && Enc_Temp_prev==0b00100000) {Encoder_Dir = -1;}                                           // -1 - шаг против часовой
else if (Enc_Temp==0b00100100 && Enc_Temp_prev==0b00000000) {Encoder_Dir =  1;}                                           // +1 - шаг по часовой 
Enc_Temp_prev = Enc_Temp;
     if (Var_Set == true && Encoder_Dir != 0) {                                                                            // Если находимся в режиме изменения переменной 
        *Menu[Menu_Index].param += Encoder_Dir; Encoder_Dir = 0;                                                           // то меняем ее сразу и
        *Menu[Menu_Index].param = constrain(*Menu[Menu_Index].param, Menu[Menu_Index].var_Min, Menu[Menu_Index].var_Max);} // ограничиваем в диапазоне var_Min ÷ var_Max
     if (AutoWindStart == true && Encoder_Dir != 0) {Set_Speed = constrain(Set_Speed + Encoder_Dir, 1, 100);}}             // Если повернуть энкодер во время автонамотки 
                                                                                                                           // то меняем значение скорости
ISR(INT1_vect)
  {                               // Вектор прерывания от кнопки энкодера
 Push_Button = true;
 if (AutoWindStart == true) {Pause = true;}  // Если нажать кнопку энкодера во время автонамотки то выставляем флаг паузы 
 else return;
  }

void PrintScreen() {                          // Подпрограмма: Выводим экран на LCD
  static byte Prev_Screen;
  if (Menu[Menu_Index].Screen != Prev_Screen) {
  lcd.clear();
  sprintf(Str_Buffer, Menu[Menu[Menu_Index].Screen].format, *Menu[Menu[Menu_Index].Screen].param * Menu[Menu[Menu_Index].Screen].param_coef);
  lcd.print(Str_Buffer);
  lcd.setCursor(0, 1); 
  sprintf(Str_Buffer, Menu[Menu[Menu_Index].Screen + 1].format, *Menu[Menu[Menu_Index].Screen + 1].param * Menu[Menu[Menu_Index].Screen + 1].param_coef);
  lcd.print(Str_Buffer);
  Prev_Screen = Menu[Menu_Index].Screen;}
  if      (Menu_Index & 1) {PrintSymbol(0,1,0x3E); PrintSymbol(0,0,0x20);}  // Если индекс меню нечетный выводим курсор на вторую строку
  else                     {PrintSymbol(0,0,0x3E); PrintSymbol(0,1,0x20);}  // Иначе выводим курсор на первую строку  
  switch (Menu_Index) {                                                     // Выводим стрелки ⯅⯆ на соответствующих строках меню
    case TurnsSet:     PrintSymbol(15,1,1);                      break;
    case StepSet:      PrintSymbol(15,1,1);                      break;  
    case SpeedSet:     PrintSymbol(15,1,1); PrintSymbol(15,0,0); break;
    case LaySet:       PrintSymbol(15,1,1); PrintSymbol(15,0,0); break;  
    case Direction:    PrintSymbol(15,1,1); PrintSymbol(15,0,0); break;    
    case Start:        PrintSymbol(15,1,1); PrintSymbol(15,0,0); break;       
    case Cancel:       PrintSymbol(15,0,0);                      break;    
    case ShaftPos:     PrintSymbol(15,1,1);                      break;
    case LayPos:       PrintSymbol(15,1,1);                      break;
    case StepMul:      PrintSymbol(15,0,0);                      break;
    case PosCancel:    PrintSymbol(15,0,0);                      break;}}
                                                                                                          
void PrintSymbol(byte LCD_Column, byte LCD_Row, byte Symbol_Code) { // Подпрограмма: Выводим символ на экран
  lcd.setCursor(LCD_Column, LCD_Row); 
  lcd.write(byte(Symbol_Code));}

void SetQuote   (int First_Cur, int Second_Cur) {                 // Подпрограмма: Выводим выделение изменяемой переменной на LCD
  PrintSymbol(First_Cur,  Menu[Menu_Index].string_number,0x3E);   // Выводим символ >
  PrintSymbol(Second_Cur, Menu[Menu_Index].string_number,0x3C);   // Выводим символ <
  PrintSymbol(0,          Menu[Menu_Index].string_number,0x20);}  // Стираем основной курсор

void ClearQuote (int First_Cur, int Second_Cur) {                 // Подпрограмма: Стираем выделение изменяемой переменной на LCD
  PrintSymbol(First_Cur,  Menu[Menu_Index].string_number,0x20);   // Стираем символ >
  PrintSymbol(Second_Cur, Menu[Menu_Index].string_number,0x20);   // Стираем символ <
  PrintSymbol(0,          Menu[Menu_Index].string_number,0x3E);}  // Выводим основной курсор     

void LCD_Print_Var() {                                                  // Подпрограмма: Выводим новое значение переменной на LCD
  static int Previous_Param;
    if (*Menu[Menu_Index].param != Previous_Param){
      lcd.setCursor(10, Menu[Menu_Index].string_number);
      sprintf(Str_Buffer, Menu[Menu_Index].format_Set_var, *Menu[Menu_Index].param * Menu[Menu_Index].param_coef);
      lcd.print(Str_Buffer);
      Previous_Param = *Menu[Menu_Index].param;}}

void PrintWendingScreen() { // Подпрограмма вывода экрана автонамотки
  lcd.clear();
  sprintf(Str_Buffer, Menu[14].format, Actual_Turn, Set_Turns, Actual_Layer, Set_Layers);
  lcd.print(Str_Buffer);
  lcd.setCursor(0, 1);
  sprintf(Str_Buffer, Menu[15].format, Set_Speed, Set_Step*ShaftStep);
  lcd.print(Str_Buffer);  }

void AutoWindingPrg() {                                             // Подпрограмма автоматической намотки

  digitalWrite(EN_STEP, LOW);   // Разрешение управления двигателями
  digitalWrite(DIR_Z, HIGH);  
  if (Steppers_Dir == 1) PORTB &= 0b11011111; 
  else if (Steppers_Dir == -1) PORTB |= 0b00100000;
  PrintWendingScreen();
  Push_Button = false; 
  while (Actual_Layer < Set_Layers)                                 // Пока текущее кол-во слоев меньше заданного проверяем сколько сейчас витков
    {
          OCR1A = 65535;
          OCR1A_NOM = 4800000 / (Set_Speed*MicroSteps); 
          TIMSK1=2;
      while (Actual_Turn < Set_Turns)                               // Пока текущее кол-во витков меньше заданного продолжаем мотать
        {           
          if (Pause == true)                                                // Проверяем не нужно ли сделать паузу
            {
              TIMSK1=0;
              Push_Button = false; 
              Pause = false;
              lcd.setCursor(0, 1); 
              sprintf(Str_Buffer, "PRESS CONTINUE  ");                         // "PRESS CONTINUE  "
              lcd.print(Str_Buffer); 
              while (!Push_Button){} 
              lcd.setCursor(0, 1); 
              sprintf(Str_Buffer, Menu[15].format, Set_Speed, Set_Step*ShaftStep); // "SPXXX ST0.XXXX  "
              lcd.print(Str_Buffer);           
              Push_Button = false; 
              Pause = false;
              TIMSK1=2;
            }
          sprintf(Str_Buffer, "%03d", Actual_Turn); 
          lcd.setCursor(1, 0); 
          lcd.print(Str_Buffer);    
        }
      TIMSK1=0;
      
      sprintf(Str_Buffer, "%03d", Actual_Turn); 
      lcd.setCursor(1, 0); 
      lcd.print(Str_Buffer);
      
      Actual_Layer++;
      lcd.setCursor(10, 0); 
      sprintf(Str_Buffer, "%02d", Actual_Layer);
      lcd.print(Str_Buffer);
      
      if (Actual_Layer == Set_Layers) continue; 
      
      lcd.setCursor(0, 1); 
      sprintf(Str_Buffer, "PRESS CONTINUE  ");                            // "PRESS CONTINUE  "
      lcd.print(Str_Buffer);
      
      Push_Button = false;
      while(!Push_Button){}
      Push_Button = false; 
      
      lcd.setCursor(0, 1);
      sprintf(Str_Buffer, Menu[15].format, Set_Speed, Set_Step*ShaftStep);
      lcd.print(Str_Buffer);
      
      if (Steppers_Dir == 1) Steppers_Dir = -1;
      else Steppers_Dir = 1; 
      if (Steppers_Dir == 1) PORTB &= 0b11011111; 
      else if (Steppers_Dir == -1) PORTB |= 0b00100000;
      Actual_Turn = 0;        
      sprintf(Str_Buffer, "%03d", Actual_Turn); 
      lcd.setCursor(1, 0); 
      lcd.print(Str_Buffer);
      TIMSK1=2;        
     }
    digitalWrite(EN_STEP, HIGH);
    
    lcd.setCursor(0, 1); 
    sprintf(Str_Buffer, "AUTOWINDING DONE");                             // "AUTOWINDING DONE"
    lcd.print(Str_Buffer);
    Push_Button = false;
    while (Push_Button == false) {}
    Push_Button = false;
    Pause = false;
    Menu_Index = Autowinding; 
    Actual_Layer = 0;     
}

int MotorMove(int32_t Move_Var, int32_t Actual_Rot) {                    // Подпрограмма: Движение шагового двигателя до заданной координаты
long Rotation;        
  Rotation = Move_Var * Step_Mult - Actual_Rot;
  switch(Motor_Num) {
    case 1: if      (Rotation > 0) {PORTD |= 0b10000000; TCNT1=0; TIMSK1=2; while(i<2){} TIMSK1=0; TCNT1=0; Actual_Rot++; i=0; DC=false;} 
            else if (Rotation < 0) {PORTD &= 0b01111111; TCNT1=0; TIMSK1=2; while(i<2){} TIMSK1=0; TCNT1=0; Actual_Rot--; i=0; DC=false;} 
            else     TIMSK1 = 0; i = 0; DC = false; break; 
    case 2: if      (Rotation > 0) {PORTB |= 0b00100000; TCNT1=0; TIMSK1=2; while(i<2){} TIMSK1=0; TCNT1=0; Actual_Rot++; i=0; DC=false;} 
            else if (Rotation < 0) {PORTB &= 0b11011111; TCNT1=0; TIMSK1=2; while(i<2){} TIMSK1=0; TCNT1=0; Actual_Rot--; i=0; DC=false;}
            else     TIMSK1 = 0; i = 0; DC = false; break;}                    
return Actual_Rot;}

/*
void MotorStep(int StepQuant, int Dir) {                          // Подпрограмма: Движение шагового двигателя на заданное число шагов
int Temp_Step = 0;
  switch(Motor_Num) {
    case 1:      if (Dir == 1)  {PORTD |= 0b10000000;} 
            else if (Dir == -1) {PORTD &= 0b01111111;}  
    case 2:      if (Dir == 1)  {PORTB |= 0b00100000;}  
            else if (Dir == -1) {PORTB &= 0b11011111;}}                           
while (Temp_Step < StepQuant) {i=0; TCNT1=0; TIMSK1=2; while(i<2){} TIMSK1=0; TCNT1=0; DC=false; Temp_Step++;}}
*/


ISR(TIMER1_COMPA_vect) {                      // Вектор прерывания от таймера/счетчика 1
  
 if (AutoWindStart) 
 {
  Motor_Num = 0;
  if (NSteps < 200 * MicroSteps) 
  {
   PORTD |= 0b00010000;
   if (NTurn>>4 > 200 - Set_Step) PORTB |= 0b00010000;
   while (i<6) {i++;} i=0;
   PORTD &= 0b11101111; 
   if (NTurn>>4 > 200 - Set_Step) PORTB &= 0b11101111;
   NTurn++;
   if (NTurn>>4 > 200) {NTurn=0; Actual_Turn++;}

  INCR = Set_Speed * 5 / (MicroSteps);
  Temp = NSteps * INCR;
  OCR1A_TEMP = 300000 * 1000 / Temp;
  OCR1A = min (65535, OCR1A_TEMP);
  } 
  if (NSteps >= 200 * MicroSteps) 
  {
   OCR1A = OCR1A_NOM;
   PORTD |= 0b00010000;
   if (NTurn>>4 > 200 - Set_Step) PORTB |= 0b00010000;
   while (i<6) {i++;} i=0;
   PORTD &= 0b11101111; 
   if (NTurn>>4 > 200 - Set_Step) PORTB &= 0b11101111;
   NTurn++;
   if (NTurn>>4 > 200) {NTurn=0; Actual_Turn++;}
  }
    NSteps++;
  }
  
  i++;                                        // Счетчик кол-ва заходов в прерывание
  DC =! DC;                                   // Первое прерывание устанавливает STEP следующее - сбрасывает
    if      (Motor_Num == 1) {
      if (DC == true) {PORTD |= 0b00010000;}  // STEP_Z
      else            {PORTD &= 0b11101111;}}
    else if (Motor_Num == 2) {
      if (DC == true) {PORTB |= 0b00010000;}  // STEP_A
      else            {PORTB &= 0b11101111;}}}








      
