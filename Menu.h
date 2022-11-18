#pragma once

#include "LiquidCrystal.h"

class MenuItemBase
{
    LiquidCrystal& _lcd;
public:
    virtual Draw()
    {

    }
};

class TextMenuItem : public MenuItemBase
{

};

class IntValueMenuItem : public MenuItemBase
{

};

class SetValueMenuItem : public MenuItemBase
{

};


class MainMenu
{
    LiquidCrystal& _lcd;

    MenuItemBase *items[];
public:
    MainMenu(LiquidCrystal &lcd) : _lcd(lcd)
    {

    }

    void AddItem(const char * text)
    {
        
    }
    void AddIntValueItem(const char * text)
    {

    }
    void AddTextValueItem(const char * text)
    {

    }

};
