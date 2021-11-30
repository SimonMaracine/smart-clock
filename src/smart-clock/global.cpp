#include <SoftwareSerial.h>

#include "global.h"

void change_screen(screen_mode_func screen) {
    global.current_screen = screen;
    Serial.println("Changed screen");
}

uint16_t swapRB(uint16_t colorBGR) {  // TODO this is wrong!!!
    uint16_t result = 0;
    uint16_t multiplier = 1;

    for (uint16_t i = 0; i < 16; i++) {
        uint16_t mask = 1 << (16 - i - 1);

        if (colorBGR & mask) {
            result += multiplier;    
        }

        multiplier *= 2;
    }

    return result;
}
