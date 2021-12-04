#ifndef CLOCK_SCREEN_H
#define CLOCK_SCREEN_H

namespace clock_screen {
    struct Data {
        unsigned long raw_time;  // Number of seconds since 00:00 (midnight)
        unsigned int hour;
        unsigned int minute;
        unsigned int second;
        
        unsigned int day;
        unsigned int month;
        unsigned int year;

        bool reinitialize_after_one_minute = false;
        unsigned long last_initialized = 0;  // Measured using raw_time
    };

    void screen();
}

#endif  // CLOCK_SCREEN_H
