#include <WiFiS3.h>
#include <Servo.h>
#include <Arduino_LED_Matrix.h>
#include "arduino_secrets.h"   // contains SECRET_SSID and SECRET_PASS  //

// ==== WiFi credentials ==== //
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

ArduinoLEDMatrix matrix;


IPAddress localIp(192, 168, 0, 153);
IPAddress dns(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

WiFiServer server(5000);

// === Encoder + Servo Configuration ===
const int encoderPins[8] = {2, 3, 4, 5, 6, 7, 8, 9};
const int limitSwitchPin = 13;
const int servoPin = 11;

Servo parallaxServo;

const float stepsPerTurn = 128.0;
const float turnsPerRevolution = 12.0;
const float degPerStep = 360.0 / (stepsPerTurn * turnsPerRevolution);

const int encoderMap[128] = {
  127,63,62,58,56,184,152,24,8,72,73,77,79,15,47,175,
  191,159,31,29,28,92,76,12,4,36,164,166,167,135,151,215,
  223,207,143,142,14,46,38,6,2,18,82,83,211,195,203,235,
  239,231,199,71,7,23,19,3,1,9,41,169,233,225,229,245,
  247,243,227,163,131,139,137,129,128,132,148,212,244,240,242,250,
  251,249,241,209,193,197,196,192,64,66,74,106,122,120,121,125,
  253,252,248,232,224,226,98,96,32,33,37,53,61,60,188,190,
  254,126,124,116,112,113,49,48,16,144,146,154,158,30,94,95
};

int zeroOffset = 0;
long currentTurn = 0;
int lastPosition = 0;

float Kp = 36;
float Kd = 3;
float lastError = 0;
unsigned long lastTime = 0;

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

// === SETUP ===
void setup() {
  matrix.begin();
  for (int i = 0; i < 8; i++) pinMode(encoderPins[i], INPUT_PULLUP);
  pinMode(limitSwitchPin, INPUT_PULLUP);

  parallaxServo.attach(servoPin);
  stopServo();

  // Initialize encoder reference
  int raw = readEncoder();
  int pos = getPositionIndex(raw);
  if (pos < 0) pos = 0;
  zeroOffset = pos;
  lastPosition = pos;

  // WiFi connection
  WiFi.config(localIp, dns, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  matrix.renderBitmap(frame_ok, 8, 12);
  server.begin();
}

// === MAIN LOOP ===
void loop() {
  WiFiClient client = server.available();
  if (client) {
    while (client.connected()) {
      if (client.available()) {
        String cmd = client.readStringUntil('\n');
        cmd.trim();

        if (cmd.equalsIgnoreCase("home")) {
          homeSystem();
        } 
        else if (cmd.startsWith("go ")) {
          float angle = cmd.substring(3).toFloat();
          goToAngle(angle);
        }
        else {
          // Unknown command
        }
      }
    }
    client.stop();
  }
}

// === ENCODER FUNCTIONS ===
void updateEncoder() {
  int raw = readEncoder();
  int pos = getPositionIndex(raw);
  if (pos < 0) return;
  int diff = pos - lastPosition;
  if (diff > 64) currentTurn--;
  if (diff < -64) currentTurn++;
  lastPosition = pos;
}

float getCurrentDegrees() {
  updateEncoder();
  long absoluteSteps = (currentTurn * 128L) + (lastPosition - zeroOffset);
  float degrees = fmod((absoluteSteps * degPerStep + 360.0), 360.0);
  return degrees;
}

int readEncoder() {
  int value = 0;
  for (int i = 0; i < 8; i++) {
    if (digitalRead(encoderPins[i]) == HIGH) value |= (1 << i);
  }
  return value;
}

int getPositionIndex(int reading) {
  for (int i = 0; i < 128; i++) if (encoderMap[i] == reading) return i;
  return -1;
}

// === HOMING ===
void homeSystem() {
  parallaxServo.writeMicroseconds(1600);
  while (readLimitSwitch() == HIGH) delay(30);
  stopServo();
  delay(300);

  parallaxServo.writeMicroseconds(1450);
  delay(500);
  stopServo();
  delay(300);

  parallaxServo.writeMicroseconds(1525);
  while (readLimitSwitch() == HIGH) delay(30);
  stopServo();
  resetPosition();
}

// === LIMIT SWITCH ===
bool readLimitSwitch() {
  if (digitalRead(limitSwitchPin) == LOW) {
    delay(20);
    if (digitalRead(limitSwitchPin) == LOW) return LOW;
  }
  return HIGH;
}

// === GO TO ANGLE ===
void goToAngle(float targetDeg) {
  unsigned long now = millis();
  lastTime = now;
  lastError = 0;

  while (true) {
    float current = getCurrentDegrees();
    float error = targetDeg - current;
    if (error > 180) error -= 360;
    if (error < -180) error += 360;

    unsigned long currentTime = millis();
    float dt = (currentTime - lastTime) / 1000.0;
    float derivative = (error - lastError) / dt;

    float control = Kp * error + Kd * derivative;
    if (control > 100) control = 100;
    if (control < -100) control = -100;

    if (fabs(error) < 0.2) {
      stopServo();
      break;
    }

    if (control > 0) parallaxServo.writeMicroseconds(1500 - abs(control));
    else if (control < 0) parallaxServo.writeMicroseconds(1500 + abs(control));
    else parallaxServo.writeMicroseconds(1500);

    lastError = error;
    lastTime = currentTime;
    delay(20);
  }
}

// === SERVO CONTROL ===
void stopServo() { parallaxServo.writeMicroseconds(1500); }

void resetPosition() {
  int raw = readEncoder();
  int pos = getPositionIndex(raw);
  if (pos >= 0) {
    zeroOffset = pos;
    currentTurn = 0;
  }
}
