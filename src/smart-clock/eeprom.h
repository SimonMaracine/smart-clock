#ifndef EEPROM_H
#define EEPROM_H

namespace eeprom {
    void read(String& ssid, String& password);
    void write(const String& ssid, const String& password);    
}

#endif  // EEPROM_H
