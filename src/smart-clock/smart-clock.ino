#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#include "screens.h"
#include "clock_screen.h"

#define TFT_CS 4
#define TFT_RST 16                                            
#define TFT_DC 5

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
screen_mode_func current_screen = NULL;

void setup() {
    Serial.begin(9600);

    tft.initR(INITR_BLACKTAB);
    Serial.println("Initialized display");

    current_screen = clock_screen::screen;
}

void loop() {
    current_screen();
    delay(20);
}
