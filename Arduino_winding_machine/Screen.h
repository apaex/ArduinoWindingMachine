#pragma once

#include "strings.h"
#include "LiquidCrystalCyr.h"
#include "Winding.h"

class MainScreen {
public:
  LiquidCrystalCyr &lcd;
  const Winding *w = 0;

  MainScreen(LiquidCrystalCyr &lcd_)
    : lcd(lcd_) {}

  void Init(const Winding &w_) {
    w = &w_;
  }

  void Draw() {
    if (!w) return;
    lcd.clear();

    lcd.printfAt_P(0, 0, LINE1_FORMAT, 0, w->total_turns, w->speed);
    lcd.printfAt_P(0, 1, LINE2_FORMAT,  0, (int)ceil((double)w->total_turns / w->turns_per_level), w->step / 1000, w->step % 1000);
  }

  void UpdateTurns(uint16_t v) {
    lcd.printfAt_P(1, 0, LINE4_FORMAT, v);
  }

  void UpdateLayers(uint16_t v) {
    lcd.printfAt_P(1, 1, LINE5_FORMAT, v);
  }

  void UpdateSpeed(uint16_t v) {
    lcd.printfAt_P(9, 0, LINE6_FORMAT, v);
  }

  void PlannerStatus(byte status) {
    if (status >= LENGTH(plannerStatuses))
      return;

    char s[21];
    strcpy_P(s, plannerStatuses[status]);
    PrintLine(3, s);
  }

  void Message(PGM_P st) {
    char s[21];
    strcpy_P(s, st);
    PrintLine((lcd.nRows < 4) ? 1 : 3, s);
  }

  void Message(PGM_P format, byte param) {
    char s[21];
    sprintf_P(s, format, param);
    PrintLine((lcd.nRows < 4) ? 1 : 3, s);
  }

private:
  void PrintLine(byte row, const char *st) {
    if (row >= lcd.nRows)
      return;

    lcd.printAt(0, row, st);

    for (byte i = strlen(st); i < lcd.nCols; ++i)
      lcd.print(' ');
  }
};
