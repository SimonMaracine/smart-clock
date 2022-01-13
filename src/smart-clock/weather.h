#ifndef WEATHER_H
#define WEATHER_H

namespace weather {
    struct Data {
        int room_temperature = 0;
        int outside_temperature = 0;
        unsigned int humidity = 0;
        long sunrise = 0;
        long sunset = 0;

        float sun_position = -1.0f;  // Between -1.0 and 1.0

        float latitude = 0.0f;
        float longitude = 0.0f;
    };

    void start_draw();
    void update();
    void draw();
}

#endif  // WEATHER_H
