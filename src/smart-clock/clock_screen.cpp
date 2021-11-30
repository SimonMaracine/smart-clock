#include <Adafruit_ST7735.h>

#include "clock_screen.h"
#include "screens.h"

extern Adafruit_ST7735 tft;

namespace clock_screen {
    void screen() {
        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(tft.width() / 2, tft.height() / 2);
        tft.setTextColor(ST77XX_WHITE);
        tft.print("Clock Screen");
    }
}
