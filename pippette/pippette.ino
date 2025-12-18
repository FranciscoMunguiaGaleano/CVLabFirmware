#include <WiFiS3.h>
#include <Arduino_LED_Matrix.h>
#include <Servo.h>
#include "arduino_secrets.h"   // contains SECRET_SSID and SECRET_PASS  //

// ==== WiFi credentials ==== //
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int servospeed = 100;

// Fixed network settings
IPAddress localIp(192, 168, 0, 150);     // desired static IP
IPAddress dns(192, 168, 0, 1);           // usually same as gateway
IPAddress gateway(192, 168, 0, 1);       // router
IPAddress subnet(255, 255, 255, 0);      // subnet mask


WiFiServer server(5000);
ArduinoLEDMatrix matrix;

Servo servo1;
Servo servo2;

// attach your servos to correct pins
const int servo1Pin = 9;
const int servo2Pin = 10;

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

  servo1.attach(servo1Pin);
  servo1.write(180);
  servo2.attach(servo2Pin);
  servo2.write(0);

  //WiFi.config(localIp, gateway, subnet, dns);
  WiFi.config(localIp, dns, gateway, subnet);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    matrix.renderBitmap(frame_err, 8, 12);
  }

  matrix.renderBitmap(frame_ok, 8, 12);
  server.begin();
  
}

void loop() {
  
  //delay(2000);
  //
  //delay(2000);
  WiFiClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        String cmd = client.readStringUntil('\n');
        cmd.trim();

        if (cmd == "1") { // Cargar pipeta.
          servo1.write(100);
          matrix.renderBitmap(frame_1, 8, 12);
          delay(1000);
          //servo1.write(180);
          //delay(1000);
        }
        else if (cmd == "2") { // Descargar pipeta
          servo1.write(70);
          matrix.renderBitmap(frame_2, 8, 12);
          delay(1000);
          servo1.write(180);
          delay(1000);
        }
        else if (cmd == "3") { // expulsar tip
          servo2.write(120);
          matrix.renderBitmap(frame_3, 8, 12);
          delay(1000);
          servo2.write(0);
          delay(1000);
        }
        else if (cmd == "4") { // regresar servos a cero
          //servo1.write(180);
          moveServoSmooth(servo1, servo1.read(), 180, servospeed);
          servo2.write(0);
          matrix.renderBitmap(frame_4, 8, 12);
          delay(1000);
        }
        else if (cmd == "5") {
          servospeed = 100;
        }
        else if (cmd == "6") {
          servospeed = 50;
        }
        else if (cmd == "7") {
          servospeed = 10;
        }
        else {
          matrix.renderBitmap(frame_err, 8, 12);
        }
      }
    }
    client.stop();
  }
}

void moveServoSmooth(Servo &servo, int from, int to, int stepDelay) {
  if (from < to) {
    for (int pos = from; pos <= to; pos++) {
      servo.write(pos);
      delay(stepDelay); // controla la velocidad
    }
  } else {
    for (int pos = from; pos >= to; pos--) {
      servo.write(pos);
      delay(stepDelay);
    }
  }
}

