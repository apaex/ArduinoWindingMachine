#pragma once
// сохраняйте этот файл при обновлении прошивки, чтобы не потерялись все ваши настройки. Старый файл конфигурации будет подходить ко всем новым версиям

#define ENCODER_CLK         2
#define ENCODER_DT          5 
#define ENCODER_SW          3

#define STEPPER_STEP_Z      4 
#define STEPPER_DIR_Z       7
#define STEPPER_STEP_A      12
#define STEPPER_DIR_A       13
#define STEPPER_EN          8

#define BUZZER              10
#define BUTTON_STOP         11

#define DISPLAY_RS          A0
#define DISPLAY_EN          A1
#define DISPLAY_D4          A2
#define DISPLAY_D5          A3
#define DISPLAY_D6          A4
#define DISPLAY_D7          A5

#define KEYBOARD_PIN        A6           // для подключения аналоговой клавиатуры на A0 необходимо перенести вывод дисплея DISPLAY_RS на другой пин, например, на 6


#define THREAD_PITCH        1000         // шаг резьбы вала укладчика в мкм

#define DISPLAY_NCOL        20           // размер дисплея: ширина
#define DISPLAY_NROW        4            // размер дисплея: высота
#define DISPLAY_I2C         0            // использовать I2C подключение дисплея (1 - да, 0 - нет)
#define DISPLAY_ADDRESS     0x27         // I2C адрес дисплея (0x27 для PCF8574T, 0x3F для PCF8574AT)

#define STEPPER_Z_STEPS       200          // число шагов двигателя катушки на 1 оборот
#define STEPPER_Z_MICROSTEPS  16           // делитель на плате драйвера двигателя катушки
#define STEPPER_Z_REVERSE     1            // обратить направление вращения для двигателя катушки (1 - да, 0 - нет)
#define STEPPER_A_STEPS       200          // число шагов двигателя укладчика на 1 оборот
#define STEPPER_A_MICROSTEPS  16           // делитель на плате драйвера двигателя укладчика
#define STEPPER_A_REVERSE     0            // обратить направление вращения для двигателя укладчика (1 - да, 0 - нет)

#define ENCODER_TYPE        EB_HALFSTEP  // тип энкодера: EB_FULLSTEP или EB_HALFSTEP. если энкодер делает один шаг за два щелчка, нужно изменить настройку
#define ENCODER_INPUT       INPUT        // если есть подтягивающие резисторы - ставьте INPUT, если нет - INPUT_PULLUP

#define KEYBOARD_LEFT       0            // 0 
#define KEYBOARD_UP         33           // k33
#define KEYBOARD_DOWN       93           // 1k0
#define KEYBOARD_RIGHT      171          // 2k0
#define KEYBOARD_SELECT     350          // 5k2

#define LANGUAGE            RU           // EN, RU
//#define DEBUG
#define TRANSFORMER_COUNT   3            // 36 max
