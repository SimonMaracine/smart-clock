#include <Adafruit_ST7735.h>

#include "clock_screen.h"
#include "global.h"

extern GlobalData global;

namespace clock_screen {
    static unsigned long convert_24hour_to_raw(unsigned int hours, unsigned int minutes, unsigned int seconds) {
        unsigned long result = 0;

        result += hours * 3600;
        result += minutes * 60;
        result += seconds;

        return result;
    }

    static void convert_raw_to_24hour(unsigned long raw_time, unsigned int* hours, unsigned int* minutes, unsigned int* seconds) {
        *hours = raw_time / 3600;
        raw_time -= *hours * 3600;

        *minutes = raw_time / 60;
        raw_time -= *minutes * 60;

        *seconds = raw_time;
    }

    static void get_time_from_internet() {
        global.clock_screen_data.hours = 12;  // TODO placeholder data
        global.clock_screen_data.minutes = 5;
        global.clock_screen_data.seconds = 0;

        global.clock_screen_data.raw_time = convert_24hour_to_raw(global.clock_screen_data.hours,
                global.clock_screen_data.minutes, global.clock_screen_data.seconds);
    }

    void screen() {
        static unsigned long last_time = 0;
        unsigned long current_time = millis();

        static bool initialized = false;
        if (!initialized) {
            get_time_from_internet();
            initialized = true;
        }

        if (current_time - last_time >= ONE_SECOND) {
            last_time = current_time;
            global.clock_screen_data.raw_time++;

            convert_raw_to_24hour(global.clock_screen_data.raw_time, &global.clock_screen_data.hours,
                    &global.clock_screen_data.minutes, &global.clock_screen_data.seconds);

//            Serial.print("Hours: ");
//            Serial.println(global.clock_screen_data.hours);
//            Serial.print("Minutes: ");
//            Serial.println(global.clock_screen_data.minutes);
//            Serial.print("Seconds: ");
//            Serial.println(global.clock_screen_data.seconds);

            global.tft.fillScreen(ST77XX_BLACK);

            // Draw background
            global.tft.fillCircle(80, 48, 48, swapRB(0x0005));

            // Draw dots
            global.tft.fillRect(80, 1, 2, 2, swapRB(ST77XX_WHITE));  // 12
            global.tft.fillRect(105, 8, 2, 2, swapRB(ST77XX_WHITE));  // 1
            global.tft.fillRect(121, 26, 2, 2, swapRB(ST77XX_WHITE));  // 2
            global.tft.fillRect(126, 47, 2, 2, swapRB(ST77XX_WHITE));  // 3
            global.tft.fillRect(121, 68, 2, 2, swapRB(ST77XX_WHITE));  // 4
            global.tft.fillRect(105, 86, 2, 2, swapRB(ST77XX_WHITE));  // 5
            global.tft.fillRect(80, 93, 2, 2, swapRB(ST77XX_WHITE));  // 6
            global.tft.fillRect(55, 86, 2, 2, swapRB(ST77XX_WHITE));  // 7
            global.tft.fillRect(39, 68, 2, 2, swapRB(ST77XX_WHITE));  // 8
            global.tft.fillRect(34, 47, 2, 2, swapRB(ST77XX_WHITE));  // 9
            global.tft.fillRect(39, 26, 2, 2, swapRB(ST77XX_WHITE));  // 10
            global.tft.fillRect(55, 8, 2, 2, swapRB(ST77XX_WHITE));  // 11

            // Draw thin line
            static const unsigned int OFFSET = 15;  // Seconds
            static const unsigned int SIZE = 40;
            const double x = cos(radians(global.clock_screen_data.seconds * 6) - radians(OFFSET * 6)) * SIZE;
            const double y = sin(radians(global.clock_screen_data.seconds * 6) - radians(OFFSET * 6)) * SIZE;
            global.tft.drawLine(80, 48, 80 + x, 48 + y, swapRB(0xBDF7));

//            Serial.print("seconds: ");
//            Serial.print(global.clock_screen_data.seconds);
//            Serial.print("  cos: ");
//            Serial.print(cos(radians(global.clock_screen_data.seconds * 6) - radians(OFFSET * 6)));
//            Serial.print("  sin: ");
//            Serial.println(sin(radians(global.clock_screen_data.seconds * 6) - radians(OFFSET * 6)));
//            Serial.print("\n");




        }
        
    }
}
