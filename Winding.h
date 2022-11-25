#pragma once 

#include <EEPROM.h>

template< typename T > void EEPROM_save( int idx, const T &t ){
    T v = 0;
    EEPROM.get(idx, v);
    if (t != v)
      EEPROM.put(idx, t);
}


struct Winding
{
  uint16_t turns=0;
  uint8_t step=0;
  uint8_t speed=30;    
  uint8_t layers=0;
  bool dir=1;
};

void Load(Winding& o, int &p)
{
  EEPROM.get(p, o.turns);        p+=2;
  EEPROM.get(p, o.step);         p+=1;
  EEPROM.get(p, o.speed);        p+=1;  
  EEPROM.get(p, o.layers);       p+=1;
  EEPROM.get(p, o.dir);          p+=1;   
}

void Save(const Winding& o, int &p)
{
  EEPROM_save(p, o.turns);       p+=2;
  EEPROM_save(p, o.step);        p+=1;
  EEPROM_save(p, o.speed);       p+=1;
  EEPROM_save(p, o.layers);      p+=1;
  EEPROM_save(p, o.dir);         p+=1;   
}

struct Settings 
{
  bool stopPerLayer = 0;
  uint8_t shaftStep = 1;
  uint8_t layerStep = 1;
  int shaftPos = 0;
  int layerPos = 0;
};

void Load(Settings& o, int &p)
{
  EEPROM.get(p, o.stopPerLayer);        p+=1;
}

void Save(const Settings& o, int &p)
{
  EEPROM_save(p, o.stopPerLayer);       p+=1; 
}


