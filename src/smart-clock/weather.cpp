#include <ArduinoJson.h>

#include "weather.h"
#include "global.h"
#include "http_request.h"

static const char* key = "5b21b94798ac261edec0f06f12e72f62";

static const float A = -0.9f;
static const float B = 0.0f;
static const float C = 0.9f;

static const float SUN_RADIUS = 5.0f;

namespace weather {
    static float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    static float sun_equation(float x) {
        return A * x * x + B * x + C;    
    }

    static float calculate_sun_position(const Time& sunrise_time, const Time& sunset_time, const Time& curent_time) {
        const unsigned long sunrise = convert_24hour_to_raw(sunrise_time);
        const unsigned long sunset = convert_24hour_to_raw(sunset_time);
        const unsigned long current = convert_24hour_to_raw(curent_time);

        return mapf((float) current, (float) sunrise, (float) sunset, -1.0f, 1.0f);
    }

    static bool get_world_position_from_internet() {
        String result = http_request::get("http://ip-api.com/json/");

        if (result == "") {
            DSERIAL.println("Couldn't get world position from internet");
            return false;
        }

        StaticJsonDocument<256> json;
        deserializeJson(json, result.c_str());

        float latitude = json["lat"];
        float longitude = json["lon"];

        global.weather_data.latitude = latitude;
        global.weather_data.longitude = longitude;

        DSERIAL.printf("Got world position: %f, %f\n", latitude, longitude);
        return true;
    }
    
    static bool get_weather_from_internet() {
        char url[256];
        sprintf(url, "http://api.openweathermap.org/data/2.5/onecall?lat=%f&lon=%f&exclude=daily,alerts,hourly,minutely&appid=%s&units=metric",
                global.weather_data.latitude, global.weather_data.longitude, key);

        String result = http_request::get(url);

        if (result == "") {
            DSERIAL.println("Couldn't get weather from internet");
            return false;
        }

        StaticJsonDocument<512> json;
        deserializeJson(json, result.c_str());

        float temperature = json["current"]["temp"];
        unsigned int humidity = json["current"]["humidity"];
        int sunrise = json["current"]["sunrise"];
        int sunset = json["current"]["sunset"];

        global.weather_data.outside_temperature = temperature;
        global.weather_data.humidity = humidity;
        global.weather_data.sunrise = sunrise;
        global.weather_data.sunset = sunset;

        DSERIAL.printf("Got weather: T=%f, H=%u, sunrise=%d, sunset=%d\n", temperature, humidity, sunrise, sunset);
        return true;
    }

    static void update_room_temperature() {
        
    }

    static void update_weather() {
        bool success1 = get_world_position_from_internet();
        bool success2 = get_weather_from_internet();

        if (!success1 || !success2) {
            DSERIAL.println("Retrying to update weather after one minute...");

            if (global.weather_data.reupdate_tries < 5) {
                global.weather_data.reupdate_after_one_minute = true;    
            } else {
                global.weather_data.reupdate_tries = 0;
            }

            global.weather_data.last_updated = global.weather_data.raw_time;
            return;  // Don't do anything else
        }

        DSERIAL.println("Updated weather");
        global.weather_data.reupdate_after_one_minute = false;  // Don't update after one minute, if it succeeded
        global.weather_data.reupdate_tries = 0;

        global.weather_data.last_updated = global.weather_data.raw_time;
    }

    void start_draw() {
        // Clear screen
        global.tft.fillScreen(ST77XX_BLACK);

        // Draw background
        global.tft.fillRect(0, 0, global.tft.width(), 97, swapRB(0x969E));

        {
            // Draw sun
            const float x = mapf(global.weather_data.sun_position, -1.0f, 1.0f, SUN_RADIUS, global.tft.width() - SUN_RADIUS);
            const float y = mapf(sun_equation(global.weather_data.sun_position), 0.0f, 0.9f, 98.0f + SUN_RADIUS, SUN_RADIUS);
            global.tft.fillCircle(lround(x), lround(y), SUN_RADIUS, swapRB(0xFFE0));
        }

        // Draw border
        global.tft.drawFastVLine(0, 0, 97, swapRB(0xF800));
        global.tft.drawFastHLine(0, 0, global.tft.width(), swapRB(0xF800));
        global.tft.drawFastVLine(global.tft.width() - 1, 0, 97, swapRB(0xF800));
        global.tft.drawFastHLine(0, 97, global.tft.width(), swapRB(0xF800));

        // Draw temperatures
        global.tft.fillRect(0, 98, global.tft.width(), global.tft.height(), ST77XX_BLACK);
        
        char temperatures[16];
        sprintf(temperatures, "%d%cC    %d%cC", global.weather_data.room_temperature, 0xF7,
                global.weather_data.outside_temperature, 0xF7);

        global.tft.setTextColor(ST77XX_WHITE);
        global.tft.setTextSize(2);
        global.tft.setTextWrap(false);

        int16_t x, y;
        uint16_t w, h;
        global.tft.getTextBounds(temperatures, 0, 0, &x, &y, &w, &h);

        global.tft.setCursor(global.tft.width() / 2 - w / 2, 106);
        global.tft.print(temperatures);
    }

