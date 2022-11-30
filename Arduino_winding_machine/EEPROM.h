#pragma once 

#include <EEPROM.h>

template< typename T > void EEPROM_load( int& idx, T &t ){
    EEPROM.get(idx, t);
    idx += sizeof(T);
}

template< typename T > void EEPROM_save( int& idx, const T &t ){
    T v = 0;
    EEPROM.get(idx, v);
    if (t != v)
      EEPROM.put(idx, t);
    idx += sizeof(T);
}
