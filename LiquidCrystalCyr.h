#pragma once

#include <LiquidCrystal.h>
#include "font.h"

class LiquidCrystalCyr : public LiquidCrystal
{

public:
    LiquidCrystalCyr(uint8_t rs, uint8_t enable,
                     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                     uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) : LiquidCrystal(rs, enable, d0, d1, d2, d3, d4, d5, d6, d7){};
    LiquidCrystalCyr(uint8_t rs, uint8_t rw, uint8_t enable,
                     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                     uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7) : LiquidCrystal(rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7){};
    LiquidCrystalCyr(uint8_t rs, uint8_t rw, uint8_t enable,
                     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) : LiquidCrystal(rs, rw, enable, d0, d1, d2, d3){};
    LiquidCrystalCyr(uint8_t rs, uint8_t enable,
                     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3) : LiquidCrystal(rs, enable, d0, d1, d2, d3){};

    void setCursor(uint8_t col, uint8_t row)
    {
        _col = col;
        _row = row;
        return LiquidCrystal::setCursor(col, row);
    }

    virtual size_t write(uint8_t c)
    {
        if (c >= 192 && c <= 223)
        {
            c = char_map[c - 192];
            if (c < FONT_CHAR_COUNT)
                c = createChar_(c);
        }
        LiquidCrystal::write((byte)c);
        ++_row;
    }

    void printf(const char *format, ...)
    {
        char buf[80];
        va_list args;
        va_start(args, format);
        vsnprintf(buf, 80, format, args);
        va_end(args);
        print(buf);
    }

    void printfAt(uint8_t col, uint8_t row, const char *format, ...)
    {
        char buf[80];
        va_list args;
        va_start(args, format);
        vsnprintf(buf, 80, format, args);
        va_end(args);
        setCursor(col, row);
        print(buf);
    }

private:
    byte _row = 0;
    byte _col = 0;
    byte _query[8] = {8, 7, 6, 5, 4, 3, 2, 1};
    byte _gen[FONT_CHAR_COUNT] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    byte get_char_cell(byte lcd_c)
    {
        byte i = 0;

        if (!lcd_c)
            i = 7;

        while ((i < 7) && (_query[i] != lcd_c))
            ++i;

        byte r = _query[i];

        for (; i > 0; --i)
            _query[i] = _query[i - 1];
        _query[0] = r;

        return r;
    }

    byte createChar_(byte c)
    {
        byte lcd_c = get_char_cell(_gen[c]);

        if (!_gen[c])
        {
            for (byte i = 0; i < FONT_CHAR_COUNT; ++i)
                if (_gen[i] == lcd_c)
                    _gen[i] = 0;
            _gen[c] = lcd_c;

            this->createChar(lcd_c - 1, font[c]);
            _row -= 8;
            LiquidCrystal::setCursor(_row, _col);
        }

        return _gen[c] - 1;
    }

    
public:
    void PrintSymbol(byte col, byte row, byte Symbol_Code) // Подпрограмма: Выводим символ на экран
    {
        setCursor(col, row);
        write(byte(Symbol_Code));
    }

    void PrintDirection(byte col, byte row, bool b)
    {
        char ch = b ? 0x3E : 0x3C;
        PrintSymbol(col + 0, row, ch);
        PrintSymbol(col + 1, row, ch);
        PrintSymbol(col + 2, row, ch);
    }

    void PrintBool(byte col, byte row, bool b)
    {
        setCursor(col, row);
        print(b ? "ON " : "OFF");
    }
};
