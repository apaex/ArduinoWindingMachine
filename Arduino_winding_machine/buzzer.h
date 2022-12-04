#include "config.h"
#pragma once

class Buzzer 
{
public:
  Buzzer(byte pin) : _pin(pin) 
  {
    pinMode(BUZZER, OUTPUT);
  };
  
  void Multibeep(byte beeps, unsigned int on, unsigned int off)
  {
    for (byte i = 0; i < beeps; ++i)
    {
      digitalWrite(_pin, HIGH);
      delay(on);
      digitalWrite(_pin, LOW);
      if (i + 1 < beeps)
        delay(off);
    }
  }

private:
  byte _pin;
};
