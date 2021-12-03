#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
//#include <SPI.h>  // TODO is this needed?
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include "clock_screen.h"
#include "global.h"

GlobalData global;
const char* ssid = "WWJD";
const char* password = "4wwJdtoday?";

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

    if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;
        http.begin(client, "http://worldtimeapi.org/api/ip");

        int response_code = http.GET();

        if (response_code > 0) {
            Serial.printf("HTTP response code: %d\n", response_code);
            
            if (response_code == HTTP_CODE_OK || response_code == HTTP_CODE_MOVED_PERMANENTLY) {
                String payload = http.getString();
                Serial.println(payload);
            } else {
                Serial.println("Something went wrong...");
            }
        } else {
            Serial.printf("Error code: %d\n", response_code);
        }

        http.end();
    } else {
        Serial.println("WiFi disconnected");
    }
}

void loop() {
//    global.current_screen();
//    delay(5);
    delay(1000);
}
