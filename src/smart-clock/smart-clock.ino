#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>

#include "analog_clock.h"
#include "weather.h"
#include "global.h"
#include "eeprom.h"

GlobalData global;

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

    if (global.current_time - last_time >= M_THIRTY_SECONDS) {
        last_time = global.current_time;

        DSERIAL.printf("Checking connection to %s...\n", g_ssid.c_str());

        if (WiFi.status() != WL_CONNECTED) {
            DSERIAL.printf("Error. Trying to reconnect to %s...\n", g_ssid.c_str());
            WiFi.disconnect();
            WiFi.reconnect();
        } else {
            DSERIAL.println("Connection is good");
        }
    }
}

static void check_serial() {
    static unsigned long last_time = 0;

    if (millis() - last_time >= M_TEN_SECONDS) {
        last_time = millis();

        if (Serial.available()) {
            char input[256];
            serial_read(input);

            Serial.printf("Got input: %s\n", input);

            if (strcmp(input, "start") == 0) {
                char ssid[256];
                char password[256];

                while (true) {
                    Serial.println("Please type SSID");
                    serial_read(ssid);
                    Serial.printf("SSID: %s\n", ssid);

                    Serial.println("Please type password (check to see if there is anybody around)");
                    serial_read(password);
                    Serial.printf("Password: %s\n", password);

                    Serial.println("Please confirm ('yes' to save / 'abort' to abort / anything else to retry)");
                    char confirm[256];
                    serial_read(confirm);
                    if (strcmp(confirm, "yes") == 0) {
                        Serial.println("Saving");
                        break;
                    } else if (strcmp(confirm, "abort") == 0) {
                        Serial.println("Aborting");
                        return;
                    } else {
                        Serial.println("Retrying");
                    }
                }

                // Save in EEPROM
                Serial.println("Writing to EEPROM...");
                eeprom::write(ssid, password);
                Serial.println("Done");

                // Restart WiFi
                Serial.println("Restarting WiFi...");
                WiFi.disconnect();
                WiFi.begin(ssid, password);
                Serial.println("Done");

                g_ssid = ssid;
                g_password = password;
            }
        }
    }
}

static void get_ssid_and_password(String& ssid_out, String& password_out) {
    Serial.println("Please send SSID and password (or else...)");

    char ssid[256];
    char password[256];

    while (true) {
        Serial.println("Please type SSID");
        while (true) {
            if (Serial.available() > 0) {
                serial_read(ssid);
                Serial.printf("SSID: %s\n", ssid);
                break;
            }
        }

        Serial.println("Please type password (check to see if there is anybody around)");
        while (true) {
            if (Serial.available() > 0) {
                serial_read(password);
                Serial.printf("Password: %s\n", password);
                break;
            }
        }

        Serial.println("Please confirm ('yes' to save / anything else to retry)");
        char confirm[256];
        while (true) {
            if (Serial.available() > 0) {
                serial_read(confirm);
                Serial.printf("Answer: %s\n", confirm);
                break;
            }
        }

        if (strcmp(confirm, "yes") == 0) {
            Serial.println("Saving");
            break;
        } else {
            Serial.println("Retrying");
        }
    }

    // Save in EEPROM
    Serial.println("Writing to EEPROM...");
    eeprom::write(ssid, password);
    Serial.println("Done");

    ssid_out = ssid;
    password_out = password;
}

static void get_input() {
    global.button[1] = global.button[0];
    global.button[0] = digitalRead(BUTTON);
}

void setup() {
    Serial.begin(9600);
    EEPROM.begin(512);

    pinMode(BUTTON, INPUT);

    global.tft.initR(INITR_BLACKTAB);
    global.tft.setRotation(3);  // TODO maybe set this to 1 when putting everything together
    global.tft.fillScreen(ST77XX_BLACK);
    DSERIAL.println("Initialized display");

    Serial.println("Reading EEPROM...");
    eeprom::read(g_ssid, g_password);

    if (g_ssid == "" || g_password == "") {
        get_ssid_and_password(g_ssid, g_password);
    }

    WiFi.begin(g_ssid.c_str(), g_password.c_str());
    Serial.printf("Connecting to %s...\n", g_ssid.c_str());

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        DSERIAL.print(".");
        check_serial();
    }
    DSERIAL.print("\n");

    Serial.println("Connection established!");
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
    check_serial();
}
