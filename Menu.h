#pragma once
#include "LiquidCrystalCyr.h"

#define CH_UP 0
#define CH_DW 1
#define CH_QL 0x3C
#define CH_QR 0x3E
#define CH_CR 0x3E

class MenuItem
{                       
  public:
    byte screen;        // Индекс экрана
    byte line;          // Номер строки на экране
    const char* text;   

    MenuItem(byte screen_, byte line_, const char* text_) : screen(screen_), line(line_), text(text_) {}

    virtual void Draw(LiquidCrystalCyr &lcd, byte row)
    {
      lcd.setCursor(2, row);
      lcd.print(text);
      Update(lcd, row);
    }

    virtual void Update(LiquidCrystalCyr &lcd, byte row) {}
    virtual void IncValue(int8_t inc) {}
};

class BoolMenuItem : public MenuItem
{  
  public:
    bool* value;   
    const char ** items;

    BoolMenuItem(byte screen_, byte num_, const char* text_, bool * value_, const char ** items_) : MenuItem(screen_, num_, text_), value(value_), items(items_) {}

    virtual void Update(LiquidCrystalCyr &lcd, byte row) 
    {
      lcd.setCursor(12, row);
      lcd.print(items[*value]);     
    }

    virtual void IncValue(int8_t )
    {
      *value = !*value;
    }
};

template <class T>
class ValueMenuItem : public MenuItem
{    
  public:                   
    const char* format;  // Формат значения при вводе переменной
    T *value;            // Указатель на адрес текущей переменной изменяемой на экране
    T minVal;            // Ограничение значения переменной снизу
    T maxVal;            // Ограничение значения переменной сверху
    T scale;             // Размерный коэффициент значения переменной
    T increment;

    ValueMenuItem(byte screen_, byte num_, const char* text_, const char* format_, T* value_, T min_, T max_, T scale_ = 1, T increment_ = 1) : MenuItem(screen_, num_, text_), format(format_), value(value_), minVal(min_), maxVal(max_), scale(scale_), increment(increment_) {}

    virtual void Update(LiquidCrystalCyr &lcd, byte row)
    {
      lcd.printfAt(10, row, format, int(*value) * scale);
    }

    virtual void IncValue(int8_t inc)
    {       
      *value = constrain(*value + inc * increment, minVal, maxVal);    
    }
};

class SetMenuItem : public MenuItem
{  
  public:
    const char* format; // Формат значения при вводе переменной
    uint8_t* value;   
    const uint8_t ** items;
    uint8_t nItems;

    SetMenuItem(byte screen_, byte num_, const char* text_, const char* format_, uint8_t * value_, const uint8_t ** items_, uint8_t nItems_) : MenuItem(screen_, num_, text_), format(format_), value(value_), items(items_), nItems(nItems_) {}

    virtual void Update(LiquidCrystalCyr &lcd, byte row) 
    {
      lcd.printfAt(10, row, format, *value);
    }

    virtual void IncValue(int8_t )
    {
      if (++index >= nItems)
        index = 0;
      *value = items[index];
    }

  private:
    uint8_t index = 0;
};

typedef ValueMenuItem<uint8_t> ByteMenuItem;
typedef ValueMenuItem<int16_t> IntMenuItem;
typedef ValueMenuItem<uint16_t> UIntMenuItem;

class MainMenu
{
private:
    LiquidCrystalCyr &lcd;

public:
    MenuItem **items;
    byte nItems;

    byte index = 0; // Переменная хранит номер текущей строки меню

    MainMenu(MenuItem **menu, byte count, LiquidCrystalCyr &lcd_) : items(menu), nItems(count), lcd(lcd_) {}

    // для текущего меню получаем индекс первого элемента
    byte GetFirstIndex() const
    {
        return index - items[index]->line;
    }

    // для текущего меню получаем индекс последнего элемента
    byte GetLastIndex() const
    {
        byte scr = items[index]->screen;
        byte r = index;
        while ((r + 1 < nItems) && (items[r + 1]->screen == scr))
          ++r;
        return r;
    }

    void Draw() // Подпрограмма: Выводим экран на LCD
    {
        byte scr = items[index]->screen;
        byte page = items[index]->line / lcd.nRows;
        byte cur = items[index]->line % lcd.nRows;
        byte first = index - items[index]->line + page * lcd.nRows;

        static byte prev_screen = -1;
        static byte prev_page = -1;

        if (scr != prev_screen || page != prev_page)
        {
            lcd.clear();

            for (int i = 0; i < lcd.nRows; ++i)
            {
                if (first + i >= nItems)
                    break;
                    
                MenuItem *m = items[first + i];

                if (m->screen != scr)
                    break;

                m->Draw(lcd, i);
            }

            prev_screen = scr;
            prev_page = page;
        }

        for (int i = 0; i < lcd.nRows; ++i)
            lcd.PrintSymbol(0, i, (i == cur) ? CH_CR : ' ');

        if (page > 0) // Выводим стрелки ⯅⯆ на соответствующих строках меню
            lcd.PrintSymbol(lcd.nCols - 1, 0, CH_UP);
        if (items[first + lcd.nRows]->screen == scr)
            lcd.PrintSymbol(lcd.nCols - 1, lcd.nRows - 1, CH_DW);
    }
    
    byte GetCursor() const
    {
      return items[index]->line % lcd.nRows;
    }

    void IncCurrent(int8_t increment) 
    {
        byte cur = GetCursor();  

        items[index]->IncValue(increment);     
        items[index]->Update(lcd, cur);
    }

    void DrawQuotes(bool enable, byte leftPos, byte rightPos)
    {
        byte cur = GetCursor();
        
        lcd.PrintSymbol(leftPos, cur, enable ? CH_QR : ' ');  // Выводим символ >
        lcd.PrintSymbol(rightPos, cur, enable ? CH_QL : ' '); // Выводим символ <
        lcd.PrintSymbol(0, cur, !enable ? CH_CR : ' ');       // Стираем основной курсор
    }

    void SetQuote(byte leftPos, byte rightPos) { DrawQuotes(1, leftPos, rightPos); }
    void ClearQuote(byte leftPos, byte rightPos) { DrawQuotes(0, leftPos, rightPos); }

    MenuItem* operator[](byte idx)       { return items[idx]; }
    const MenuItem* operator[](byte idx) const { return items[idx]; }

};
