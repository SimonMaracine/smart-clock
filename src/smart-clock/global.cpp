#include <SoftwareSerial.h>

#include "global.h"
#include "analog_clock.h"

void change_screen(screen_mode_func screen) {
    global.current_screen = screen;

    DSERIAL.println("Changed screen");
}

uint16_t swapRB(uint16_t colorBGR) {
    if (colorBGR & (1 << 15)) {
        uint16_t blue_bit = colorBGR & (1 << 4);
        colorBGR |= 1 << 4;
        if (blue_bit) {
            colorBGR |= 1 << 15;
        } else {
            colorBGR &= ~(1 << 15);
        }
    } else {
        uint16_t blue_bit = colorBGR & (1 << 4);
        colorBGR &= ~(1 << 4);
        if (blue_bit) {
            colorBGR |= 1 << 15;
        } else {
            colorBGR &= ~(1 << 15);
        }
    }

    if (colorBGR & (1 << 14)) {
        uint16_t blue_bit = colorBGR & (1 << 3);
        colorBGR |= 1 << 3;
        if (blue_bit) {
            colorBGR |= 1 << 14;
        } else {
            colorBGR &= ~(1 << 14);
        }
    } else {
        uint16_t blue_bit = colorBGR & (1 << 3);
        colorBGR &= ~(1 << 3);
        if (blue_bit) {
            colorBGR |= 1 << 14;
        } else {
            colorBGR &= ~(1 << 14);
        }
    }

    if (colorBGR & (1 << 13)) {
        uint16_t blue_bit = colorBGR & (1 << 2);
        colorBGR |= 1 << 2;
        if (blue_bit) {
            colorBGR |= 1 << 13;
        } else {
            colorBGR &= ~(1 << 13);
        }
    } else {
        uint16_t blue_bit = colorBGR & (1 << 2);
        colorBGR &= ~(1 << 2);
        if (blue_bit) {
            colorBGR |= 1 << 13;
        } else {
            colorBGR &= ~(1 << 13);
        }
    }

    if (colorBGR & (1 << 12)) {
        uint16_t blue_bit = colorBGR & (1 << 1);
        colorBGR |= 1 << 1;
        if (blue_bit) {
            colorBGR |= 1 << 12;
        } else {
            colorBGR &= ~(1 << 12);
        }
    } else {
        uint16_t blue_bit = colorBGR & (1 << 1);
        colorBGR &= ~(1 << 1);
        if (blue_bit) {
            colorBGR |= 1 << 12;
        } else {
            colorBGR &= ~(1 << 12);
        }
    }

    if (colorBGR & (1 << 11)) {
        uint16_t blue_bit = colorBGR & (1 << 0);
        colorBGR |= 1 << 0;
        if (blue_bit) {
            colorBGR |= 1 << 11;
        } else {
            colorBGR &= ~(1 << 11);
        }
    } else {
        uint16_t blue_bit = colorBGR & (1 << 0);
        colorBGR &= ~(1 << 0);
        if (blue_bit) {
            colorBGR |= 1 << 11;
        } else {
            colorBGR &= ~(1 << 11);
        }
    }

    return colorBGR;
}
