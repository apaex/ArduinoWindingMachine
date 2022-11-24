#pragma once

#include "strings.h"
#include "LiquidCrystalCyr.h"
#include "Winding.h"

class MainScreen
{
public:
  LiquidCrystalCyr &lcd;
  const Winding &w;
  const Winding &current;

  MainScreen(LiquidCrystalCyr &lcd_, const Winding &w_, const Winding &current_) : lcd(lcd_), w(w_), current(current_) {}

  void Draw()  // Подпрограмма вывода экрана автонамотки
  {  
    lcd.clear();
    lcd.printfAt_P(0,0, LINE1_FORMAT, current.turns, w.turns, current.layers, w.layers);
    lcd.printfAt_P(0,1, LINE2_FORMAT, current.speed, w.step * THREAD_PITCH);  
    UpdateTurns();
    UpdateLayers();
    UpdateSpeed();
  }

  void UpdateTurns()  
  {  
    lcd.printfAt_P(1, 0, LINE4_FORMAT, current.turns+1);
  }

  void UpdateLayers()  
  {  
    lcd.printfAt_P(10, 0, LINE5_FORMAT, current.layers+1);
  }

  void UpdateSpeed()  
  {  
    lcd.printfAt_P(2, 1, LINE6_FORMAT, current.speed);
  }
};