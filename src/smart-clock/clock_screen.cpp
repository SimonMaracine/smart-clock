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
    static void convert_24hour_to_raw(const Data& clock_screen_data, unsigned long* raw_time) {
        unsigned long result = 0;

        result += clock_screen_data.hour * 3600;
        result += clock_screen_data.minute * 60;
        result += clock_screen_data.second;

        *raw_time = result;
    }

    static void convert_raw_to_24hour(unsigned long raw_time, Data* clock_screen_data) {
        clock_screen_data->hour = raw_time / 3600;
        raw_time -= clock_screen_data->hour * 3600;

        clock_screen_data->minute = raw_time / 60;
        raw_time -= clock_screen_data->minute * 60;

        clock_screen_data->second = raw_time;
    }

    static void get_time_from_internet() {
        global.clock_screen_data.hour = 23;  // TODO placeholder data
        global.clock_screen_data.minute = 59;
        global.clock_screen_data.second = 53;
        global.clock_screen_data.day = 1;
        global.clock_screen_data.month = 12;
        global.clock_screen_data.year = 2021;

        convert_24hour_to_raw(global.clock_screen_data, &global.clock_screen_data.raw_time);
    }

    void screen() {
        static unsigned long last_time = 0;
        unsigned long current_time = millis();

        static bool initialized = false;
        if (!initialized) {
            get_time_from_internet();
            initialized = true;

            // Clear screen
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

            if (global.clock_screen_data.raw_time == 86400) {  // When it reaches 24 hours
                global.clock_screen_data.raw_time = 0;
//                initialized = false;  // TODO uncomment this

                global.tft.setTextColor(swapRB(ST77XX_WHITE));
                global.tft.setTextSize(1);
                global.tft.setTextWrap(false);

                int16_t x, y;
                uint16_t w, h;
                global.tft.getTextBounds("Yesterday", 0, 0, &x, &y, &w, &h);

                global.tft.setCursor(global.tft.width() / 2 - w / 2, 119);
                global.tft.print("Yesterday");
            }

            convert_raw_to_24hour(global.clock_screen_data.raw_time, &global.clock_screen_data);

            Serial.printf("Hour: %u\n", global.clock_screen_data.hour);
            Serial.printf("Minute: %u\n", global.clock_screen_data.minute);
            Serial.printf("Second: %u\n", global.clock_screen_data.second);

            // Draw lines background
            global.tft.fillCircle(80, 48, 41, swapRB(0x0005));
            
            {
                // Draw thin line (second)
                const double x = cos(radians(global.clock_screen_data.second * 6) - radians(15 * 6)) * 40;
                const double y = sin(radians(global.clock_screen_data.second * 6) - radians(15 * 6)) * 40;
                global.tft.drawLine(80, 48, 80 + x, 48 + y, swapRB(0xBDF7));
            }
            {
                // Draw medium line (minute)
                const double x = cos(radians(global.clock_screen_data.minute * 6) - radians(15 * 6)) * 35;
                const double y = sin(radians(global.clock_screen_data.minute * 6) - radians(15 * 6)) * 35;
                global.tft.drawLine(80, 48, 80 + x, 48 + y, swapRB(0x7BCF));
            }
            {
                // Draw thick line (hour)
                const double x = cos(radians(global.clock_screen_data.hour * 30) - radians(3 * 30)) * 24;
                const double y = sin(radians(global.clock_screen_data.hour * 30) - radians(3 * 30)) * 24;
                global.tft.drawLine(80, 48, 80 + x, 48 + y, swapRB(0x4208));
            }
        }
        
    }
}
