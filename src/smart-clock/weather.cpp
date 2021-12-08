#include <ArduinoJson.h>

#include "weather.h"
#include "global.h"
#include "http_request.h"

static const char* key = "5b21b94798ac261edec0f06f12e72f62";

static const float A = -0.9f;
static const float B = 0.0f;
static const float C = 0.9f;

static const float SUN_RADIUS = 10.0f;

namespace weather {
    static float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    static float sun_equation(float x) {
        return A * x * x + B * x + C;    
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
        sprintf(url, "https://api.openweathermap.org/data/2.5/onecall?lat=%f&lon=%f&exclude=daily,alerts&appid=%s&units=metric",
                global.weather_data.latitude, global.weather_data.longitude, key);

        String result = http_request::get(url);

        if (result == "") {
            DSERIAL.println("Couldn't get weather from internet");
            return false;
        }

        StaticJsonDocument<256> json;
        deserializeJson(json, result.c_str());

        float temperature = json["current"]["temp"];
        unsigned int humidity = json["current"]["humidity"];
        long sunrise = json["current"]["sunrise"];
        long sunset = json["current"]["sunset"];

        global.weather_data.outside_temperature = temperature;
        global.weather_data.humidity = humidity;

        DSERIAL.printf("Got weather: T=%f, H=%u, sunrise=%l, sunset=%l\n", temperature, humidity, sunrise, sunset);
        return true;
    }

    static void update_room_temperature() {
        
    }

    static void update_weather() {
        
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
        global.weather_data.sun_position += 0.001f * (global.delta_time * 0.01f);
        global.weather_data.sun_position = constrain(global.weather_data.sun_position, -1.0f, 1.0f);

//        DSERIAL.println(global.weather_data.sun_position);
    }

    void draw() {
        static unsigned long last_time = 0;

        if (global.current_time - last_time >= M_THREE_SECONDS) {
            last_time = global.current_time;

            global.tft.fillRect(0, 0, global.tft.width(), 97, swapRB(0x969E));

            {
                const float x = mapf(global.weather_data.sun_position, -1.0f, 1.0f, SUN_RADIUS, global.tft.width() - SUN_RADIUS);
                const float y = mapf(sun_equation(global.weather_data.sun_position), 0.0f, 0.9f, 98.0f + SUN_RADIUS, SUN_RADIUS);
                global.tft.fillCircle(lround(x), lround(y), 10, swapRB(0xFFE0));
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
