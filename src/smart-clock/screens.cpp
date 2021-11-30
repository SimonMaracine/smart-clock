#include <SoftwareSerial.h>

#include "screens.h"

void change_screen(screen_mode_func screen) {
    current_screen = screen;
    Serial.println("Changed screen");
}
