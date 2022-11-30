#pragma once

// щРНР ТЮИК ЯНУПЮМЕМ Б ЙНДХПНБЙЕ Windows 1251
// This file is saved in Windows 1251 encoding

#define EN 1
#define RU 2

const char PROGMEM LINE1_FORMAT[] = "T%03d/%03d L%02d/%02d";
const char PROGMEM LINE2_FORMAT[] = "Sp%03d St0.%04d";
const char PROGMEM LINE4_FORMAT[] = "%03d";
const char PROGMEM LINE5_FORMAT[] = "%02d";
const char PROGMEM LINE6_FORMAT[] = "%03d";

const char PROGMEM MENU_FORMAT_06[] = "% 4dT";
const char PROGMEM MENU_FORMAT_10[] = "%03d";
const char PROGMEM MENU_FORMAT_13[] = "%02d";
const char PROGMEM MENU_FORMAT_11[] = "0.%03d";
const char PROGMEM MENU_FORMAT_17[] = "%+04d";

#if LANGUAGE == RU

const char STRING_ON[] = "бйк ";
const char STRING_OFF[] = "бшйк";

const char PROGMEM STRING_1[] = "мюлнрйю нйнмвемю";
const char PROGMEM STRING_2[] = "мюфлхре щмйндеп ";

const char PROGMEM MENU_01[] = "цпсоою 1";
const char PROGMEM MENU_02[] = "цпсоою 2";
const char PROGMEM MENU_03[] = "цпсоою 3";
const char PROGMEM MENU_04[] = "онгхжхх";
const char PROGMEM MENU_05[] = "мюярпнийх";
const char PROGMEM MENU_06[] = "налнрйю 1";
const char PROGMEM MENU_07[] = "налнрйю 2";
const char PROGMEM MENU_08[] = "налнрйю 3";
const char PROGMEM MENU_09[] = "мюгюд";  
const char PROGMEM MENU_10[] = "бхрйнб:";
const char PROGMEM MENU_11[] = "ьюц:";
const char PROGMEM MENU_12[] = "RPM:";
const char PROGMEM MENU_13[] = "якнеб:";
const char PROGMEM MENU_14[] = "мюопюбк.";
const char PROGMEM MENU_15[] = "ярюпр";
const char PROGMEM MENU_17[] = "йюрсьйю:";
const char PROGMEM MENU_18[] = "ьюц:";
const char PROGMEM MENU_19[] = "ондювю:";
const char PROGMEM MENU_22[] = "ярноякни";
const char PROGMEM MENU_23[] = "сяйнпем.";




#else

const char STRING_ON[] = "ON ";
const char STRING_OFF[] = "OFF";

const char PROGMEM STRING_1[] = "AUTOWINDING DONE";
const char PROGMEM STRING_2[] = "PRESS CONTINUE  ";

const char PROGMEM MENU_01[] = "Setup 1";
const char PROGMEM MENU_02[] = "Setup 2";
const char PROGMEM MENU_03[] = "Setup 3";
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

#endif
