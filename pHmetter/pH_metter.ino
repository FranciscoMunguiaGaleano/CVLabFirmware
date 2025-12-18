#include <WiFiS3.h>
#include <Arduino_LED_Matrix.h>
#include "arduino_secrets.h"   // contains SECRET_SSID and SECRET_PASS  //

// ==== WiFi credentials ==== //
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

ArduinoLEDMatrix matrix;

IPAddress localIp(192, 168, 0, 155);
IPAddress dns(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiServer server(5000);

// === pH Sensor Configuration ===
const int phSensorPin = A0;  // analog pin for pH sensor

// === LED Matrix Frames ===
byte frame_ok[8][12] = {
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,1,1,0,0,1,0,0,1,0,0 },
  { 0,1,0,0,1,0,1,0,1,0,0,0 },
  { 0,1,0,0,1,0,1,1,0,0,0,0 },
  { 0,1,0,0,1,0,1,1,0,0,0,0 },
  { 0,1,0,0,1,0,1,0,1,0,0,0 },
  { 0,0,1,1,0,0,1,0,0,1,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 }
};

byte frame_err[8][12] = {
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,1,1,1,0,0,1,1,1,0,0,0 },
  { 0,1,0,0,0,0,1,0,0,1,0,0 },
  { 0,1,1,1,0,0,1,1,0,0,0,0 },
  { 0,1,0,0,0,0,1,0,1,0,0,0 },
  { 0,1,1,1,0,0,1,1,1,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 }
};

void setup() {
  Serial.begin(115200);
  pinMode(phSensorPin, INPUT);
  matrix.begin();

  // Connect to WiFi
  WiFi.config(localIp, dns, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  unsigned long startAttempt = millis();
  const unsigned long timeout = 15000; // 15 seconds timeout

  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < timeout) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    matrix.renderBitmap(frame_ok, 8, 12);  // show OK on connection
  } else {
    Serial.println("\nWiFi Connection Failed!");
    matrix.renderBitmap(frame_err, 8, 12); // show ERR on failure
  }

  server.begin();
  Serial.println("TCP server started on port 5000");
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("Client connected");
    while (client.connected()) {
      if (client.available()) {
        String cmd = client.readStringUntil('\n');
        cmd.trim();

        if (cmd.equals("1")) {
          int sensorValue = analogRead(phSensorPin);
          client.println(sensorValue);
          Serial.print("Sent ADC value: ");
          Serial.println(sensorValue);
        } 
        else {
          client.println("Unknown command");
        }
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}
