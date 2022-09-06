#ifndef WEATHER_H
#define WEATHER_H

#include "update_phase.h"

namespace weather {
    struct Data {
        float room_temperature = 0.0f;
        unsigned int room_humidity = 0;
        float outside_temperature = 0.0f;
        unsigned int outside_humidity = 0;
        unsigned int cloudiness = 0;
        String weather_condition = "";

        int sunrise = 0;
        int sunset = 0;

        float sun_position = -1.0f;  // Between -1.0 and 1.0

        float latitude = 0.0f;  // Got from the internet
        float longitude = 0.0f;
        bool got_world_position = false;

        UpdatePhase update_phase = UpdatePhase::ThirtyMinutes;
        unsigned int update_failures = 0;
        unsigned long last_updated_sec = 0;  // Measured using raw_time_sec

        UpdatePhase room_temp_update_phase = UpdatePhase::OneMinute;
        unsigned long room_temp_last_updated_sec = 0;  // Measured using raw_time_sec
    };

    void start_draw();
    void update();
    void draw();
}

#endif  // WEATHER_H
