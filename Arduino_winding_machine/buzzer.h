#include "config.h"
#pragma once

class Buzzer 
{
public:
  Buzzer(byte pin) : _pin(pin) 
  {
    pinMode(BUZZER, OUTPUT);
  };
  
  void Multibeep(int beeps, int on, int off)
  {
    for (int i = 0; i < beeps; ++i)
    {
      digitalWrite(_pin, HIGH);
      delay(on);
      digitalWrite(_pin, LOW);
      delay(off);
    }
  }

private:
  byte _pin;
};