    void update() {
        static unsigned long last_time = 0;

        static bool initialized = false;
        if (!initialized) {
            update_weather();
            initialized = true;
        }

        if (global.current_time - last_time >= M_ONE_SECOND) {
            last_time = global.current_time;
            global.weather_data.raw_time++;

            if (global.weather_data.reupdate_after_one_minute) {
                if (global.weather_data.raw_time - global.weather_data.last_updated >= S_ONE_MINUTE) {
                    global.weather_data.reupdate_after_one_minute = false;
                    global.weather_data.reupdate_tries++;
                    update_weather();
                }
            }

            // Try to update once every thirty minutes
            if (global.weather_data.raw_time - global.weather_data.last_updated >= S_THIRTY_MINUTES) {
                update_weather();    
            }

            // Update when it reaches 24 hours
            if (global.weather_data.raw_time == S_TWENTYFOUR_HOURS) {
                global.weather_data.raw_time = 0;
            }
        }

        const Time sunrise = get_time_from_unix_time(global.weather_data.sunrise);  // { 6, 30, 0 };
        const Time sunset = get_time_from_unix_time(global.weather_data.sunset);  // { 20, 10, 0 };
        const Time current_time = { global.clock_data.hour, global.clock_data.minute, global.clock_data.second };

        DSERIAL.printf("Sunrise %u:%u:%u\n", sunrise.hour, sunrise.minute, sunrise.second);
        DSERIAL.printf("Sunset %u:%u:%u\n", sunset.hour, sunset.minute, sunset.second);

        global.weather_data.sun_position = calculate_sun_position(sunrise, sunset, current_time);
        global.weather_data.sun_position = constrain(global.weather_data.sun_position, -1.0f, 1.0f);

//        DSERIAL.println(global.weather_data.sun_position);
    }

    void draw() {
        static unsigned long last_time = 0;

        if (global.current_time - last_time >= M_FIVE_SECONDS) {
            last_time = global.current_time;

            global.tft.fillRect(0, 0, global.tft.width(), 97, swapRB(0x969E));

            {
                const float x = mapf(global.weather_data.sun_position, -1.0f, 1.0f, SUN_RADIUS, global.tft.width() - SUN_RADIUS);
                const float y = mapf(sun_equation(global.weather_data.sun_position), 0.0f, 0.9f, 98.0f + SUN_RADIUS, SUN_RADIUS);
                global.tft.fillCircle(lround(x), lround(y), SUN_RADIUS, swapRB(0xFFE0));
            }

            global.tft.drawFastVLine(0, 0, 97, swapRB(0xF800));
            global.tft.drawFastHLine(0, 0, global.tft.width(), swapRB(0xF800));
            global.tft.drawFastVLine(global.tft.width() - 1, 0, 97, swapRB(0xF800));
            global.tft.drawFastHLine(0, 97, global.tft.width(), swapRB(0xF800));

            global.tft.fillRect(0, 98, global.tft.width(), global.tft.height(), ST77XX_BLACK);

            char temperatures[16];
            sprintf(temperatures, "%d%cC    %d%cC", global.weather_data.room_temperature, 0xF7,
                    global.weather_data.outside_temperature, 0xF7);

            global.tft.setTextColor(ST77XX_WHITE);
            global.tft.setTextSize(2);
            global.tft.setTextWrap(false);

            int16_t x, y;
            uint16_t w, h;
            global.tft.getTextBounds(temperatures, 0, 0, &x, &y, &w, &h);

            global.tft.setCursor(global.tft.width() / 2 - w / 2, 106);
            global.tft.print(temperatures);
        }
    }
}
