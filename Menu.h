#pragma once
#include "LiquidCrystalCyr.h"


#define CH_UP 0
#define CH_DW 1


struct MenuType
{                       // Структура описывающая меню
    byte Screen;        // Индекс экрана
    byte string_number; // Номер строки на экране
    char type;
    char format[22];        // Формат строки
    char format_Set_var[6]; // Формат значения при вводе переменной
    void *param;            // Указатель на адрес текущей переменной изменяемой на экране
    int var_Min;            // Ограничение значения переменной снизу
    int var_Max;            // Ограничение значения переменной сверху
    byte param_coef;        // Размерный коэффициент значения переменной
    byte increment;
};

class MainMenu
{
private:
    LiquidCrystalCyr *lcd = NULL;

public:
    MenuType *items;
    byte nItems;

    byte nCols;
    byte nRows;

    byte index = 0; // Переменная хранит номер текущей строки меню

    MainMenu(MenuType *menu, byte count) : items(menu), nItems(count) {}

    void InitRender(LiquidCrystalCyr &lcd_, byte nCols_, byte nRows_)
    {
        lcd = &lcd_;
        nCols = nCols_;
        nRows = nRows_;
    }

    // для текущего меню получаем индекс первого элемента
    byte GetFirstIndex()
    {
        return index - items[index].string_number;
    }

    // для текущего меню получаем индекс последнего элемента
    byte GetLastIndex()
    {
        byte scr = items[index].Screen;
        byte r = index;
        while ((r + 1 < nItems) && (items[r + 1].Screen == scr))
            ++r;
        return r;
    }

    void Update() // Подпрограмма: Выводим экран на LCD
    {
        byte scr = items[index].Screen;
        byte page = items[index].string_number / nRows;
        byte cur = items[index].string_number % nRows;
        byte first = index - items[index].string_number + page * nRows;

        static byte prev_screen = -1;
        static byte prev_page = -1;

        if (scr != prev_screen || page != prev_page)
        {
            lcd->clear();

            for (int i = 0; i < nRows; ++i)
            {
                MenuType &m = items[first + i];

                if (m.Screen != scr)
                    break;

                lcd->setCursor(2, i);
                switch (m.type)
                {
                case 'i':
                    lcd->printf(m.format, *(int *)m.param * m.param_coef);
                    break;
                case 'd':
                    lcd->print(m.format);
                    lcd->PrintDirection(12, i, *(bool *)m.param);
                    break;
                case 'b':
                    lcd->print(m.format);
                    lcd->PrintBool(12, i, *(bool *)m.param);
                    break;
                default:
                    lcd->print(m.format);
                }
            }

            prev_screen = scr;
            prev_page = page;
        }

        for (int i = 0; i < nRows; ++i)
            lcd->PrintSymbol(0, i, (i == cur) ? 0x3E : 0x20);

        if (page > 0) // Выводим стрелки ⯅⯆ на соответствующих строках меню
            lcd->PrintSymbol(nCols - 1, 0, CH_UP);
        if (items[first + nRows].Screen == scr)
            lcd->PrintSymbol(nCols - 1, nRows - 1, CH_DW);
    }
    


    void LCD_Print_Var() // Подпрограмма: Выводим новое значение переменной на LCD
    {
        static int Previous_Param;

        if (*(int *)items[index].param == Previous_Param)
            return;

        byte cur = items[index].string_number % nRows;

        lcd->printfAt(10, cur, items[index].format_Set_var, *(int *)items[index].param * items[index].param_coef);
        Previous_Param = *(int *)items[index].param;
    }

    void SetQuote(int First_Cur, int Second_Cur) // Подпрограмма: Выводим выделение изменяемой переменной на LCD
    {
        byte cur = items[index].string_number % nRows;
        lcd->PrintSymbol(First_Cur, cur, 0x3E);  // Выводим символ >
        lcd->PrintSymbol(Second_Cur, cur, 0x3C); // Выводим символ <
        lcd->PrintSymbol(0, cur, 0x20);          // Стираем основной курсор
    }

    void ClearQuote(int First_Cur, int Second_Cur) // Подпрограмма: Стираем выделение изменяемой переменной на LCD
    {
        byte cur = items[index].string_number % nRows;
        lcd->PrintSymbol(First_Cur, cur, 0x20);  // Стираем символ >
        lcd->PrintSymbol(Second_Cur, cur, 0x20); // Стираем символ <
        lcd->PrintSymbol(0, cur, 0x3E);          // Выводим основной курсор
    }

    void PrintDirection(bool b)
    {
        byte cur = items[index].string_number % nRows;
        lcd->PrintDirection(12, cur, b);
    }

    void PrintBool(bool b)
    {
        byte cur = items[index].string_number % nRows;
        lcd->PrintBool(12, cur, b);
    }
};
