#include "more_weather.h"
#include "global.h"

static const int INFO_OFFSET = 10;
static const int INFO_SPACING = 14;

static const short BACKGROUND_COLOR = 0x084A;

namespace more_weather {
    static void _draw_info() {
        char room_temperature[32];
        sprintf(room_temperature, "Room temp: %.1fC", g.weather_data.room_temperature);

        char outside_temperature[32];
        sprintf(outside_temperature, "Outside temp: %.1fC", g.weather_data.outside_temperature);

        char room_humidity[32];
        sprintf(room_humidity, "Room hum: %u%%", g.weather_data.room_humidity);

        char outside_humidity[32];
        sprintf(outside_humidity, "Outside hum: %u%%", g.weather_data.outside_humidity);

        char cloudiness[32];
        sprintf(cloudiness, "Cloudiness: %u%%", g.weather_data.cloudiness);

        char weather_condition[50];
        strncpy(weather_condition, g.weather_data.weather_condition.c_str(), 50);

        g.tft.setTextColor(ST77XX_WHITE);
        g.tft.setTextSize(1);
        g.tft.setTextWrap(false);

        g.tft.setCursor(INFO_OFFSET, INFO_SPACING);
        g.tft.print(room_temperature);

        g.tft.setCursor(INFO_OFFSET, INFO_SPACING * 2);
        g.tft.print(outside_temperature);

        g.tft.setCursor(INFO_OFFSET, INFO_SPACING * 3);
        g.tft.print(room_humidity);

        g.tft.setCursor(INFO_OFFSET, INFO_SPACING * 4);
        g.tft.print(outside_humidity);

        g.tft.setCursor(INFO_OFFSET, INFO_SPACING * 5);
        g.tft.print(cloudiness);

        g.tft.setTextWrap(true);

        g.tft.setCursor(INFO_OFFSET, INFO_SPACING * 6);
        g.tft.print(weather_condition);
    }

    void start_draw() {
        // Clear screen
        g.tft.fillScreen(color(BACKGROUND_COLOR));

        // Draw information
        _draw_info();
    }

    void draw() {
        static unsigned long last_time = 0;

        if (g.current_time - last_time >= M_FIVE_SECONDS) {
            last_time = g.current_time;

            g.tft.fillScreen(color(BACKGROUND_COLOR));

            _draw_info();
        }
    }
}
