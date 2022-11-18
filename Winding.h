#include <EEPROM.h>

struct Winding
{
  uint16_t turns=0;
  uint16_t step=0;
  uint16_t speed=1;
  uint16_t layers=1;
  uint8_t dir=1;

  void Load(int &p)
  {
    EEPROM.get(p, turns);        p+=2;
    EEPROM.get(p, step);         p+=2;
    EEPROM.get(p, speed);        p+=2;
    EEPROM.get(p, layers);       p+=2;
    EEPROM.get(p, dir);          p+=1;   
  }

  void Save(int &p)
  {
    EEPROM.put(p, turns);        p+=2;
    EEPROM.put(p, step);         p+=2;
    EEPROM.put(p, speed);        p+=2;
    EEPROM.put(p, layers);       p+=2;
    EEPROM.put(p, dir);          p+=1;   
  }
};