#ifndef EEPROM_H
#define EEPROM_H

namespace eeprom {
    void read(String& ssid, String& password, String& dst);
    void write(const String& ssid, const String& password, const String& dst);
}

#endif  // EEPROM_H
