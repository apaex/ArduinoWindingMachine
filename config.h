#pragma once

#define ENC_CLK   2
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

#define THREAD_PITCH        1000         // Шаг резьбы вала укладчика в мкм

#define DISPLAY_NCOL        20           // размер дисплея: ширина
#define DISPLAY_NROW        4            // размер дисплея: высота
#define DISPLAY_ADDRESS     0x27         // I2C адрес дисплея (0x27 для PCF8574T, 0x3F для PCF8574AT)

#define STEPPERS_STEPS      200          // число шагов двигателя на 1 оборот
#define STEPPERS_MICROSTEPS 16           // делитель на плате драйвера двигателя

#define ENCODER_TYPE        EB_HALFSTEP  // тип энкодера: EB_FULLSTEP или EB_HALFSTEP. если энкодер делает один поворот за два щелчка, нужно изменить настройку
#define ENCODER_INPUT       INPUT        // если есть подтягивающие резисторы - ставь INPUT, если нет - INPUT_PULLUP
