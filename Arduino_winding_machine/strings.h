#pragma once

// ›ÚÓÚ Ù‡ÈÎ ÒÓı‡ÌÂÌ ‚ ÍÓ‰ËÓ‚ÍÂ Windows 1251
// This file is saved in Windows 1251 encoding

#define EN 1
#define RU 2

const char PROGMEM LINE1_FORMAT[] = "T%03d/%03d %03drpm";
const char PROGMEM LINE2_FORMAT[] = "L%02d/%02d %d.%03dmm";
const char PROGMEM LINE4_FORMAT[] = "%03d";
const char PROGMEM LINE5_FORMAT[] = "%02d";
const char PROGMEM LINE6_FORMAT[] = "%03d";

const char PROGMEM MENU_FORMAT_02[] = "%d";
const char PROGMEM MENU_FORMAT_06[] = "% 4dT";
const char PROGMEM MENU_FORMAT_10[] = "%03d";
const char PROGMEM MENU_FORMAT_13[] = "%04d";
const char PROGMEM MENU_FORMAT_11[] = "%d.%03d";
const char PROGMEM MENU_FORMAT_17[] = "%+04d";


#if LANGUAGE == RU

const char STRING_ON[] = "¬ À ";
const char STRING_OFF[] = "¬€ À";

const char PROGMEM STRING_1[] = "Õ¿ÃŒ“ ¿ Œ ŒÕ◊≈Õ¿";
const char PROGMEM STRING_2[] = "Õ¿∆Ã»“≈ ›Õ Œƒ≈–";
const char PROGMEM STRING_3[] = "Œ¡ÃŒ“ ¿ %d —“¿–“";

const char PROGMEM MENU_01[] = "¿¬“ŒÕ¿ÃŒ“ ¿";
const char PROGMEM MENU_02[] = " ŒÕ‘»√";
const char PROGMEM MENU_04[] = "œŒ«»÷»»";
const char PROGMEM MENU_05[] = "Õ¿—“–Œ… »";
const char PROGMEM MENU_06[] = "Œ¡ÃŒ“ ¿ 1";
const char PROGMEM MENU_07[] = "Œ¡ÃŒ“ ¿ 2";
const char PROGMEM MENU_08[] = "Œ¡ÃŒ“ ¿ 3";
const char PROGMEM MENU_09[] = "Õ¿«¿ƒ";  
const char PROGMEM MENU_10[] = "Õ¿ —ÀŒ…:";
const char PROGMEM MENU_11[] = "ÿ¿√:";
const char PROGMEM MENU_12[] = "RPM:";
const char PROGMEM MENU_13[] = "¬»“ Œ¬:";
const char PROGMEM MENU_14[] = "Õ¿œ–¿¬À.";
const char PROGMEM MENU_15[] = "—“¿–“";
const char PROGMEM MENU_17[] = " ¿“”ÿ ¿:";
const char PROGMEM MENU_18[] = "ÿ¿√:";
const char PROGMEM MENU_19[] = "œŒƒ¿◊¿:";
const char PROGMEM MENU_22[] = "—ÀŒ…—“Œœ";
const char PROGMEM MENU_23[] = "”— Œ–≈Õ.";

const char PROGMEM PLANNER_STATUS_0[] = "œ¿”«¿";
const char PROGMEM PLANNER_STATUS_1[] = "Õ¿ÃŒ“ ¿";
const char PROGMEM PLANNER_STATUS_2[] = "Œ—“¿ÕŒ¬ ¿";
const char PROGMEM PLANNER_STATUS_3[] = "";
const char PROGMEM PLANNER_STATUS_4[] = "“Œ–ÃŒ∆≈Õ»≈";

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
