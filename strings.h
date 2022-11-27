#pragma once

// ›ÚÓÚ Ù‡ÈÎ ÒÓı‡ÌÂÌ ‚ ÍÓ‰ËÓ‚ÍÂ Windows 1251
// This file is saved in Windows 1251 encoding



#if LANGUAGE == ru

const char STRING_ON[] = "¬ À ";
const char STRING_OFF[] = "¬€ À";

const char PROGMEM LINE1_FORMAT[] = "T%03d/%03d L%02d/%02d";
const char PROGMEM LINE2_FORMAT[] = "Sp%03d St0.%04d";
const char PROGMEM LINE4_FORMAT[] = "%03d";
const char PROGMEM LINE5_FORMAT[] = "%02d";
const char PROGMEM LINE6_FORMAT[] = "%03d";

const char PROGMEM STRING_1[] = "AUTOWINDING DONE";
const char PROGMEM STRING_2[] = "PRESS CONTINUE  ";

const char PROGMEM MENU_01[] = "√–”œœ¿ 1";
const char PROGMEM MENU_02[] = "√–”œœ¿ 2";
const char PROGMEM MENU_03[] = "√–”œœ¿ 3";
const char PROGMEM MENU_04[] = "œŒ«»÷»»";
const char PROGMEM MENU_05[] = "Õ¿—“–Œ… »";
const char PROGMEM MENU_06[] = "Œ¡ÃŒ“ ¿ 1";
const char PROGMEM MENU_07[] = "Œ¡ÃŒ“ ¿ 2";
const char PROGMEM MENU_08[] = "Œ¡ÃŒ“ ¿ 3";
const char PROGMEM MENU_09[] = "Õ¿«¿ƒ";  
const char PROGMEM MENU_10[] = "¬»“ Œ¬:";
const char PROGMEM MENU_11[] = "ÿ¿√:";
const char PROGMEM MENU_12[] = "RPM:";
const char PROGMEM MENU_13[] = "—ÀŒ≈¬:";
const char PROGMEM MENU_14[] = "Õ¿œ–¿¬À.";
const char PROGMEM MENU_15[] = "—“¿–“";
const char PROGMEM MENU_17[] = " ¿“”ÿ ¿:";
const char PROGMEM MENU_18[] = "ÿ¿√:";
const char PROGMEM MENU_19[] = "œŒƒ¿◊¿:";
const char PROGMEM MENU_22[] = "—“Œœ—ÀŒ…";
const char PROGMEM MENU_23[] = "”— Œ–.";

#else

const char STRING_ON[] = "ON ";
const char STRING_OFF[] = "OFF";

const char PROGMEM LINE1_FORMAT[] = "T%03d/%03d L%02d/%02d";
const char PROGMEM LINE2_FORMAT[] = "Sp%03d St0.%04d";
const char PROGMEM LINE4_FORMAT[] = "%03d";
const char PROGMEM LINE5_FORMAT[] = "%02d";
const char PROGMEM LINE6_FORMAT[] = "%03d";

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
