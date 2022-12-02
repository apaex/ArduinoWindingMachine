#pragma once

#include <HardwareSerial.h>

template <class T>
void DebugWrite(T v)
{
#ifdef DEBUG
  Serial.println(v);
#endif
}

template <class T>
void DebugWrite(const char *s, T v)
{
#ifdef DEBUG
  Serial.print(s);
  Serial.print(": ");
  Serial.println(v);
#endif
}

void DebugWrite(const char *st, int32_t x, int32_t y)
{
#ifdef DEBUG
  Serial.print(st);
  Serial.print("(");
  Serial.print(x);
  Serial.print(',');
  Serial.print(y);
  Serial.println(")");
#endif
}
