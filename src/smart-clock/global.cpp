#include <SoftwareSerial.h>
#include <TimeLib.h>

#include "global.h"
#include "analog_clock.h"
#include "weather.h"
#include "more_weather.h"
#include "more_info.h"

void change_screen(Screen screen) {
    g.current_screen = screen;
    
    switch (screen) {
        case Screen::Clock:
            g.current_screen_func = analog_clock::draw;
            break;
        case Screen::Weather:
            g.current_screen_func = weather::draw;
            break;
        case Screen::MoreWeather:
            g.current_screen_func = more_weather::draw;
            break;
        case Screen::MoreInfo:
            g.current_screen_func = more_info::draw;
            break;
    }

    Serial.println("Changed screen");
}

bool is_button_pressed(bool* button) {
    if (button[0] && !button[1]) {
        return true;
    } else {
        return false;
    }
}

Time get_time_from_unix_time(int unix_time) {
    // These time functions don't take into account UTC and DST
    const int hours = hour(unix_time) + g.clock_data.utc_offset + g.clock_data.dst;
    const int minutes = minute(unix_time);
    const int seconds = second(unix_time);

    return { hours, minutes, seconds };
}

unsigned long convert_24hour_to_raw(const Time& time_struct) {
    unsigned long result = 0;

    result += time_struct.hour * 3600;
    result += time_struct.minute * 60;
    result += time_struct.second;

    return result;
}
