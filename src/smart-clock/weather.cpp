#include <ArduinoJson.h>

#include "weather.h"
#include "global.h"
#include "http_request.h"

static const char* KEY = "5b21b94798ac261edec0f06f12e72f62";

static const float A = -0.9f;
static const float B = 0.0f;
static const float C = 0.9f;

static const float SUN_RADIUS = 5.0f;

static const short SUN_COLOR = 0xFE09;
static const short SKY_COLOR = 0x9EFC;
static const short BORDER_COLOR = 0x9048;

namespace weather {
    static float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    static float sun_equation(float x) {
        return A * x * x + B * x + C;
    }

    static float calculate_sun_position(const Time& sunrise_time, const Time& sunset_time, const Time& current_time) {
        const unsigned long sunrise = convert_24hour_to_raw(sunrise_time);
        const unsigned long sunset = convert_24hour_to_raw(sunset_time);
        const unsigned long current = convert_24hour_to_raw(current_time);

        return mapf((float) current, (float) sunrise, (float) sunset, -1.0f, 1.0f);
    }

    static bool get_world_position_from_internet() {
        const String result = http_request::get("http://ip-api.com/json/");

        if (result.isEmpty()) {
            Serial.println("Couldn't get world position from internet");
            return false;
        }

        DynamicJsonDocument json {1024};
        deserializeJson(json, result.c_str());

        const float latitude = json["lat"];
        const float longitude = json["lon"];

        g.weather_data.latitude = latitude;
        g.weather_data.longitude = longitude;

        Serial.printf("Got world position: %f, %f\n", latitude, longitude);
        return true;
    }
    
    static bool get_weather_from_internet() {
        char url[256];
        sprintf(
            url,
            "http://api.openweathermap.org/data/2.5/onecall?lat=%f&lon=%f&exclude=daily,alerts,hourly,minutely&appid=%s&units=metric",
            g.weather_data.latitude,
            g.weather_data.longitude,
            KEY
        );

        const String result = http_request::get(url);

        if (result.isEmpty()) {
            Serial.println("Couldn't get weather from internet");
            return false;
        }

        DynamicJsonDocument json {1024};
        deserializeJson(json, result.c_str());

        const float temperature = json["current"]["temp"];
        const unsigned int humidity = json["current"]["humidity"];
        const unsigned int cloudiness = json["current"]["clouds"];
        const int sunrise = json["current"]["sunrise"];
        const int sunset = json["current"]["sunset"];

        g.weather_data.weather_condition.clear();
        g.weather_data.weather_condition += "Weather: ";

        const JsonArray weather = json["current"]["weather"];

        for (const JsonObject& object : weather) {
            const char* condition = object["main"];
            g.weather_data.weather_condition += condition;
            g.weather_data.weather_condition += " ";
        }

        g.weather_data.outside_temperature = temperature;
        g.weather_data.outside_humidity = humidity;
        g.weather_data.cloudiness = cloudiness;
        g.weather_data.sunrise = sunrise;
        g.weather_data.sunset = sunset;  

        Serial.printf("Got weather: T=%f, H=%u, sunrise=%d, sunset=%d\n", temperature, humidity, sunrise, sunset);
        return true;
    }

    static void update_room_temperature_and_humidity() {
        const float temperature = g.dht.readTemperature();
        if (isnan(temperature)) {
            Serial.println("Failed updating room temperature");
            return;
        }

        g.weather_data.room_temperature = temperature;

        const float humidity = g.dht.readHumidity();
        if (isnan(humidity)) {
            Serial.println("Failed updating room humidity");
            return;
        }

        g.weather_data.room_humidity = humidity;

        Serial.println("Updated room temperature and humidity");

        g.weather_data.room_temp_last_updated_sec = g.clock_data.raw_time_sec;
    }

    static void update_weather() {
        if (!g.weather_data.got_world_position) {
            if (!get_world_position_from_internet()) {
                Serial.println("Error getting the world position");
            } else {
                g.weather_data.got_world_position = true;
            }
        }

        bool success = false;

        if (g.weather_data.got_world_position) {
            success = get_weather_from_internet();    
        }

        if (!success) {
            g.weather_data.update_failures++;

            // Give up on trying to update every minute
            if (g.weather_data.update_failures == 3) {
                g.weather_data.update_phase = UpdatePhase::ThirtyMinutes;
                g.weather_data.update_failures = 0;

                Serial.println("Give up on retrying to update weather");
            } else {
                g.weather_data.update_phase = UpdatePhase::OneMinute;

                Serial.println("Retrying to update weather after one minute...");
            }

            g.weather_data.last_updated_sec = g.clock_data.raw_time_sec;
            return;  // Don't do anything else
        }

        Serial.println("Updated weather");

        g.weather_data.update_phase = UpdatePhase::ThirtyMinutes;
        g.weather_data.last_updated_sec = g.clock_data.raw_time_sec;
    }

