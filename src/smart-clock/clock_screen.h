#ifndef CLOCK_SCREEN_H
#define CLOCK_SCREEN_H

namespace clock_screen {
    struct Data {
        unsigned long raw_time;
        unsigned int hours;
        unsigned int minutes;
        unsigned int seconds;
        
        unsigned int day;
        unsigned int month;
        unsigned int year;
    };

    void screen();
}

#endif  // CLOCK_SCREEN_H
