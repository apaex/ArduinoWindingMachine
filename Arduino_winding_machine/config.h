#pragma once

#define ENCODER_CLK         2
#define ENCODER_DT          5 
#define ENCODER_SW          3

#define STEPPERS_STEP_Z     4 
#define STEPPERS_DIR_Z      7
#define STEPPERS_STEP_A     12
#define STEPPERS_DIR_A      13
#define STEPPERS_EN         8

#define BUZZ_OUT            10
#define BUTTON_STOP         11

#define DISPLAY_RS          14
#define DISPLAY_EN          15
#define DISPLAY_D4          16
#define DISPLAY_D5          17
#define DISPLAY_D6          18
#define DISPLAY_D7          19

#define THREAD_PITCH        1000         // Шаг резьбы вала укладчика в мкм

#define DISPLAY_NCOL        20           // размер дисплея: ширина
#define DISPLAY_NROW        4            // размер дисплея: высота
#define DISPLAY_I2C         0            // использовать I2C подключение дисплея
#define DISPLAY_ADDRESS     0x27         // I2C адрес дисплея (0x27 для PCF8574T, 0x3F для PCF8574AT)

#define STEPPERS_STEPS      200          // число шагов двигателя на 1 оборот
#define STEPPERS_MICROSTEPS 16           // делитель на плате драйвера двигателя

#define ENCODER_TYPE        EB_HALFSTEP  // тип энкодера: EB_FULLSTEP или EB_HALFSTEP. если энкодер делает один поворот за два щелчка, нужно изменить настройку
#define ENCODER_INPUT       INPUT        // если есть подтягивающие резисторы - ставьте INPUT, если нет - INPUT_PULLUP

#define LANGUAGE            RU           // EN, RU
//#define DEBUG

