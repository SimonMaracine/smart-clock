#ifndef ANALOG_CLOCK_H
#define ANALOG_CLOCK_H

namespace analog_clock {
    struct Data {
        unsigned long raw_time;  // Number of seconds since 00:00 (midnight)
        unsigned long hour;
        unsigned long minute;
        unsigned long second;
        
        unsigned long day = 1;
        unsigned long month = 1;
        unsigned long year = 2021;

        bool reupdate_after_one_minute = false;
        unsigned long last_updated = 0;  // Measured using raw_time
        unsigned int reupdate_tries = 0;
    };

    void start_draw();
    void update();
    void draw();
}

#endif  // ANALOG_CLOCK_H
