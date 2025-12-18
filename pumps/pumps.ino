#include <WiFiS3.h>
#include <Arduino_LED_Matrix.h>
#include "arduino_secrets.h"   // contains SECRET_SSID and SECRET_PASS  //

// ==== WiFi credentials ==== //
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

// Fixed network settings
IPAddress localIp(192, 168, 0, 152);
IPAddress dns(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiServer server(5000);
ArduinoLEDMatrix matrix;

// Shaker H-bridge
const int shaker_en  = 3;
const int shaker_dir = 4;

// Lift H-bridge (moved off 0,1)
const int lift_en  = A2;
const int lift_dir = A3;

// Pumps
const int pump_1_en  = 5;
const int pump_1_dir = 6;
const int pump_2_en  = 7;
const int pump_2_dir = 8;
const int pump_3_en  = 9;
const int pump_3_dir = 10;
const int pump_4_en  = A4;
const int pump_4_dir = A5;

// Emergency pin
const int emergency_pin = 13;

// Lift motion control
bool liftMoving = false;
bool liftUpDir  = false;
unsigned long liftStartTime = 0;
const unsigned long liftDuration = 12000; // 12 seconds

// Bitmaps (use 12x8)
byte frame_ok[8][12] = {
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,1,1,0,0,1,0,0,1,0,0},
  {0,1,0,0,1,0,1,0,1,0,0,0},
  {0,1,0,0,1,0,1,1,0,0,0,0},
  {0,1,0,0,1,0,1,1,0,0,0,0},
  {0,1,0,0,1,0,1,0,1,0,0,0},
  {0,0,1,1,0,0,1,0,0,1,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0}
};

byte frame_err[8][12] = {
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,1,1,1,1,0,1,1,1,1,0,0},
  {0,1,0,0,0,0,1,0,0,1,0,0},
  {0,1,0,0,0,0,1,1,1,1,0,0},
  {0,1,1,1,0,0,1,1,0,0,0,0},
  {0,1,0,0,0,0,1,0,1,0,0,0},
  {0,1,1,1,1,0,1,0,0,1,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0}
};

// simple icons
byte frame_1[8][12] = {
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,1,0,0,0,0,0,0},
  {0,0,0,0,1,1,0,0,0,0,0,0},
  {0,0,0,0,0,1,0,0,0,0,0,0},
  {0,0,0,0,0,1,0,0,0,0,0,0},
  {0,0,0,0,0,1,0,0,0,0,0,0},
  {0,0,0,0,1,1,1,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0}
};

byte frame_2[8][12] = {
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,1,1,0,0,0,0,0},
  {0,0,0,0,1,0,0,1,0,0,0,0},
  {0,0,0,0,0,0,0,1,0,0,0,0},
  {0,0,0,0,0,1,1,0,0,0,0,0},
  {0,0,0,0,1,0,0,0,0,0,0,0},
  {0,0,0,0,1,1,1,1,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0}
};

byte frame_3[8][12] = {
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,1,1,1,0,0,0,0,0},
  {0,0,0,0,0,0,0,1,0,0,0,0},
  {0,0,0,0,0,1,1,0,0,0,0,0},
  {0,0,0,0,0,0,0,1,0,0,0,0},
  {0,0,0,0,1,0,0,1,0,0,0,0},
  {0,0,0,0,0,1,1,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0}
};

byte frame_4[8][12] = {
  {0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,1,0,0,1,0,0,0,0},
  {0,0,0,0,1,0,0,1,0,0,0,0},
  {0,0,0,0,0,1,1,1,0,0,0,0},
  {0,0,0,0,0,0,0,1,0,0,0,0},
  {0,0,0,0,0,0,0,1,0,0,0,0},
  {0,0,0,0,0,0,0,1,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0}
};

void setup() {
  matrix.begin();

  // Pin setup
  int pins[] = { shaker_en, shaker_dir, lift_en, lift_dir,
                 pump_1_en, pump_1_dir, pump_2_en, pump_2_dir,
                 pump_3_en, pump_3_dir, pump_4_en, pump_4_dir };
  for (int i = 0; i < 12; i++) pinMode(pins[i], OUTPUT);

  pinMode(emergency_pin, INPUT_PULLUP);

  // WiFi connect with timeout
  WiFi.config(localIp, dns, gateway, subnet);
  WiFi.begin(ssid, password);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    matrix.renderBitmap(frame_err, 12, 8);
  }

  if (WiFi.status() == WL_CONNECTED) {
    server.begin();
    matrix.renderBitmap(frame_ok, 12, 8);
  } else {
    matrix.renderBitmap(frame_err, 12, 8);
  }
}

void loop() {
  WiFiClient client = server.available();

  // Handle active lift motion
  if (liftMoving && millis() - liftStartTime > liftDuration) {
    liftMoving = false;
    digitalWrite(lift_en, LOW);
    digitalWrite(lift_dir, LOW);
    matrix.renderBitmap(frame_ok, 12, 8);
  }

  if (client) {
    while (client.connected()) {
      if (client.available()) {
        String cmd = client.readStringUntil('\n');
        cmd.trim();

        if (cmd == "1") { 
          digitalWrite(lift_en, HIGH); 
          digitalWrite(lift_dir, LOW);
          //lift_start(true); 
          matrix.renderBitmap(frame_1, 12, 8); 
        }
        else if (cmd == "2") { 
          //lift_start(false); 
          digitalWrite(lift_en, LOW); 
          digitalWrite(lift_dir, HIGH);
          matrix.renderBitmap(frame_2, 12, 8); 
          }
        else if (cmd == "3") { shaker_on(); matrix.renderBitmap(frame_1, 12, 8); }
        else if (cmd == "4") { shaker_off(); matrix.renderBitmap(frame_2, 12, 8); }
        else if (cmd == "5") { setAllPumps(true); matrix.renderBitmap(frame_1, 12, 8); }
        else if (cmd == "6") { setAllPumps(false); matrix.renderBitmap(frame_2, 12, 8); }
        else if (cmd == "7") {
          if (digitalRead(emergency_pin) == HIGH)
            matrix.renderBitmap(frame_3, 12, 8);
          else
            matrix.renderBitmap(frame_4, 12, 8);
        } 
        else if (cmd == "8") {
        digitalWrite(lift_en, LOW); 
        digitalWrite(lift_dir, LOW);
        }
        else {
          matrix.renderBitmap(frame_err, 12, 8);
        }
      }
    }
    client.stop();
  }
}

// ==== Actuator functions ====
void shaker_on()  { digitalWrite(shaker_en, HIGH); digitalWrite(shaker_dir, LOW); }
void shaker_off() { digitalWrite(shaker_en, LOW);  digitalWrite(shaker_dir, LOW); }

void lift_start(bool up) {
  digitalWrite(lift_dir, up ? LOW : HIGH);
  digitalWrite(lift_en, HIGH);
  liftStartTime = millis();
  liftMoving = true;
  liftUpDir = up;
}

void setPump(int en, int dir, bool on) {
  digitalWrite(en, on ? HIGH : LOW);
  digitalWrite(dir, LOW);
}

void setAllPumps(bool on) {
  setPump(pump_1_en, pump_1_dir, on);
  setPump(pump_2_en, pump_2_dir, on);
  setPump(pump_3_en, pump_3_dir, on);
  setPump(pump_4_en, pump_4_dir, on);
}




