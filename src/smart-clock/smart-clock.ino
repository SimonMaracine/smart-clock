#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <ESP8266WiFi.h>

#include "analog_clock.h"
#include "global.h"

GlobalData global;
const char* ssid = "WWJD";
const char* password = "4wwJdtoday?";

static void check_connection() {
    static unsigned long last_time = 0;
    unsigned long current_time = millis();

    if (current_time - last_time >= M_THIRTY_SECONDS) {
        last_time = current_time;

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

void setup() {
    DSERIAL.begin(9600);

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

    change_screen(analog_clock::draw);
}

void loop() {
    global.clock_data.current_time = millis();

    analog_clock::update();
    global.current_screen();
    check_connection();

    delay(1);
}
