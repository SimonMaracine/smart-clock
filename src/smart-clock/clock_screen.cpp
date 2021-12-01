#include <Adafruit_ST7735.h>

#include "clock_screen.h"
#include "global.h"

extern GlobalData global;

const char* MONTHS[13] = {
    NULL,
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};

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
        global.clock_screen_data.hours = 3;  // TODO placeholder data
        global.clock_screen_data.minutes = 7;
        global.clock_screen_data.seconds = 55;
        global.clock_screen_data.day = 1;
        global.clock_screen_data.month = 12;
        global.clock_screen_data.year = 2021;

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

            // Draw date
            char date[16];
            sprintf(date, "%s %02u %u", MONTHS[global.clock_screen_data.month], global.clock_screen_data.day,
                    global.clock_screen_data.year);

            global.tft.setTextColor(swapRB(ST77XX_WHITE));
            global.tft.setTextSize(2);
            global.tft.setTextWrap(false);

            int16_t x, y;
            uint16_t w, h;
            global.tft.getTextBounds(date, 0, 0, &x, &y, &w, &h);

            global.tft.setCursor(global.tft.width() / 2 - w / 2, 102);
            global.tft.print(date);
        }

        if (current_time - last_time >= ONE_SECOND) {
            last_time = current_time;
            global.clock_screen_data.raw_time++;

            convert_raw_to_24hour(global.clock_screen_data.raw_time, &global.clock_screen_data.hours,
                    &global.clock_screen_data.minutes, &global.clock_screen_data.seconds);

            Serial.print("Hours: ");
            Serial.println(global.clock_screen_data.hours);
            Serial.print("Minutes: ");
            Serial.println(global.clock_screen_data.minutes);
            Serial.print("Seconds: ");
            Serial.println(global.clock_screen_data.seconds);

            // Draw lines background
            global.tft.fillCircle(80, 48, 41, swapRB(0x0005));

            {
                // Draw thin line (second)
                const double x = cos(radians(global.clock_screen_data.seconds * 6) - radians(15 * 6)) * 40;
                const double y = sin(radians(global.clock_screen_data.seconds * 6) - radians(15 * 6)) * 40;
                global.tft.drawLine(80, 48, 80 + x, 48 + y, swapRB(0xBDF7));
            }
            
            {
                // Draw medium line (minute)
                const double x = cos(radians(global.clock_screen_data.minutes * 6) - radians(15 * 6)) * 35;
                const double y = sin(radians(global.clock_screen_data.minutes * 6) - radians(15 * 6)) * 35;
                global.tft.drawLine(80, 48, 80 + x, 48 + y, swapRB(0x7BCF));
            }

            {
                // Draw thick line (hour)
                const double x = cos(radians(global.clock_screen_data.hours * 30) - radians(3 * 30)) * 24;
                const double y = sin(radians(global.clock_screen_data.hours * 30) - radians(3 * 30)) * 24;
                global.tft.drawLine(80, 48, 80 + x, 48 + y, swapRB(0x4208));
            }
        }
        
    }
}
