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

    lcd.printfAt_P(0, 0, LINE1_FORMAT, 0, w->turns, 0, w->layers);
    lcd.printfAt_P(0, 1, LINE2_FORMAT, w->speed, w->step);
  }

  void UpdateTurns(uint16_t v) {
    lcd.printfAt_P(1, 0, LINE4_FORMAT, v);
  }

  void UpdateLayers(uint16_t v) {
    lcd.printfAt_P(10, 0, LINE5_FORMAT, v);
  }

  void UpdateSpeed(uint16_t v) {
    lcd.printfAt_P(2, 1, LINE6_FORMAT, v);
  }

  void PlannerStatus(byte status) {
    if (status >= LENGTH(plannerStatuses))
      return;

    PrintLine_P(3, plannerStatuses[status]);
  }

  void Message(PGM_P st) {
    PrintLine_P((lcd.nRows < 4) ? 1 : 3, st);
  }

  void Message(PGM_P format, byte param) {
    char s[21];
    sprintf_P(s, format, param);
    PrintLine((lcd.nRows < 4) ? 1 : 3, s);
  }

private:
  void PrintLine(byte row, PGM_P st) {
    if (row >= lcd.nRows)
      return;

    lcd.printAt(0, row, st);

    for (byte i = strlen(st); i < lcd.nCols; ++i)
      lcd.print(' ');
  }

  void PrintLine_P(byte row, PGM_P st) {
    if (row >= lcd.nRows)
      return;

    lcd.printAt_P(0, row, st);

    for (byte i = strlen_P(st); i < lcd.nCols; ++i)
      lcd.print(' ');
  }
};
