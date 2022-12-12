#pragma once

#include <HardwareSerial.h>

void TimePrint() {
  int time = millis() / 1000;
  if (time / 60 / 60 < 10) { Serial.print("0"); }
  Serial.print(time / 60 / 60);
  Serial.print(":");
  if (time / 60 % 60 < 10) { Serial.print("0"); }
  Serial.print((time / 60) % 60);
  Serial.print(":");
  if (time % 60 < 10) { Serial.print("0"); }
  Serial.print(time % 60);
  Serial.print(" ");
}

template<class T>
void DebugWrite(T v) {
#ifdef DEBUG
  TimePrint();
  Serial.println(v);
#endif
}

template<class T>
void DebugWrite(const char *st, T v) {
#ifdef DEBUG
  TimePrint();
  Serial.print(st);
  Serial.print(": ");
  Serial.println(v);
#endif
}

void DebugWrite(const char *st, int32_t x, int32_t y) {
#ifdef DEBUG
  TimePrint();
  Serial.print(st);
  Serial.print("(");
  Serial.print(x);
  Serial.print(',');
  Serial.print(y);
  Serial.println(")");
#endif
}

template<class T>
void DebugWrite(const char *st, const T arr[], int n) {
#ifdef DEBUG
  TimePrint();
  Serial.print(st);
  Serial.print(": ");
  
  for (int i = 0; i < n; ++i) {
    Serial.print(arr[i]);
    Serial.print(',');
  }
  Serial.println("");
#endif
}
