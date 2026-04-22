#include <WiFiS3.h>
#include <Arduino_LED_Matrix.h>
#include <Servo.h>
#include "arduino_secrets.h"   // contains SECRET_SSID and SECRET_PASS

// ==== WiFi credentials ==== //
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

// Fixed network settings
IPAddress localIp(192, 168, 0, 159);
IPAddress dns(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiServer server(5000);
ArduinoLEDMatrix matrix;

// ==== SINGLE SERVO ==== //
Servo servo1;
const int servoPin = 3;

// ==== SIMPLE FRAMES ==== //
byte frame_ok[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0 },
  { 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0 },
  { 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0 },
  { 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0 },
  { 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0 },
  { 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

byte frame_err[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0 },
  { 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0 },
  { 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0 },
  { 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0 },
  { 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0 },
  { 0, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

byte frame_1[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

byte frame_2[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
byte frame_3[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};
byte frame_4[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

void setup() {
  
  matrix.begin();

  // Attach servo to pin 3
  servo1.attach(servoPin);
  servo1.write(180);  // start at home

  // WiFi setup
  WiFi.config(localIp, dns, gateway, subnet);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    matrix.renderBitmap(frame_err, 8, 12);
  }

  matrix.renderBitmap(frame_ok, 8, 12);
  server.begin();
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    while (client.connected()) {
      if (client.available()) {
        String cmd = client.readStringUntil('\n');
        cmd.trim();

        // ==== COMMANDS ==== //
        if (cmd == "1") {
          // Go to HOME (0°)
          matrix.renderBitmap(frame_1, 8, 12);
          servo1.write(160);
          delay(500); 
          servo1.write(180);
        }
        else if (cmd == "2") {
          // Go to 70°
          matrix.renderBitmap(frame_2, 8, 12);
          servo1.write(150);
          delay(500);
          servo1.write(180);
        }
        else {
          matrix.renderBitmap(frame_err, 8, 12);
        }
      }
    }
    client.stop();
  }
}

// ==== SMOOTH MOVEMENT ==== //
void moveServoSmooth(Servo &servo, int from, int to, int stepDelay) {
  if (from < to) {
    for (int pos = from; pos <= to; pos++) {
      servo.write(pos);
      delay(stepDelay);
    }
  } else {
    for (int pos = from; pos >= to; pos--) {
      servo.write(pos);
      delay(stepDelay);
    }
  }
}