    void start_draw() {
        // Clear screen
        g.tft.fillScreen(ST77XX_BLACK);

        // Draw background
        g.tft.fillRect(0, 0, g.tft.width(), 97, color(SKY_COLOR));

        {
            // Draw sun
            const float x = mapf(g.weather_data.sun_position, -1.0f, 1.0f, SUN_RADIUS, g.tft.width() - SUN_RADIUS);
            const float y = mapf(sun_equation(g.weather_data.sun_position), 0.0f, 0.9f, 98.0f + SUN_RADIUS, SUN_RADIUS);
            g.tft.fillCircle(lround(x), lround(y), SUN_RADIUS, color(SUN_COLOR));
        }

        // Draw border
        g.tft.drawFastVLine(0, 0, 97, color(BORDER_COLOR));
        g.tft.drawFastHLine(0, 0, g.tft.width(), color(BORDER_COLOR));
        g.tft.drawFastVLine(g.tft.width() - 1, 0, 97, color(BORDER_COLOR));
        g.tft.drawFastHLine(0, 97, g.tft.width(), color(BORDER_COLOR));

        // Draw temperatures
        g.tft.fillRect(0, 98, g.tft.width(), g.tft.height(), ST77XX_BLACK);
        
        char temperatures[16];
        sprintf(temperatures, "%.1fC %.1fC", g.weather_data.room_temperature,
                g.weather_data.outside_temperature);

        g.tft.setTextColor(ST77XX_WHITE);
        g.tft.setTextSize(2);
        g.tft.setTextWrap(false);

        int16_t x, y;
        uint16_t w, h;
        g.tft.getTextBounds(temperatures, 0, 0, &x, &y, &w, &h);

        g.tft.setCursor(g.tft.width() / 2 - w / 2, 106);
        g.tft.print(temperatures);
    }

    void update() {
        static unsigned long last_time = 0;

        static bool initialized = false;
        if (!initialized) {
            update_weather();
            update_room_temperature_and_humidity();
            initialized = true;
        }

        if (g.current_time - last_time >= M_ONE_SECOND) {
            last_time = g.current_time;

            switch (g.weather_data.update_phase) {
                case UpdatePhase::TwentyMinutes:
                    if (g.clock_data.raw_time_sec - g.weather_data.last_updated_sec >= S_TWENTY_MINUTES) {
                        update_weather();
                    }
                    break;
                case UpdatePhase::OneMinute:
                    if (g.clock_data.raw_time_sec - g.weather_data.last_updated_sec >= S_ONE_MINUTE) {
                        update_weather();
                    }
                    break;
                default:
                    break;
            }

            switch (g.weather_data.room_temp_update_phase) {
                case UpdatePhase::OneMinute:
                    if (g.clock_data.raw_time_sec - g.weather_data.room_temp_last_updated_sec >= S_ONE_MINUTE) {
                        update_room_temperature_and_humidity();
                    }
                    break;
                default:
                    break;
            }
        }

        const Time sunrise = get_time_from_unix_time(g.weather_data.sunrise);
        const Time sunset = get_time_from_unix_time(g.weather_data.sunset);
        const Time current_time = { g.clock_data.hour, g.clock_data.minute, g.clock_data.second };

        g.weather_data.sun_position = calculate_sun_position(sunrise, sunset, current_time);
        g.weather_data.sun_position = constrain(g.weather_data.sun_position, -1.0f, 1.0f);
    }

    void draw() {
        static unsigned long last_time = 0;

        if (g.current_time - last_time >= M_FIVE_SECONDS) {
            last_time = g.current_time;

            g.tft.fillRect(0, 0, g.tft.width(), 97, color(SKY_COLOR));

            {
                const float x = mapf(g.weather_data.sun_position, -1.0f, 1.0f, SUN_RADIUS, g.tft.width() - SUN_RADIUS);
                const float y = mapf(sun_equation(g.weather_data.sun_position), 0.0f, 0.9f, 98.0f + SUN_RADIUS, SUN_RADIUS);
                g.tft.fillCircle(lround(x), lround(y), SUN_RADIUS, color(SUN_COLOR));
            }

            g.tft.drawFastVLine(0, 0, 97, color(BORDER_COLOR));
            g.tft.drawFastHLine(0, 0, g.tft.width(), color(BORDER_COLOR));
            g.tft.drawFastVLine(g.tft.width() - 1, 0, 97, color(BORDER_COLOR));
            g.tft.drawFastHLine(0, 97, g.tft.width(), color(BORDER_COLOR));

            g.tft.fillRect(0, 98, g.tft.width(), g.tft.height(), ST77XX_BLACK);

            char temperatures[16];
            sprintf(temperatures, "%.1fC %.1fC", g.weather_data.room_temperature,
                    g.weather_data.outside_temperature);

            g.tft.setTextColor(ST77XX_WHITE);
            g.tft.setTextSize(2);
            g.tft.setTextWrap(false);

            int16_t x, y;
            uint16_t w, h;
            g.tft.getTextBounds(temperatures, 0, 0, &x, &y, &w, &h);

            g.tft.setCursor(g.tft.width() / 2 - w / 2, 106);
            g.tft.print(temperatures);
        }
    }
}
