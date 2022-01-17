#include <ArduinoJson.h>

#include "analog_clock.h"
#include "global.h"
#include "http_request.h"

static const char* MONTHS[13] = {
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

static const unsigned int CLOCK_X_POS = 80;
static const unsigned int CLOCK_Y_POS = 48;
static const unsigned int CLOCK_RADIUS = 48;
static const unsigned int THIN_LINE = 40;
static const unsigned int MEDIUM_LINE = 35;
static const unsigned int THICK_LINE = 26;

namespace analog_clock {
    static void convert_24hour_to_raw(const Data& clock_data, unsigned long* raw_time) {
        unsigned long result = 0;

        result += clock_data.hour * 3600;
        result += clock_data.minute * 60;
        result += clock_data.second;

        *raw_time = result;
    }

    static void convert_raw_to_24hour(unsigned long raw_time, Data* clock_data) {
        clock_data->hour = raw_time / 3600;
        raw_time -= clock_data->hour * 3600;

        clock_data->minute = raw_time / 60;
        raw_time -= clock_data->minute * 60;

        clock_data->second = raw_time;
    }

    static void parse_datetime(const char* datetime, unsigned long* year, unsigned long* month, unsigned long* day,
            unsigned long* hour, unsigned long* minute, unsigned long* second) {
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
            DSERIAL.println("Couldn't get time from internet");
            return false;
        }

        StaticJsonDocument<512> json;
        deserializeJson(json, result.c_str());

        const char* datetime = json["datetime"];

        unsigned long year, month, day, hour, minute, second;
        parse_datetime(datetime, &year, &month, &day, &hour, &minute, &second);

        global.clock_data.hour = hour;
        global.clock_data.minute = minute;
        global.clock_data.second = second + 3;  // Add an offset accounting for this delay
        global.clock_data.day = day;
        global.clock_data.month = month;
        global.clock_data.year = year;

        const char* utc_offset = json["utc_offset"];

        char offset[8];
        memcpy(offset, utc_offset, sizeof(char) * 3);
        offset[3] = 0;
        global.clock_data.utc_offset = atoi(offset);

        convert_24hour_to_raw(global.clock_data, &global.clock_data.raw_time);

        DSERIAL.printf("Got time: %lu-%02lu-%02lu %02lu:%02lu:%02lu\n", year, month, day, hour, minute, second);
        return true;
    }

    static void draw_date() {
        global.tft.fillRect(0, 101, global.tft.width(), global.tft.height() - 101, ST77XX_BLACK);

        char date[16];
        sprintf(date, "%s %02lu %lu", MONTHS[global.clock_data.month], global.clock_data.day,
                global.clock_data.year);

        global.tft.setTextColor(ST77XX_WHITE);
        global.tft.setTextSize(2);
        global.tft.setTextWrap(false);

        int16_t x, y;
        uint16_t w, h;
        global.tft.getTextBounds(date, 0, 0, &x, &y, &w, &h);

        global.tft.setCursor(global.tft.width() / 2 - w / 2, 102);
        global.tft.print(date);
    }

    static void update_clock() {
        if (!get_time_from_internet()) {
            DSERIAL.println("Retrying to update clock after one minute...");

            if (global.clock_data.reupdate_tries < 5) {
                global.clock_data.reupdate_after_one_minute = true;    
            } else {
                global.clock_data.reupdate_tries = 0;
            }

            global.clock_data.last_updated = global.clock_data.raw_time;
            return;  // Don't do anything else
        }

        draw_date();

        DSERIAL.println("Updated clock");
        global.clock_data.reupdate_after_one_minute = false;  // Don't update after one minute, if it succeeded
        global.clock_data.reupdate_tries = 0;

        global.clock_data.last_updated = global.clock_data.raw_time;
    }

    void start_draw() {
        // Clear screen
        global.tft.fillScreen(ST77XX_BLACK);

        // Draw background
        global.tft.fillCircle(CLOCK_X_POS, CLOCK_Y_POS, CLOCK_RADIUS, swapRB(0x0005));

        // Draw dots
        global.tft.fillRect(80, 1, 2, 2, ST77XX_WHITE);  // 12
        global.tft.fillRect(105, 8, 2, 2, ST77XX_WHITE);  // 1
        global.tft.fillRect(121, 26, 2, 2, ST77XX_WHITE);  // 2
        global.tft.fillRect(126, 47, 2, 2, ST77XX_WHITE);  // 3
        global.tft.fillRect(121, 68, 2, 2, ST77XX_WHITE);  // 4
        global.tft.fillRect(105, 86, 2, 2, ST77XX_WHITE);  // 5
        global.tft.fillRect(80, 93, 2, 2, ST77XX_WHITE);  // 6
        global.tft.fillRect(55, 86, 2, 2, ST77XX_WHITE);  // 7
        global.tft.fillRect(39, 68, 2, 2, ST77XX_WHITE);  // 8
        global.tft.fillRect(34, 47, 2, 2, ST77XX_WHITE);  // 9
        global.tft.fillRect(39, 26, 2, 2, ST77XX_WHITE);  // 10
        global.tft.fillRect(55, 8, 2, 2, ST77XX_WHITE);  // 11

        // Draw date
        draw_date();
    }

    void update() {
        static unsigned long last_time = 0;

        static bool initialized = false;
        if (!initialized) {
            update_clock();
            initialized = true;
        }

        if (global.current_time - last_time >= M_ONE_SECOND) {
            last_time = global.current_time;
            global.clock_data.raw_time++;

            if (global.clock_data.reupdate_after_one_minute) {
                if (global.clock_data.raw_time - global.clock_data.last_updated >= S_ONE_MINUTE) {
                    global.clock_data.reupdate_after_one_minute = false;
                    global.clock_data.reupdate_tries++;
                    update_clock();
                }
            }

            // Try to update once every thirty minutes
            if (global.clock_data.raw_time - global.clock_data.last_updated >= S_THIRTY_MINUTES) {
                update_clock();    
            }

            // Update when it reaches 24 hours
            if (global.clock_data.raw_time == (S_TWENTYFOUR_HOURS + S_TEN_SECONDS)) {
                global.clock_data.raw_time = S_TEN_SECONDS;
                update_clock();
            }

            convert_raw_to_24hour(global.clock_data.raw_time, &global.clock_data);

//            DSERIAL.printf("%02lu:%02lu.%02lu\n", global.clock_data.hour, global.clock_data.minute,
//                    global.clock_data.second);
        }
    }

    void draw() {
        static unsigned long last_time = 0;

        if (global.current_time - last_time >= M_ONE_SECOND) {
            last_time = global.current_time;

            // Draw lines background
            global.tft.fillCircle(CLOCK_X_POS, CLOCK_Y_POS, THIN_LINE + 1, swapRB(0x0005));

            {
                // Draw thin line (second)
                const double x = cos(radians(global.clock_data.second * 6) - radians(15 * 6)) * THIN_LINE;
                const double y = sin(radians(global.clock_data.second * 6) - radians(15 * 6)) * THIN_LINE;
                global.tft.drawLine(CLOCK_X_POS, CLOCK_Y_POS, lround(CLOCK_X_POS + x), lround(CLOCK_Y_POS + y), swapRB(0xEF5F));
            }
            {
                // Draw medium line (minute)
                const long offset = map(global.clock_data.second, 0, 60, 0, 6);
                const double x = cos(radians(global.clock_data.minute * 6) - radians(15 * 6) + radians(offset)) * MEDIUM_LINE;
                const double y = sin(radians(global.clock_data.minute * 6) - radians(15 * 6) + radians(offset)) * MEDIUM_LINE;
                global.tft.drawLine(CLOCK_X_POS, CLOCK_Y_POS, lround(CLOCK_X_POS + x), lround(CLOCK_Y_POS + y), swapRB(0x8C7F));
            }
            {
                // Draw thick line (hour)
                const long offset = map(global.clock_data.minute, 0, 60, 0, 30);
                const double x = cos(radians(global.clock_data.hour * 30) - radians(3 * 30) + radians(offset)) * THICK_LINE;
                const double y = sin(radians(global.clock_data.hour * 30) - radians(3 * 30) + radians(offset)) * THICK_LINE;
                global.tft.drawLine(CLOCK_X_POS, CLOCK_Y_POS, lround(CLOCK_X_POS + x), lround(CLOCK_Y_POS + y), swapRB(0xD822));
            }
        }
    }
}
