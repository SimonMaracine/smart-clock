#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>

#include "analog_clock.h"
#include "weather.h"
#include "more_weather.h"
#include "more_info.h"
#include "global.h"
#include "eeprom.h"

GlobalData g;

// WWJD
// 4wwJdtoday?
static String g_ssid, g_password;

static void serial_read(char* string_out) {
    unsigned int index = 0;

    while (true) {
        if (Serial.available()) {
            char character = Serial.read();

            if (character != '\n') {
                string_out[index] = character;
                index++;
            } else {
                string_out[index] = 0;
                break;
            }
        }
        delay(2);
    }
}

static void check_connection() {
    static unsigned long last_time = 0;

    if (g.current_time - last_time >= M_FOURTYFIVE_SECONDS) {
        last_time = g.current_time;

        Serial.printf("Checking connection to %s...\n", g_ssid.c_str());

        if (WiFi.status() != WL_CONNECTED) {
            Serial.printf("Error. Trying to reconnect to %s...\n", g_ssid.c_str());
            WiFi.disconnect();
            WiFi.reconnect();
        } else {
            Serial.println("Connection is good");
        }
    }
}

/*
    Return true to abort running other code after this function
*/
static bool read_serial_input_for_ssid_and_password(bool abort_option, char* ssid_out, char* password_out) {
    while (true) {
        Serial.println("Please type SSID");
        serial_read(ssid_out);
        Serial.printf("SSID: %s\n", ssid_out);
    
        Serial.println("Please type password (check to see if there is anybody around)");
        serial_read(password_out);
        Serial.printf("Password: %s\n", password_out);

        char confirm[128];

        if (abort_option) {
            Serial.println("Please confirm ('yes' to save / 'abort' to abort / anything else to retry)");    
            serial_read(confirm);
            Serial.printf("Answer: %s\n", confirm);
        
            if (strcmp(confirm, "yes") == 0) {
                Serial.println("Saving...");
                break;
            } else if (strcmp(confirm, "abort") == 0) {
                Serial.println("Aborting...");
                return true;
            } else {
                Serial.println("Retrying...");
            }
        } else {
            Serial.println("Please confirm ('yes' to save / anything else to retry)");
            serial_read(confirm);
            Serial.printf("Answer: %s\n", confirm);
            
            if (strcmp(confirm, "yes") == 0) {
                Serial.println("Saving...");
                break;
            } else {
                Serial.println("Retrying...");
            }
        }
    }

    return false;
}

/*
    Return true to abort running other code after this function
*/
static bool read_serial_input_for_dst(char* dst_out) {
    while (true) {
        Serial.println("Please type DST (either 1, -1 or 0)");
        serial_read(dst_out);
        Serial.printf("DST: %s\n", dst_out);

        char confirm[128];

        Serial.println("Please confirm ('yes' to save / 'abort' to abort / anything else to retry)");    
        serial_read(confirm);
        Serial.printf("Answer: %s\n", confirm);
    
        if (strcmp(confirm, "yes") == 0) {
            Serial.println("Saving...");
            break;
        } else if (strcmp(confirm, "abort") == 0) {
            Serial.println("Aborting...");
            return true;
        } else {
            Serial.println("Retrying...");
        }
    }

    return false;
}

static void check_serial() {
    static unsigned long last_time = 0;

    if (millis() - last_time >= M_TEN_SECONDS) {
        last_time = millis();

        if (Serial.available()) {
            char input[256];
            serial_read(input);

            Serial.printf("Got input: %s\n", input);

            if (strcmp(input, "wifi") == 0) {
                char ssid[256];
                char password[256];

                const bool to_abort = read_serial_input_for_ssid_and_password(true, ssid, password);
                if (to_abort) {
                    return;
                }

                // Save in EEPROM
                Serial.println("Writing to EEPROM...");
                eeprom::write(ssid, password, String(g.clock_data.dst));
                Serial.println("Done");

                // Restart WiFi
                Serial.println("Restarting WiFi...");
                WiFi.disconnect();
                WiFi.begin(ssid, password);
                Serial.println("Done");

                g_ssid = ssid;
                g_password = password;
            } else if (strcmp(input, "dst") == 0) {
                char dst[128];

                const bool to_abort = read_serial_input_for_dst(dst);
                if (to_abort) {
                    return;
                }

                // Save in EEPROM
                Serial.println("Writing to EEPROM...");
                eeprom::write(g_ssid, g_password, dst);
                Serial.println("Done");

                g.clock_data.dst = atoi(dst);
            }
        }
    }
}

static void force_get_ssid_and_password_from_serial() {
    Serial.println("Please send SSID and password (or else...)");

    char ssid[256];
    char password[256];

    const bool to_abort = read_serial_input_for_ssid_and_password(false, ssid, password);
    if (to_abort) {
        return;
    }

    // Save in EEPROM
    Serial.println("Writing to EEPROM...");
    eeprom::write(ssid, password, String(g.clock_data.dst));
    Serial.println("Done");

    g_ssid = ssid;
    g_password = password;
}

static void get_input() {
    g.button[1] = g.button[0];
    g.button[0] = digitalRead(BUTTON);
}

void setup() {
    Serial.begin(9600);
    EEPROM.begin(512);

    pinMode(BUTTON, INPUT);

    g.tft.initR(INITR_BLACKTAB);
    g.tft.setRotation(3);  // TODO maybe set this to 1 when putting everything together
    g.tft.fillScreen(ST77XX_BLACK);
    Serial.println("Initialized display");

    g.dht.begin();

    Serial.println("Reading EEPROM...");
    String dst;
    eeprom::read(g_ssid, g_password, dst);
    g.clock_data.dst = atoi(dst.c_str());

    if (g_ssid.isEmpty() || g_password.isEmpty()) {
        Serial.println("SSID or password is empty");
        force_get_ssid_and_password_from_serial();
    }

    WiFi.begin(g_ssid.c_str(), g_password.c_str());
    Serial.printf("Connecting to %s...\n", g_ssid.c_str());

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
        check_serial();
    }
    Serial.print("\n");

    Serial.println("Connection established!");
    Serial.printf("IP address: %s\n", WiFi.localIP().toString().c_str());

    change_screen(Screen::Clock);
    analog_clock::start_draw();
}

void loop() {
    g.current_time = millis();

    get_input();

    if (is_button_pressed(g.button)) {
        switch (g.current_screen) {
            case Screen::Clock:
                change_screen(Screen::Weather);
                weather::start_draw();
                break;
            case Screen::Weather:
                change_screen(Screen::MoreWeather);
                more_weather::start_draw();
                break;
            case Screen::MoreWeather:
                change_screen(Screen::MoreInfo);
                more_info::start_draw();
                break;
            case Screen::MoreInfo:
                change_screen(Screen::Clock);
                analog_clock::start_draw();
                break;
        }
    }

    analog_clock::update();
    weather::update();
    g.current_screen_func();
    check_connection();
    check_serial();
}
