#ifndef GLOBAL_H
#define GLOBAL_H

#include <Adafruit_ST7735.h>

#include "analog_clock.h"
#include "weather.h"

#define TFT_CS 4
#define TFT_RST 16                                            
#define TFT_DC 5

#define BUTTON 15

// In milliseconds
#define M_ONE_SECOND 1000
#define M_FIVE_SECONDS 5000
#define M_TEN_SECONDS 10000
#define M_THIRTY_SECONDS 30000
#define M_ONE_MINUTE 60000

// In seconds
#define S_TEN_SECONDS 10
#define S_ONE_MINUTE 60
#define S_THIRTY_MINUTES 1800
#define S_TWENTYFOUR_HOURS 86400

#define DEBUG_OUTPUT true  // Set to true for debug output, false for no debug output
#define DSERIAL if (DEBUG_OUTPUT) Serial

enum class Screen {
    Clock, Weather  
};

typedef void(*screen_mode_func)();

struct GlobalData {    
    Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
    screen_mode_func current_screen_func = NULL;
    Screen current_screen = Screen::Clock;

    analog_clock::Data clock_data;
    weather::Data weather_data;

    unsigned long current_time = 0;
    float delta_time = 0.0f;

    // First - is currently pressed; second - was previously pressed
    bool button[2] = { false, false };
};

struct Time {
    unsigned long hour;
    unsigned long minute;
    unsigned long second;
};

void change_screen(Screen screen);
bool is_button_pressed(bool* button);
Time get_time_from_unix_time(int unix_time);
unsigned long convert_24hour_to_raw(const Time& time_struct);

extern GlobalData global;

inline constexpr uint16_t swapRB(uint16_t colorBGR) {
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

#endif  // GLOBAL_H
