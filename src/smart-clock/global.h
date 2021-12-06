#ifndef GLOBAL_H
#define GLOBAL_H

#include <Adafruit_ST7735.h>

#include "analog_clock.h"

#define TFT_CS 4
#define TFT_RST 16                                            
#define TFT_DC 5

// In milliseconds
#define M_ONE_SECOND 1000
#define M_THIRTY_SECONDS 30000

// In seconds
#define S_THIRTY_MINUTES 1800
#define S_ONE_MINUTE 60
#define S_TWENTYFOUR_HOURS 86400
#define S_TEN_SECONDS 10

#define DEBUG_OUTPUT true  // Set to true for debug output, false for no debug output
#define DSERIAL if (DEBUG_OUTPUT) Serial

typedef void(*screen_mode_func)();

struct GlobalData {    
    Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
    screen_mode_func current_screen = NULL;
    analog_clock::Data clock_data;
};

void change_screen(screen_mode_func screen);
uint16_t swapRB(uint16_t colorBGR);

extern GlobalData global;

#endif  // GLOBAL_H
