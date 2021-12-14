#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <ESP8266WiFi.h>

#include "analog_clock.h"
#include "weather.h"
#include "global.h"

GlobalData global;
static const char* ssid = "WWJD";
static const char* password = "4wwJdtoday?";

static void check_connection() {
    static unsigned long last_time = 0;

    if (global.current_time - last_time >= M_THIRTY_SECONDS) {
        last_time = global.current_time;

        DSERIAL.printf("Checking connection to %s...\n", ssid);

        if (WiFi.status() != WL_CONNECTED) {
            DSERIAL.printf("Error. Trying to reconnect to %s...\n", ssid);
            WiFi.disconnect();
            WiFi.reconnect();
        } else {
            DSERIAL.println("Connection is good");
        }
    }
}

static void get_input() {
    global.button[1] = global.button[0];
    global.button[0] = digitalRead(BUTTON);
}

void setup() {
    DSERIAL.begin(9600);

    pinMode(BUTTON, INPUT);

    global.tft.initR(INITR_BLACKTAB);
    global.tft.setRotation(3);  // TODO maybe set this to 1 when putting everything together
    global.tft.fillScreen(ST77XX_BLACK);
    DSERIAL.println("Initialized display");

    WiFi.begin(ssid, password);
    DSERIAL.printf("Connecting to %s...\n", ssid);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        DSERIAL.print(".");
    }
    DSERIAL.print("\n");

    DSERIAL.println("Connection established!");
    DSERIAL.printf("IP address: %s\n", WiFi.localIP().toString().c_str());

    change_screen(Screen::Clock);
    analog_clock::start_draw();
}

void loop() {
    static unsigned long last_time = millis();
    global.current_time = millis();
    global.delta_time = global.current_time - last_time;
    last_time = global.current_time;

    get_input();

    if (is_button_pressed(global.button)) {
        switch (global.current_screen) {
            case Screen::Clock:
                change_screen(Screen::Weather);
                weather::start_draw();
                break;
            case Screen::Weather:
                change_screen(Screen::Clock);
                analog_clock::start_draw();
                break;
        }
    }

    analog_clock::update();
    weather::update();
    global.current_screen_func();
    check_connection();
}
