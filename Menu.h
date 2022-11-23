#pragma once
#include "LiquidCrystalCyr.h"


#define CH_UP 0
#define CH_DW 1

class MenuItem
{                       
  public:
    byte Screen;        // Индекс экрана
    byte string_number; // Номер строки на экране
    const char* format;        // Формат строки

    MenuItem(byte screen_, byte num_, const char* text_) : Screen(screen_), string_number(num_), format(text_) {}

    virtual void Draw(LiquidCrystalCyr &lcd, byte row)
    {
      lcd.setCursor(2, row);
      lcd.print(format);
      Update(lcd, row);
    }

    virtual void Update(LiquidCrystalCyr &lcd, byte row) {}
    virtual void IncValue(int8_t inc) {}
};

class BoolMenuItem : public MenuItem
{  
  public:
    bool* value;   
    const char ** setItem;

    BoolMenuItem(byte screen_, byte num_, const char* text_, bool * value_, const char ** setItem_) : MenuItem(screen_, num_, text_), value(value_), setItem(setItem_) {}

    virtual void Update(LiquidCrystalCyr &lcd, byte row) 
    {
      lcd.setCursor(12, row);
      lcd.print(setItem[*value]);     
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
    const char* format_Set_var; // Формат значения при вводе переменной
    T *param;            // Указатель на адрес текущей переменной изменяемой на экране
    T var_Min;            // Ограничение значения переменной снизу
    T var_Max;            // Ограничение значения переменной сверху
    T param_coef;        // Размерный коэффициент значения переменной
    T increment;

    ValueMenuItem(byte screen_, byte num_, const char* text_, const char* format_, T* value_, T min_, T max_, T scale_ = 1, T increment_ = 1) : MenuItem(screen_, num_, text_), format_Set_var(format_), param(value_), var_Min(min_), var_Max(max_), param_coef(scale_), increment(increment_) {}

    virtual void Update(LiquidCrystalCyr &lcd, byte row)
    {
      lcd.printfAt(10, row, format_Set_var, int(*param) * param_coef);
    }

    virtual void IncValue(int8_t inc)
    {       
      *param = constrain(*param + inc * increment, var_Min, var_Max);    
    }
};

typedef ValueMenuItem<uint8_t> ByteMenuItem;
typedef ValueMenuItem<int16_t> IntMenuItem;
typedef ValueMenuItem<uint16_t> UIntMenuItem;

class MainMenu
{
private:
    LiquidCrystalCyr *lcd = NULL;

public:
    MenuItem **items;
    byte nItems;

    byte nCols;
    byte nRows;

    byte index = 0; // Переменная хранит номер текущей строки меню

    MainMenu(MenuItem **menu, byte count) : items(menu), nItems(count) {}

    void InitRender(LiquidCrystalCyr &lcd_, byte nCols_, byte nRows_)
    {
        lcd = &lcd_;
        nCols = nCols_;
        nRows = nRows_;
    }

    // для текущего меню получаем индекс первого элемента
    byte GetFirstIndex()
    {
        return index - items[index]->string_number;
    }

    // для текущего меню получаем индекс последнего элемента
    byte GetLastIndex()
    {
        byte scr = items[index]->Screen;
        byte r = index;
        while ((r + 1 < nItems) && (items[r + 1]->Screen == scr))
            ++r;
        return r;
    }

    void Update() // Подпрограмма: Выводим экран на LCD
    {
        byte scr = items[index]->Screen;
        byte page = items[index]->string_number / nRows;
        byte cur = items[index]->string_number % nRows;
        byte first = index - items[index]->string_number + page * nRows;

        static byte prev_screen = -1;
        static byte prev_page = -1;

        if (scr != prev_screen || page != prev_page)
        {
            lcd->clear();

            for (int i = 0; i < nRows; ++i)
            {
                if (first + i >= nItems)
                    break;
                    
                MenuItem *m = items[first + i];

                if (m->Screen != scr)
                    break;

                m->Draw(*lcd, i);
            }

            prev_screen = scr;
            prev_page = page;
        }

        for (int i = 0; i < nRows; ++i)
            lcd->PrintSymbol(0, i, (i == cur) ? 0x3E : 0x20);

        if (page > 0) // Выводим стрелки ⯅⯆ на соответствующих строках меню
            lcd->PrintSymbol(nCols - 1, 0, CH_UP);
        if (items[first + nRows]->Screen == scr)
            lcd->PrintSymbol(nCols - 1, nRows - 1, CH_DW);
    }
    


    void IncCurrent(int8_t increment) 
    {
        byte cur = items[index]->string_number % nRows;   

        items[index]->IncValue(increment);     
        items[index]->Update(*lcd, cur);
    }

    void SetQuote(int First_Cur, int Second_Cur) // Подпрограмма: Выводим выделение изменяемой переменной на LCD
    {
        byte cur = items[index]->string_number % nRows;
        lcd->PrintSymbol(First_Cur, cur, 0x3E);  // Выводим символ >
        lcd->PrintSymbol(Second_Cur, cur, 0x3C); // Выводим символ <
        lcd->PrintSymbol(0, cur, 0x20);          // Стираем основной курсор
    }

    void ClearQuote(int First_Cur, int Second_Cur) // Подпрограмма: Стираем выделение изменяемой переменной на LCD
    {
        byte cur = items[index]->string_number % nRows;
        lcd->PrintSymbol(First_Cur, cur, 0x20);  // Стираем символ >
        lcd->PrintSymbol(Second_Cur, cur, 0x20); // Стираем символ <
        lcd->PrintSymbol(0, cur, 0x3E);          // Выводим основной курсор
    }

};
