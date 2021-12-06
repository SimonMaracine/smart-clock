#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include "global.h"

namespace http_request {
    String get(const char* url) {
        String payload = "";

        if (WiFi.status() == WL_CONNECTED) {
            DSERIAL.printf("GET-ing from %s...\n", url);
            WiFiClient client;
            HTTPClient http;
            http.begin(client, url);

            int response_code = http.GET();

            if (response_code > 0) {
                DSERIAL.printf("HTTP response code: %d\n", response_code);

                if (response_code == HTTP_CODE_OK || response_code == HTTP_CODE_MOVED_PERMANENTLY) {
                    payload = http.getString();
                } else {
                    DSERIAL.println("Something went wrong...");
                }
            } else {
                DSERIAL.printf("Error code: %d\n", response_code);
            }

            http.end();
        } else {
            DSERIAL.println("WiFi disconnected; cannot make HTTP request");
        }

        return payload;
    }
}
