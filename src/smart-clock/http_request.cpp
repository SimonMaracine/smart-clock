#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include "global.h"

namespace http_request {
    String get(const char* url) {
        String payload = "";

        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("GET-ing from %s...\n", url);
            WiFiClient client;
            HTTPClient http;
            http.begin(client, url);

            const int response_code = http.GET();

            if (response_code > 0) {
                Serial.printf("HTTP response code: %d\n", response_code);

                if (response_code == HTTP_CODE_OK || response_code == HTTP_CODE_MOVED_PERMANENTLY) {
                    payload = http.getString();
                } else {
                    Serial.println("Something went wrong...");
                }
            } else {
                Serial.printf("Error code: %d\n", response_code);
            }

            http.end();
        } else {
            Serial.println("WiFi disconnected; cannot make HTTP request");
        }

        return payload;
    }
}
