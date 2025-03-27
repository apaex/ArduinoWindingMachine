#pragma once

// Этот файл сохранен в кодировке Windows 1251
// This file is saved in Windows 1251 encoding

#define EN 1
#define RU 2

const char PROGMEM LINE1_FORMAT[] = "T%04d/%04d %03drpm";
const char PROGMEM LINE2_FORMAT[] = "L%03d/%03d %d.%03dmm";
const char PROGMEM LINE4_FORMAT[] = "%04d";
const char PROGMEM LINE5_FORMAT[] = "%03d";
const char PROGMEM LINE6_FORMAT[] = "%03d";

const char PROGMEM MENU_FORMAT_02[] = "%d";
const char PROGMEM MENU_FORMAT_06[] = "% 4dT";
const char PROGMEM MENU_FORMAT_10[] = "%04d";
const char PROGMEM MENU_FORMAT_13[] = "%03d";
const char PROGMEM MENU_FORMAT_11[] = "%d.%03d";
const char PROGMEM MENU_FORMAT_17[] = "%+04d";


#if LANGUAGE == RU

const char STRING_ON[] = "ВКЛ ";
const char STRING_OFF[] = "ВЫКЛ";

const char PROGMEM STRING_1[] = "НАМОТКА ОКОНЧЕНА";
const char PROGMEM STRING_2[] = "НАЖМИТЕ ЭНКОДЕР";
const char PROGMEM STRING_3[] = "ОБМОТКА %d СТАРТ";

const char PROGMEM MENU_01[] = "АВТОНАМОТКА";
const char PROGMEM MENU_02[] = "КОНФИГ";
const char PROGMEM MENU_04[] = "ПОЗИЦИИ";
const char PROGMEM MENU_05[] = "НАСТРОЙКИ";
const char PROGMEM MENU_06[] = "ОБМОТКА 1";
const char PROGMEM MENU_07[] = "ОБМОТКА 2";
const char PROGMEM MENU_08[] = "ОБМОТКА 3";
const char PROGMEM MENU_09[] = "НАЗАД";  
const char PROGMEM MENU_10[] = "ВИТКОВ:";
const char PROGMEM MENU_11[] = "ШАГ:";
const char PROGMEM MENU_12[] = "RPM:";
const char PROGMEM MENU_13[] = "СЛОЕВ:";
const char PROGMEM MENU_14[] = "НАПРАВЛ.";
const char PROGMEM MENU_15[] = "СТАРТ";
const char PROGMEM MENU_17[] = "КАТУШКА:";
const char PROGMEM MENU_18[] = "ШАГ:";
const char PROGMEM MENU_19[] = "ПОДАЧА:";
const char PROGMEM MENU_22[] = "СЛОЙСТОП";
const char PROGMEM MENU_23[] = "УСКОРЕН.";

const char PROGMEM PLANNER_STATUS_0[] = "ПАУЗА";
const char PROGMEM PLANNER_STATUS_1[] = "НАМОТКА";
const char PROGMEM PLANNER_STATUS_2[] = "ОСТАНОВКА";
const char PROGMEM PLANNER_STATUS_3[] = "";
const char PROGMEM PLANNER_STATUS_4[] = "ТОРМОЖЕНИЕ";

#else

const char STRING_ON[] = "ON ";
const char STRING_OFF[] = "OFF";

const char PROGMEM STRING_1[] = "AUTOWINDING DONE";
const char PROGMEM STRING_2[] = "PRESS CONTINUE";
const char PROGMEM STRING_3[] = "WINDING %d START";

const char PROGMEM MENU_01[] = "Autowinding";
const char PROGMEM MENU_02[] = "Setup";
const char PROGMEM MENU_04[] = "Pos control";
const char PROGMEM MENU_05[] = "Settings";
const char PROGMEM MENU_06[] = "Winding 1";
const char PROGMEM MENU_07[] = "Winding 2";
const char PROGMEM MENU_08[] = "Winding 3";
const char PROGMEM MENU_09[] = "Back";  
const char PROGMEM MENU_10[] = "Turns:";
const char PROGMEM MENU_11[] = "Step:";
const char PROGMEM MENU_12[] = "Speed:";
const char PROGMEM MENU_13[] = "Layers:";
const char PROGMEM MENU_14[] = "Direction";
const char PROGMEM MENU_15[] = "Start";
const char PROGMEM MENU_17[] = "SH pos:";
const char PROGMEM MENU_18[] = "StpMul:";
const char PROGMEM MENU_19[] = "LA pos:";
const char PROGMEM MENU_22[] = "LayerStop";
const char PROGMEM MENU_23[] = "Accel";

const char PROGMEM PLANNER_STATUS_0[] = "Pause";
const char PROGMEM PLANNER_STATUS_1[] = "Run";
const char PROGMEM PLANNER_STATUS_2[] = "Wait";
const char PROGMEM PLANNER_STATUS_3[] = "";
const char PROGMEM PLANNER_STATUS_4[] = "Brake";

#endif

const char* const plannerStatuses[] = {PLANNER_STATUS_0, PLANNER_STATUS_1, PLANNER_STATUS_2, PLANNER_STATUS_3, PLANNER_STATUS_4};
