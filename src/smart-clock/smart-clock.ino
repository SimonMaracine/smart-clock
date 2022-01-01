#include <limits.h>

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <ESP8266WiFi.h>

#include "analog_clock.h"
#include "weather.h"
#include "global.h"

GlobalData global;
//static const char* ssid = "NCC1701-D";  // "WWJD";
//static const char* password = "perfectwind065";  // "4wwJdtoday?";
static String ssid, password;

static void check_connection() {
  static unsigned long last_time = 0;

  if (global.current_time - last_time >= M_THIRTY_SECONDS) {
    last_time = global.current_time;

    DSERIAL.printf("Checking connection to %s...\n", ssid.c_str());

    if (WiFi.status() != WL_CONNECTED) {
      DSERIAL.printf("Error. Trying to reconnect to %s...\n", ssid.c_str());
      WiFi.disconnect();
      WiFi.reconnect();
    } else {
      DSERIAL.println("Connection is good");
    }
  }
}

static void check_serial() {
  static unsigned long last_time = 0;

  if (global.current_time - last_time >= M_TEN_SECONDS) {
    last_time = global.current_time;

    Serial.println("TEN SECONDS CHECK");

    if (Serial.available()) {
      String input = Serial.readString();

      Serial.println(input);
      input.trim();

      if (input == "start") {
        Serial.setTimeout(M_ONE_MINUTE);

        while (true) {
          Serial.println("Please send SSID");
          String ssid = Serial.readString();
          if (ssid == "") {
            return;
          }
          Serial.printf("SSID: %s\n", ssid.c_str());

          Serial.println("Please send password (check to see if there is nobody around)");
          String password = Serial.readString();
          if (password == "") {
            return;
          }
          Serial.printf("Password: %s\n", password.c_str());

          Serial.println("Please confirm ('yes' to save / 'abort' to abort / anything else to retry)");
          String confirm = Serial.readString();
          if (confirm == "yes") {
            Serial.println("Saving");
            break;
          } else if (confirm == "abort") {
            Serial.println("Aborting");
            return;
          } else {
            Serial.println("Retrying");
          }
        }

        Serial.setTimeout(1);

        // Save in EEPROM
        Serial.println("Writing to EEPROM...");
        Serial.println("Done");

        // Restart WiFi
        Serial.println("Restarting WiFi...");
        Serial.println("Done");
      }
    }
  }
}

static void get_ssid_and_password(String& ssid_in, String& password_in) {
    Serial.println("Please send ssid and password (or else...)");

//    Serial.setTimeout(1000 * 3600);
    while (true) {
        Serial.println("Please send SSID");
        String ssid;
        readString(ssid);

        Serial.print("SSID: ");
        Serial.println(ssid);

        Serial.println("Please send password (check to see if there is nobody around)");
        String password;
        readString(password);
        Serial.printf("Password: %s\n", password.c_str());

        Serial.println("Please confirm ('yes' to save / anything else to retry)");

        String confirm;
        readString(confirm);
        if (confirm == "yes") {
            Serial.println("Saving");
            break;
        } else {
            Serial.println("Retrying");
        }
    }

    Serial.setTimeout(1);
    // Save in EEPROM
    Serial.println("Writing to EEPROM...");
    Serial.println("Done");
    
    ssid_in = ssid;
    password_in = password;
}

static void readString(String& string) {
    int i = -1;
    while (true)
    {
        if (Serial.available())
        {
            do{
                i++;
                string += Serial.read();
            }while(string[i] != NULL);

            break;
        }
    }
    
}

static void get_input() {
  global.button[1] = global.button[0];
  global.button[0] = digitalRead(BUTTON);
}

static void read_eeprom(String& ssid, String& password) {
  ssid = "";
  password = "";
}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(1);

  pinMode(BUTTON, INPUT);

  global.tft.initR(INITR_BLACKTAB);
  global.tft.setRotation(3);  // TODO maybe set this to 1 when putting everything together
  global.tft.fillScreen(ST77XX_BLACK);
  DSERIAL.println("Initialized display");

  read_eeprom(ssid, password);

  if (ssid == "" && password == "") {
    get_ssid_and_password(ssid, password);
  }

  WiFi.begin(ssid.c_str(), password.c_str());
  DSERIAL.printf("Connecting to %s...\n", ssid.c_str());

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
  check_serial();
}
