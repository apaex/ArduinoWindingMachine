#pragma once

// ›ÚÓÚ Ù‡ÈÎ ÒÓı‡ÌÂÌ ‚ ÍÓ‰ËÓ‚ÍÂ Windows 1251
// This file is saved in Windows 1251 encoding



#if LANGUAGE == ru

const char PROGMEM LINE1_FORMAT[] = "T%03d/%03d L%02d/%02d";
const char PROGMEM LINE2_FORMAT[] = "Sp%03d St0.%04d";
const char PROGMEM LINE4_FORMAT[] = "%03d";
const char PROGMEM LINE5_FORMAT[] = "%02d";
const char PROGMEM LINE6_FORMAT[] = "%03d";

const char PROGMEM STRING_1[] = "AUTOWINDING DONE";
const char PROGMEM STRING_2[] = "PRESS CONTINUE  ";

const char MENU_01[] = "”—“¿ÕŒ¬ ¿ 1";
const char MENU_02[] = "”—“¿ÕŒ¬ ¿ 2";
const char MENU_03[] = "”—“¿ÕŒ¬ ¿ 3";
const char MENU_04[] = "œŒ«»÷»»";
const char MENU_05[] = "Õ¿—“–Œ… »";
const char MENU_06[] = "Œ¡ÃŒ“ ¿ 1";
const char MENU_07[] = "Œ¡ÃŒ“ ¿ 2";
const char MENU_08[] = "Œ¡ÃŒ“ ¿ 3";
const char MENU_09[] = "Õ¿«¿ƒ";  
const char MENU_10[] = "¬»“ Œ¬:";
const char MENU_11[] = "ÿ¿√:";
const char MENU_12[] = "RPM:";
const char MENU_13[] = "—ÀŒ≈¬:";
const char MENU_14[] = "Õ¿œ–¿¬À.";
const char MENU_15[] = "—“¿–“";
const char MENU_17[] = " ¿“”ÿ ¿:";
const char MENU_18[] = "ÿ¿√:";
const char MENU_19[] = "œŒƒ¿◊¿:";
const char MENU_22[] = "—“Œœ—ÀŒ…";

#else

const char PROGMEM LINE1_FORMAT[] = "T%03d/%03d L%02d/%02d";
const char PROGMEM LINE2_FORMAT[] = "Sp%03d St0.%04d";
const char PROGMEM LINE4_FORMAT[] = "%03d";
const char PROGMEM LINE5_FORMAT[] = "%02d";
const char PROGMEM LINE6_FORMAT[] = "%03d";

const char PROGMEM STRING_1[] = "AUTOWINDING DONE";
const char PROGMEM STRING_2[] = "PRESS CONTINUE  ";

const char MENU_01[] = "Setup 1";
const char MENU_02[] = "Setup 2";
const char MENU_03[] = "Setup 3";
const char MENU_04[] = "Pos control";
const char MENU_05[] = "Settings";
const char MENU_06[] = "Winding 1";
const char MENU_07[] = "Winding 2";
const char MENU_08[] = "Winding 3";
const char MENU_09[] = "Back";  
const char MENU_10[] = "Turns:";
const char MENU_11[] = "Step:";
const char MENU_12[] = "Speed:";
const char MENU_13[] = "Layers:";
const char MENU_14[] = "Direction";
const char MENU_15[] = "Start";
const char MENU_17[] = "SH pos:";
const char MENU_18[] = "StpMul:";
const char MENU_19[] = "LA pos:";
const char MENU_22[] = "LayerStop";

#endif
