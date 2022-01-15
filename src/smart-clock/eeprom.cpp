#include <EEPROM.h>
#include <SoftwareSerial.h>

namespace eeprom {
    void read(String& ssid, String& password) {
        int index = 0;
        char character;
    
        while (character = EEPROM.read(index)) {
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
    
        while (character = EEPROM.read(index)) {
            password += character;
    
            if (character == 255) {
                Serial.println("Error: Read from unwritten memory location");
                password = "";
                return;
            }
    
            index++;
        }
    }
    
    void write(const String& ssid, const String& password) {
        String to_write = ssid + '\0' + password + '\0';
    
        for (unsigned int i = 0; i < to_write.length(); i++) {
            EEPROM.write(i, to_write[i]);
        }
    
        EEPROM.commit();
    }
}
