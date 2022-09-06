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
    static void convert_raw_to_24hour(unsigned long raw_time_sec, Data* clock_data) {
        clock_data->hour = raw_time_sec / 3600;
        raw_time_sec -= clock_data->hour * 3600;

        clock_data->minute = raw_time_sec / 60;
        raw_time_sec -= clock_data->minute * 60;

        clock_data->second = raw_time_sec;
    }

    static void parse_datetime(const char* datetime, unsigned long* year, unsigned long* month, unsigned long* day,
            unsigned long* hour, unsigned long* minute, unsigned long* second) {
        char copy[64];
        strncpy(copy, datetime, 64);

        char* token;

        token = strtok(copy, "-T:.");  // Example: 2021-11-23T23:20:30.358093+02:00
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
        const String result = http_request::get("http://worldtimeapi.org/api/ip");

        if (result.isEmpty()) {
            Serial.println("Couldn't get time from internet");
            return false;
        }

        DynamicJsonDocument json {1024};
        deserializeJson(json, result.c_str());

        const char* datetime = json["datetime"];

        unsigned long year, month, day, hour, minute, second;
        parse_datetime(datetime, &year, &month, &day, &hour, &minute, &second);

        g.clock_data.hour = hour;
        g.clock_data.minute = minute;
        g.clock_data.second = second + 3;  // Add an offset accounting for this delay
        g.clock_data.day = day;
        g.clock_data.month = month;
        g.clock_data.year = year;

        const char* utc_offset = json["utc_offset"];

        char offset[8];
        memcpy(offset, utc_offset, sizeof(char) * 3);
        offset[3] = 0;
        g.clock_data.utc_offset = atoi(offset);

        g.clock_data.raw_time_sec = convert_24hour_to_raw(
            { g.clock_data.hour, g.clock_data.minute, g.clock_data.second }
        );

        Serial.printf("Got time: %lu-%02lu-%02lu %02lu:%02lu:%02lu\n", year, month, day, hour, minute, second);
        return true;
    }

    static void draw_date() {
        g.tft.fillRect(0, 101, g.tft.width(), g.tft.height() - 101, ST77XX_BLACK);

        char date[16];
        sprintf(date, "%s %02lu %lu", MONTHS[g.clock_data.month], g.clock_data.day,
                g.clock_data.year);

        g.tft.setTextColor(ST77XX_WHITE);
        g.tft.setTextSize(2);
        g.tft.setTextWrap(false);

        int16_t x, y;
        uint16_t w, h;
        g.tft.getTextBounds(date, 0, 0, &x, &y, &w, &h);

        g.tft.setCursor(g.tft.width() / 2 - w / 2, 102);
        g.tft.print(date);
    }

    static void update_clock() {
        if (!get_time_from_internet()) {
            g.clock_data.update_failures++;

            // Give up on trying to update every minute
            if (g.clock_data.update_failures == 3) {
                g.clock_data.update_phase = UpdatePhase::ThirtyMinutes;
                g.clock_data.update_failures = 0;

                Serial.println("Give up on retrying to update clock");
            } else {
                g.clock_data.update_phase = UpdatePhase::OneMinute;

                Serial.println("Retrying to update clock after one minute...");
            }

            g.clock_data.last_updated_sec = g.clock_data.raw_time_sec;
            return;  // Don't do anything else
        }

        draw_date();

        Serial.println("Updated clock");

        g.clock_data.update_phase = UpdatePhase::ThirtyMinutes;
        g.clock_data.last_updated_sec = g.clock_data.raw_time_sec;
    }

    void start_draw() {
        // Clear screen
        g.tft.fillScreen(ST77XX_BLACK);

        // Draw background
        g.tft.fillCircle(CLOCK_X_POS, CLOCK_Y_POS, CLOCK_RADIUS, color(0x0005));

        // Draw dots
        g.tft.fillRect(80, 1, 2, 2, ST77XX_WHITE);  // 12
        g.tft.fillRect(105, 8, 2, 2, ST77XX_WHITE);  // 1
        g.tft.fillRect(121, 26, 2, 2, ST77XX_WHITE);  // 2
        g.tft.fillRect(126, 47, 2, 2, ST77XX_WHITE);  // 3
        g.tft.fillRect(121, 68, 2, 2, ST77XX_WHITE);  // 4
        g.tft.fillRect(105, 86, 2, 2, ST77XX_WHITE);  // 5
        g.tft.fillRect(80, 93, 2, 2, ST77XX_WHITE);  // 6
        g.tft.fillRect(55, 86, 2, 2, ST77XX_WHITE);  // 7
        g.tft.fillRect(39, 68, 2, 2, ST77XX_WHITE);  // 8
        g.tft.fillRect(34, 47, 2, 2, ST77XX_WHITE);  // 9
        g.tft.fillRect(39, 26, 2, 2, ST77XX_WHITE);  // 10
        g.tft.fillRect(55, 8, 2, 2, ST77XX_WHITE);  // 11

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

        if (g.current_time - last_time >= M_ONE_SECOND) {
            last_time = g.current_time;
            g.clock_data.raw_time_sec++;

            switch (g.clock_data.update_phase) {
                case UpdatePhase::ThirtyMinutes:
                    if (g.clock_data.raw_time_sec - g.clock_data.last_updated_sec >= S_THIRTY_MINUTES) {
                        update_clock();    
                    }
                    break;
                case UpdatePhase::OneMinute:
                    if (g.clock_data.raw_time_sec - g.clock_data.last_updated_sec >= S_ONE_MINUTE) {
                        update_clock();
                    }
                    break;
                default:
                    break;
            }

            // Update when it reaches 24 hours and 10 seconds
            if (g.clock_data.raw_time_sec == (S_TWENTYFOUR_HOURS + S_TEN_SECONDS)) {
                g.clock_data.raw_time_sec = S_TEN_SECONDS;
                update_clock();
            }

            convert_raw_to_24hour(g.clock_data.raw_time_sec, &g.clock_data);
        }
    }

    void draw() {
        static unsigned long last_time = 0;

        if (g.current_time - last_time >= M_ONE_SECOND) {
            last_time = g.current_time;

            // Draw lines background
            g.tft.fillCircle(CLOCK_X_POS, CLOCK_Y_POS, THIN_LINE + 1, color(0x0005));

            {
                // Draw thin line (second)
                const double x = cos(radians(g.clock_data.second * 6) - radians(15 * 6)) * THIN_LINE;
                const double y = sin(radians(g.clock_data.second * 6) - radians(15 * 6)) * THIN_LINE;
                g.tft.drawLine(CLOCK_X_POS, CLOCK_Y_POS, lround(CLOCK_X_POS + x), lround(CLOCK_Y_POS + y), color(0xEF5F));
            }
            {
                // Draw medium line (minute)
                const long offset = map(g.clock_data.second, 0, 60, 0, 6);
                const double x = cos(radians(g.clock_data.minute * 6) - radians(15 * 6) + radians(offset)) * MEDIUM_LINE;
                const double y = sin(radians(g.clock_data.minute * 6) - radians(15 * 6) + radians(offset)) * MEDIUM_LINE;
                g.tft.drawLine(CLOCK_X_POS, CLOCK_Y_POS, lround(CLOCK_X_POS + x), lround(CLOCK_Y_POS + y), color(0x8C7F));
            }
            {
                // Draw thick line (hour)
                const long offset = map(g.clock_data.minute, 0, 60, 0, 30);
                const double x = cos(radians(g.clock_data.hour * 30) - radians(3 * 30) + radians(offset)) * THICK_LINE;
                const double y = sin(radians(g.clock_data.hour * 30) - radians(3 * 30) + radians(offset)) * THICK_LINE;
                g.tft.drawLine(CLOCK_X_POS, CLOCK_Y_POS, lround(CLOCK_X_POS + x), lround(CLOCK_Y_POS + y), color(0xD822));
            }
        }
    }
}
