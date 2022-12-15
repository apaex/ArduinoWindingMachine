#pragma once
#include "LiquidCrystalCyr.h"

#define CH_UP (char)0
#define CH_DW 1
#define CH_QL 0x3C
#define CH_QR 0x3E
#define CH_CR 0x3E

class MenuItem {
public:
  byte screen;  // Индекс экрана
  byte line;    // Номер строки на экране
  const char *text;

  MenuItem(byte screen_, byte line_, const char *text_)
    : screen(screen_), line(line_), text(text_) {}

  virtual void GetText(char *s) const {
    strcpy_P(s, text);
  }
  virtual void GetValue(char *s) const {
    s[0] = 0;
  }
  virtual void IncValue(int8_t inc, bool cycle = false) {}
};

class BoolMenuItem : public MenuItem {
public:
  bool *value;
  const char **items;

  BoolMenuItem(byte screen_, byte num_, const char *text_, bool *value_, const char **items_)
    : MenuItem(screen_, num_, text_), value(value_), items(items_) {}

  virtual void GetValue(char *s) const {
    strcpy(s, items[*value]);
  }
  virtual void IncValue(int8_t, bool cycle) {
    *value = !*value;
  }
};

template<class T>
class ValueMenuItem : public MenuItem {
public:
  const char *format;  // Формат значения при вводе переменной
  T value = 0;

  ValueMenuItem(byte screen_, byte num_, const char *text_, const char *format_)
    : MenuItem(screen_, num_, text_), format(format_) {}

  virtual void GetValue(char *s) const {
    sprintf_P(s, format, value);
  }
};

template<class T>
class VariableMenuItem : public MenuItem {
public:
  const char *format;  // Формат значения при вводе переменной
  T *value;            // Указатель на адрес текущей переменной изменяемой на экране
  T minVal;            // Ограничение значения переменной снизу
  T maxVal;            // Ограничение значения переменной сверху
  T increment;

  VariableMenuItem(byte screen_, byte num_, const char *text_, const char *format_, T *value_, T min_, T max_, T increment_ = 1)
    : MenuItem(screen_, num_, text_), format(format_), value(value_), minVal(min_), maxVal(max_), increment(increment_) {}

  virtual void GetValue(char *s) const {
    sprintf_P(s, format, *value);
  }
  virtual void IncValue(int8_t inc, bool cycle) {
    T a = *value + inc * increment;
    if (a > maxVal)
      a = cycle ? minVal : maxVal;
    else if (a < minVal)
      a = cycle ? maxVal : minVal;

    *value = a;
  }
};

class FloatMenuItem : public VariableMenuItem<int> {
public:
  FloatMenuItem(byte screen_, byte num_, const char *text_, const char *format_, int *value_, int min_, int max_, int increment_ = 1)
    : VariableMenuItem<int>(screen_, num_, text_, format_, value_, min_, max_, increment_) {}

  virtual void GetValue(char *s) const {
    sprintf_P(s, format, *value / 1000, *value % 1000);
  }
};


class SetMenuItem : public MenuItem {
public:
  const char *format;  // Формат значения при вводе переменной
  uint8_t *value;
  const uint8_t *items;
  uint8_t nItems;

  SetMenuItem(byte screen_, byte num_, const char *text_, const char *format_, uint8_t *value_, const uint8_t items_[], uint8_t nItems_)
    : MenuItem(screen_, num_, text_), format(format_), value(value_), items(items_), nItems(nItems_) {}

  virtual void GetValue(char *s) const {
    sprintf_P(s, format, *value);
  }
  virtual void IncValue(int8_t, bool) {
    if (++index >= nItems)
      index = 0;
    *value = items[index];
  }

private:
  uint8_t index = 0;
};

typedef VariableMenuItem<int8_t> ByteMenuItem;
typedef VariableMenuItem<int16_t> IntMenuItem;
typedef ValueMenuItem<uint16_t> ValMenuItem;

class MainMenu {
public:
  MenuItem **items;
  byte nItems;

  byte index = 0;  // Переменная хранит номер текущей строки меню

  MainMenu(MenuItem **menu, byte count, LiquidCrystalCyr &lcd_)
    : items(menu), nItems(count), lcd(lcd_) {}

  void IncIndex(int8_t inc) {
    index = constrain(index + inc, GetFirstIndex(), GetLastIndex());
  }

  void Draw(bool force = false) {
    byte scr = items[index]->screen;
    byte page = items[index]->line / lcd.nRows;
    byte cur = items[index]->line % lcd.nRows;
    byte first = index - items[index]->line + page * lcd.nRows;

    static byte prev_screen = -1;
    static byte prev_page = -1;

    if (scr != prev_screen || page != prev_page || force) {
      lcd.clear();

      for (int i = 0; i < lcd.nRows; ++i) {
        if (first + i >= nItems)
          break;

        MenuItem *m = items[first + i];

        if (m->screen != scr)
          break;

        DrawItem(m, i);
        UpdateItem(m, i);
      }

      prev_screen = scr;
      prev_page = page;
    }

    for (int i = 0; i < lcd.nRows; ++i)
      lcd.printAt(0, i, (i == cur) ? CH_CR : ' ');

    if (page > 0)  // Выводим стрелки ⯅⯆ на соответствующих строках меню
      lcd.printAt(lcd.nCols - 1, 0, CH_UP);
    if (items[first + lcd.nRows]->screen == scr)
      lcd.printAt(lcd.nCols - 1, lcd.nRows - 1, CH_DW);
  }

  void IncCurrent(int8_t increment, bool cycle = false) {
    byte cur = GetCursor();

    items[index]->IncValue(increment, cycle);
    UpdateItem(items[index], cur);
  }

  void DrawQuotes(bool enable) {
    byte cur = GetCursor();

    byte leftPos = GetValueCol() - 1;
    char s[11];
    items[index]->GetValue(s);
    byte rightPos = leftPos + strlen(s) + 1;

    lcd.printAt(leftPos, cur, enable ? CH_QR : ' ');   // Выводим символ >
    lcd.printAt(rightPos, cur, enable ? CH_QL : ' ');  // Выводим символ <
    lcd.printAt(0, cur, !enable ? CH_CR : ' ');        // Стираем основной курсор
  }

  MenuItem *operator[](byte idx) {
    return items[idx];
  }
  const MenuItem *operator[](byte idx) const {
    return items[idx];
  }

private:
  LiquidCrystalCyr &lcd;

  byte GetTextCol() const {
    return (lcd.nCols > 16) ? 2 : 1;
  }
  byte GetValueCol() const {
    return (lcd.nCols > 16) ? 12 : 10;
  }

  void DrawItem(MenuItem *m, byte row) {
    char s[21];
    m->GetText(s);
    lcd.printAt(GetTextCol(), row, s);
  }

  void UpdateItem(MenuItem *m, byte row) {
    char s[11];
    m->GetValue(s);
    lcd.printAt(GetValueCol(), row, s);
  }

  // для текущего меню получаем индекс первого элемента
  byte GetFirstIndex() const {
    return index - items[index]->line;
  }

  // для текущего меню получаем индекс последнего элемента
  byte GetLastIndex() const {
    byte scr = items[index]->screen;
    byte r = index;
    while ((r + 1 < nItems) && (items[r + 1]->screen == scr))
      ++r;
    return r;
  }

  byte GetCursor() const {
    return items[index]->line % lcd.nRows;
  }
};
