#ifndef WEATHER_H
#define WEATHER_H

namespace weather {
    struct Data {
        float room_temperature = 0.0f;
        unsigned int room_humidity = 0;  // TODO get this too
        float outside_temperature = 0.0f;
        unsigned int outside_humidity = 0;
        int sunrise = 0;
        int sunset = 0;

        float sun_position = -1.0f;  // Between -1.0 and 1.0

        float latitude = 0.0f;
        float longitude = 0.0f;

        unsigned long raw_time;  // Number of seconds since 00:00 (midnight)
        bool reupdate_after_one_minute = false;
        unsigned long last_updated = 0;  // Measured using raw_time
        unsigned int reupdate_tries = 0;

        unsigned long temperature_last_updated = 0;  // Measured using raw_time
    };

    void start_draw();
    void update();
    void draw();
}

#endif  // WEATHER_H
