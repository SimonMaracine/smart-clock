#include <Adafruit_ST7735.h>
#include <ArduinoJson.h>

#include "clock_screen.h"
#include "global.h"
#include "http_request.h"

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

    static void parse_datetime(const char* datetime, unsigned int* year, unsigned int* month, unsigned int* day,
            unsigned int* hour, unsigned int* minute, unsigned int* second) {
        char copy[64];
        strcpy(copy, datetime);

        char* token;

        token = strtok(copy, "-T:.");  // 2021-11-23T23:20:30.358093+02:00
        *year = atoi(token);
        token = strtok(NULL, "-T:.");
        *month = atoi(token);
        token = strtok(NULL, "-T:.");
        *day = atoi(token);

        token = strtok(NULL, "-T:.");
        *hour = atoi(token);
        token = strtok(NULL, "-T:.");
        *minute = atoi(token);
        token = strtok(NULL, "-T:.");
        *second = atoi(token);
    }

    static bool get_time_from_internet() {
        String result = http_request::get("http://worldtimeapi.org/api/ip");

        if (result == "") {
            Serial.println("Couldn't get time from internet");
            return false;
        }

        StaticJsonDocument<256> json;
        deserializeJson(json, result.c_str());

        const char* datetime = json["datetime"];

        unsigned int year, month, day, hour, minute, second;
        parse_datetime(datetime, &year, &month, &day, &hour, &minute, &second);

        global.clock_screen_data.hour = hour;
        global.clock_screen_data.minute = minute;
        global.clock_screen_data.second = second + 1;  // Add an offset accounting for this delay
        global.clock_screen_data.day = day;
        global.clock_screen_data.month = month;
        global.clock_screen_data.year = year;

        convert_24hour_to_raw(global.clock_screen_data, &global.clock_screen_data.raw_time);

        Serial.printf("Got time: %u-%u-%u %u:%u:%u\n", year, month, day, hour, minute, second);
        return true;
    }

    static void initialize() {
        if (!get_time_from_internet()) {
            global.clock_screen_data.reinitialize_after_one_minute = true;
            Serial.println("Retrying to initialize clock after one minute...");
            return;  // Don't do anything else
        }

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

        Serial.println("Initialized clock");
    }

    void screen() {
        static unsigned long last_time = 0;
        unsigned long current_time = millis();

        static bool initialized = false;

        if (global.clock_screen_data.reinitialize_after_one_minute) {
            if (global.clock_screen_data.raw_time - global.clock_screen_data.last_initialized >= ONE_MINUTE) {
                initialized = false;
                global.clock_screen_data.reinitialize_after_one_minute = false;
            }
        }

        if (!initialized) {
            initialize();
            initialized = true;
            global.clock_screen_data.last_initialized = global.clock_screen_data.raw_time;
        }

        if (current_time - last_time >= ONE_SECOND) {
            last_time = current_time;
            global.clock_screen_data.raw_time++;

            if (global.clock_screen_data.raw_time == TWENTYFOUR_HOURS) {  // Reset when it reaches 24 hours
                global.clock_screen_data.raw_time = 0;
                initialized = false;

                global.tft.setTextColor(swapRB(ST77XX_WHITE));
                global.tft.setTextSize(1);
                global.tft.setTextWrap(false);

                int16_t x, y;
                uint16_t w, h;
                global.tft.getTextBounds("Yesterday", 0, 0, &x, &y, &w, &h);

                global.tft.setCursor(global.tft.width() / 2 - w / 2, 119);
                global.tft.print("Yesterday");
            }

            if (global.clock_screen_data.raw_time % TWENTY_MINUTES == 0) {  // Try to update once every twenty minutes
                get_time_from_internet();  // Ignore errors
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
