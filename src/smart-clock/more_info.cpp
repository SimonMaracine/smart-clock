#include "more_info.h"
#include "global.h"

static const int INFO_OFFSET = 10;
static const int INFO_SPACING = 14;

namespace more_info {
    static void _draw_info() {
        const Time rise = get_time_from_unix_time(g.weather_data.sunrise);
        const Time set = get_time_from_unix_time(g.weather_data.sunset);

        char sunrise[32];
        sprintf(sunrise, "Sunrise: %lu:%lu:%lu", rise.hour, rise.minute, rise.second);

        char sunset[32];
        sprintf(sunset, "Sunset: %lu:%lu:%lu", set.hour, set.minute, set.second);

        char utc_offset[32];
        sprintf(utc_offset, "UTC offset: %d", g.clock_data.utc_offset);

        char dst[32];
        sprintf(dst, "DST: %d", g.clock_data.dst);

        char latitude[32];
        sprintf(latitude, "Latitude: %f", g.weather_data.latitude);

        char longitude[32];
        sprintf(longitude, "Longitude: %f", g.weather_data.longitude);

        g.tft.setTextColor(ST77XX_WHITE);
        g.tft.setTextSize(1);
        g.tft.setTextWrap(false);

        g.tft.setCursor(INFO_OFFSET, INFO_SPACING);
        g.tft.print(sunrise);

        g.tft.setCursor(INFO_OFFSET, INFO_SPACING * 2);
        g.tft.print(sunset);

        g.tft.setCursor(INFO_OFFSET, INFO_SPACING * 3);
        g.tft.print(utc_offset);

        g.tft.setCursor(INFO_OFFSET, INFO_SPACING * 4);
        g.tft.print(dst);

        g.tft.setCursor(INFO_OFFSET, INFO_SPACING * 5);
        g.tft.print(latitude);

        g.tft.setCursor(INFO_OFFSET, INFO_SPACING * 6);
        g.tft.print(longitude);
    }
    
    void start_draw() {
        // Clear screen
        g.tft.fillScreen(ST77XX_BLACK);

        // Draw information
        _draw_info();
    }

    void draw() {
        static unsigned long last_time = 0;

        if (g.current_time - last_time >= M_FIVE_SECONDS) {
            last_time = g.current_time;

            g.tft.fillScreen(ST77XX_BLACK);

            _draw_info();
        }
    }
}
