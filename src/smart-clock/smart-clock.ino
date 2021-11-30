#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#include "clock_screen.h"
#include "global.h"

GlobalData global;

void setup() {
    Serial.begin(9600);
    
    change_screen(clock_screen::screen);

    global.tft.initR(INITR_BLACKTAB);
    Serial.println("Initialized display");

    global.tft.setRotation(3);  // TODO maybe set this to 1 when putting everything together
    global.tft.fillScreen(ST77XX_BLACK);
}

void loop() {
    global.current_screen();
    delay(5);
}
