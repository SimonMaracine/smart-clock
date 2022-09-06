#include <EEPROM.h>
#include <SoftwareSerial.h>

namespace eeprom {
    void read(String& ssid, String& password, String& dst) {
        int index = 0;

        while (char character = EEPROM.read(index)) {
            ssid += character;

            if (character == 255) {
                Serial.println("Error: Read from unwritten memory location");
                ssid = "";
                return;
            }
    
            index++;
        }

        // Reach the password now
        index++;

        while (char character = EEPROM.read(index)) {
            password += character;

            if (character == 255) {
                Serial.println("Error: Read from unwritten memory location");
                password = "";
                return;
            }
    
            index++;
        }

        // And now the DST value
        index++;

         while (char character = EEPROM.read(index)) {
            dst += character;

            if (character == 255) {
                Serial.println("Error: Read from unwritten memory location");
                dst = "";
                return;
            }
    
            index++;
        }
    }

    void write(const String& ssid, const String& password, const String& dst) {
        const String to_write = ssid + '\0' + password + '\0' + dst + '\0';

        for (size_t i = 0; i < to_write.length(); i++) {
            EEPROM.write(i, to_write[i]);
        }
    
        EEPROM.commit();
    }
}
