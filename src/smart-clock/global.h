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
#define M_THIRTY_SECONDS 30000
#define M_THREE_SECONDS 3000

// In seconds
#define S_THIRTY_MINUTES 1800
#define S_ONE_MINUTE 60
#define S_TWENTYFOUR_HOURS 86400
#define S_TEN_SECONDS 10

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

void change_screen(Screen screen);
uint16_t swapRB(uint16_t colorBGR);
bool is_button_pressed(bool* button);

extern GlobalData global;

#endif  // GLOBAL_H
