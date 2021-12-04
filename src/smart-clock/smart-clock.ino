#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <ESP8266WiFi.h>

#include "clock_screen.h"
#include "global.h"

GlobalData global;
const char* ssid = "WWJD";
const char* password = "4wwJdtoday?";

static void check_connection() {
    static unsigned long last_time = 0;
    unsigned long current_time = millis();

    if (current_time - last_time >= THIRTY_SECONDS) {
        last_time = current_time;

        Serial.printf("Checking connection to %s...\n", ssid);

        if (WiFi.status() != WL_CONNECTED) {
            Serial.printf("Error. Trying to reconnect to %s...\n", ssid);
            WiFi.disconnect();
            WiFi.reconnect();
        } else {
            Serial.println("Connection is good");
        }
    }
}

void setup() {
    Serial.begin(9600);

    global.tft.initR(INITR_BLACKTAB);
    global.tft.setRotation(3);  // TODO maybe set this to 1 when putting everything together
    global.tft.fillScreen(ST77XX_BLACK);
    Serial.println("Initialized display");

    WiFi.begin(ssid, password);
    Serial.printf("Connecting to %s...\n", ssid);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.print("\n");

    Serial.println("Connection established!");  
    Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());

    change_screen(clock_screen::screen);
}

void loop() {
    global.current_screen();
    check_connection();
    delay(5);
}
