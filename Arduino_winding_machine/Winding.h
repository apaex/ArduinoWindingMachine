#pragma once

#include "EEPROM.h"

struct Winding {
  uint16_t turns_per_level = 0;
  uint16_t step = 0;
  uint16_t speed = 30;
  uint16_t total_turns = 0;
  bool dir = 1;
};

void Load(Winding &o, int &p) {
  EEPROM_load(p, o.turns_per_level);
  EEPROM_load(p, o.step);
  EEPROM_load(p, o.speed);
  EEPROM_load(p, o.total_turns);
  EEPROM_load(p, o.dir);
}

void Save(const Winding &o, int &p) {
  EEPROM_save(p, o.turns_per_level);
  EEPROM_save(p, o.step);
  EEPROM_save(p, o.speed);
  EEPROM_save(p, o.total_turns);
  EEPROM_save(p, o.dir);
}

struct Settings {
  bool stopPerLayer = 0;
  uint8_t shaftStep = 1;
  uint8_t layerStep = 1;
  int shaftPos = 0;
  int layerPos = 0;
  uint16_t acceleration = 30;
  int8_t currentTransformer = 0;
};

void Load(Settings &o, int &p) {
  EEPROM_load(p, o.stopPerLayer);
  EEPROM_load(p, o.acceleration);  
  EEPROM_load(p, o.currentTransformer);
}

void Save(const Settings &o, int &p) {
  EEPROM_save(p, o.stopPerLayer);
  EEPROM_save(p, o.acceleration);
  EEPROM_save(p, o.currentTransformer);
}
