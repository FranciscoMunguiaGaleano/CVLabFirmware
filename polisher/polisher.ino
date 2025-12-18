#include <WiFiS3.h>
#include <Arduino_LED_Matrix.h>
#include <Servo.h>
#include "arduino_secrets.h"   // contains SECRET_SSID and SECRET_PASS  //

// ==== WiFi credentials ==== //
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
// Fixed network settings
IPAddress localIp(192, 168, 0, 151);     // desired static IP
IPAddress dns(192, 168, 0, 1);           // usually same as gateway
IPAddress gateway(192, 168, 0, 1);       // router
IPAddress subnet(255, 255, 255, 0);      // subnet mask


WiFiServer server(5000);
ArduinoLEDMatrix matrix;

Servo servo1;
Servo servo2;

// attach your servos to correct pins
unsigned long lastMotorBTime = 0; // add this globally
const int servo1Pin = 3;
int speedPolisher = 255;
int frecuencyDroper = 85;

// Motor A en puente H (pines 5 = PWM, 6 = dir)
const int motorA_pwm = 5;
const int motorA_dir = 6;

// Motor B en puente H (pines 10 = PWM, 11 = dir)
const int motorB_pwm = 10;
const int motorB_dir = 11;

bool motorB_state = false;
unsigned long lastToggleB = 0;
int motorB_interval = 300;   // tiempo ON/OFF en ms (ajustas la "frecuencia")
bool motorB_enabled = false; // si el motor está "activo"

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
  pinMode(motorA_pwm, OUTPUT);
  pinMode(motorA_dir, OUTPUT);
  pinMode(motorB_pwm, OUTPUT);
  pinMode(motorB_dir, OUTPUT);
  servo1.attach(servo1Pin, 500, 2600);
  servo1.write(179);

  WiFi.config(localIp, dns, gateway, subnet);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    matrix.renderBitmap(frame_err, 8, 12);
  }

  
  server.begin();
  matrix.renderBitmap(frame_ok, 8, 12);
  
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    while (client.connected()) {
      
      if (client.available()) {
        
        String cmd = client.readStringUntil('\n');
        cmd.trim();

        if (cmd == "1") {
          //servo1.write(179);
          moveServoSmooth(servo1, servo1.read(), 179, 10);
          matrix.renderBitmap(frame_1, 8, 12);
          delay(1000);
          //servo1.write(180);
          //delay(1000);
        }
        else if (cmd == "2") {
          //servo1.write(8);
          moveServoSmooth(servo1, servo1.read(), 8, 10);
          matrix.renderBitmap(frame_2, 8, 12);
          delay(1000);
        }
        else if (cmd == "3") {
          matrix.renderBitmap(frame_3, 8, 12);
          motorA_on(speedPolisher);
        }
        else if (cmd == "4") {
          matrix.renderBitmap(frame_4, 8, 12);
          motorA_off();
        }
        else if (cmd == "5") {
          matrix.renderBitmap(frame_ok, 8, 12);
          motorB_on(frecuencyDroper);
        }
        else if (cmd == "6") {
          matrix.renderBitmap(frame_ok, 8, 12);
          motorB_off();
        }
        else if (cmd == "7") {
          set_speed(1); //set to slow
        }
        else if (cmd == "8") {
          set_speed(2); //set to medium
        }
        else if (cmd == "9") {
          set_speed(3); //set to fast
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

void motorA_on(int velocidad) {
  digitalWrite(motorA_dir, LOW);          // dirección fija
  analogWrite(motorA_pwm, velocidad);      // 0–255
}

void motorA_off() {
  digitalWrite(motorA_dir, LOW);           // opcional
  analogWrite(motorA_pwm, 0);              // sin PWM = apagado
}

// Activa el parpadeo
void motorB_on(int velocidad) {
  digitalWrite(motorB_dir, LOW);          // dirección fija
  analogWrite(motorB_pwm, velocidad);      // 0–255
}

void motorB_off() {
  digitalWrite(motorB_dir, LOW);           // opcional
  analogWrite(motorB_pwm, 0);              // sin PWM = apagado
}

void set_speed(int speed) {
  if (speed == 1){
     speedPolisher = 255;
     frecuencyDroper = 85; 
  }
    if (speed == 2)
  {
     speedPolisher = 100;
     frecuencyDroper = 92; 
  }
    if (speed == 3)
  {
     speedPolisher = 40;
     frecuencyDroper = 100; 
  }
}


