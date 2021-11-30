#ifndef GLOBAL_H
#define GLOBAL_H

#include <Adafruit_ST7735.h>

#include "clock_screen.h"

#define TFT_CS 4
#define TFT_RST 16                                            
#define TFT_DC 5

#define ONE_SECOND 1000  // In milliseconds

typedef void(*screen_mode_func)();

struct GlobalData {    
    Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
    screen_mode_func current_screen = NULL;
    clock_screen::Data clock_screen_data;
};

void change_screen(screen_mode_func screen);
uint16_t swapRB(uint16_t colorBGR);

extern GlobalData global;

#endif  // GLOBAL_H
