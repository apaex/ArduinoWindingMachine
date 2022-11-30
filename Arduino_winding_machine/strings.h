#pragma once

// Ýòîò ôàéë ñîõðàíåí â êîäèðîâêå Windows 1251
// This file is saved in Windows 1251 encoding

#define EN 1
#define RU 2

const char PROGMEM LINE1_FORMAT[] = "T%03d/%03d L%02d/%02d";
const char PROGMEM LINE2_FORMAT[] = "Sp%03d St0.%03d";
const char PROGMEM LINE4_FORMAT[] = "%03d";
const char PROGMEM LINE5_FORMAT[] = "%02d";
const char PROGMEM LINE6_FORMAT[] = "%03d";

const char PROGMEM MENU_FORMAT_06[] = "% 4dT";
const char PROGMEM MENU_FORMAT_10[] = "%03d";
const char PROGMEM MENU_FORMAT_13[] = "%02d";
const char PROGMEM MENU_FORMAT_11[] = "0.%03d";
const char PROGMEM MENU_FORMAT_17[] = "%+04d";

#if LANGUAGE == RU

const char STRING_ON[] = "ÂÊË ";
const char STRING_OFF[] = "ÂÛÊË";

const char PROGMEM STRING_1[] = "ÍÀÌÎÒÊÀ ÎÊÎÍ×ÅÍÀ";
const char PROGMEM STRING_2[] = "ÍÀÆÌÈÒÅ ÝÍÊÎÄÅÐ ";

const char PROGMEM MENU_01[] = "ÃÐÓÏÏÀ 1";
const char PROGMEM MENU_02[] = "ÃÐÓÏÏÀ 2";
const char PROGMEM MENU_03[] = "ÃÐÓÏÏÀ 3";
const char PROGMEM MENU_04[] = "ÏÎÇÈÖÈÈ";
const char PROGMEM MENU_05[] = "ÍÀÑÒÐÎÉÊÈ";
const char PROGMEM MENU_06[] = "ÎÁÌÎÒÊÀ 1";
const char PROGMEM MENU_07[] = "ÎÁÌÎÒÊÀ 2";
const char PROGMEM MENU_08[] = "ÎÁÌÎÒÊÀ 3";
const char PROGMEM MENU_09[] = "ÍÀÇÀÄ";  
const char PROGMEM MENU_10[] = "ÂÈÒÊÎÂ:";
const char PROGMEM MENU_11[] = "ØÀÃ:";
const char PROGMEM MENU_12[] = "RPM:";
const char PROGMEM MENU_13[] = "ÑËÎÅÂ:";
const char PROGMEM MENU_14[] = "ÍÀÏÐÀÂË.";
const char PROGMEM MENU_15[] = "ÑÒÀÐÒ";
const char PROGMEM MENU_17[] = "ÊÀÒÓØÊÀ:";
const char PROGMEM MENU_18[] = "ØÀÃ:";
const char PROGMEM MENU_19[] = "ÏÎÄÀ×À:";
const char PROGMEM MENU_22[] = "ÑÒÎÏÑËÎÉ";
const char PROGMEM MENU_23[] = "ÓÑÊÎÐÅÍ.";

const char PROGMEM PLANNER_STATUS_0[] = "ÏÀÓÇÀ";
const char PROGMEM PLANNER_STATUS_1[] = "ÍÀÌÎÒÊÀ";
const char PROGMEM PLANNER_STATUS_2[] = "ÎÑÒÀÍÎÂÊÀ";
const char PROGMEM PLANNER_STATUS_3[] = "";
const char PROGMEM PLANNER_STATUS_4[] = "ÒÎÐÌÎÆÅÍÈÅ";

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

const char PROGMEM PLANNER_STATUS_0[] = "Pause";
const char PROGMEM PLANNER_STATUS_1[] = "Run";
const char PROGMEM PLANNER_STATUS_2[] = "Wait";
const char PROGMEM PLANNER_STATUS_3[] = "";
const char PROGMEM PLANNER_STATUS_4[] = "Brake";

#endif

const char* const plannerStatuses[] = {PLANNER_STATUS_0, PLANNER_STATUS_1, PLANNER_STATUS_2, PLANNER_STATUS_3, PLANNER_STATUS_4};
