#ifndef ANALOG_CLOCK_H
#define ANALOG_CLOCK_H

#include "update_phase.h"

namespace analog_clock {
    struct Data {
        int utc_offset = 0;  // Got from the internet
        int dst = 0;

        unsigned long raw_time_sec;  // Number of seconds since 00:00 (midnight)
        unsigned long hour;  // 24-hour
        unsigned long minute;
        unsigned long second;

        unsigned long day = 1;
        unsigned long month = 1;
        unsigned long year = 2021;

        UpdatePhase update_phase = UpdatePhase::ThirtyMinutes;
        unsigned int update_failures = 0;
        unsigned long last_updated_sec = 0;  // Measured using raw_time_sec
    };

    void start_draw();
    void update();
    void draw();
}

#endif  // ANALOG_CLOCK_H
