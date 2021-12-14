#include <SoftwareSerial.h>

#include "global.h"
#include "analog_clock.h"
#include "weather.h"

void change_screen(Screen screen) {
    global.current_screen = screen;
    
    switch (screen) {
        case Screen::Clock:
            global.current_screen_func = analog_clock::draw;
            break;
        case Screen::Weather:
            global.current_screen_func = weather::draw;
            break;
    }

    DSERIAL.println("Changed screen");
}

bool is_button_pressed(bool* button) {
    if (button[0] && !button[1]) {
        return true;
    } else {
        return false;
    }
}

Time get_time_from_unix_time(int unix_time) {
    unsigned long hours = 0;
    unsigned long minutes = 0;
    unsigned long seconds = 0;



    return { hours, minutes, seconds };
}

unsigned long convert_24hour_to_raw(const Time& time_struct) {
    unsigned long result = 0;

    result += time_struct.hour * 3600;
    result += time_struct.minute * 60;
    result += time_struct.second;

    return result;
}
