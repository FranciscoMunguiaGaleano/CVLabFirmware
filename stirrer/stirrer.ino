#include <WiFiS3.h>
#include <Arduino_LED_Matrix.h>
#include <Servo.h>
#include "arduino_secrets.h"   // contains SECRET_SSID and SECRET_PASS

// ==== WiFi credentials ==== //
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
// FANS //
int ON = 12;
int FAN_1 = 11;
int FAN_2 = 10;
int FAN_3 = 9;
int FAN_4 = 6;
int FAN_5 = 5;
int FAN_6 = 3;
bool FANS_ON = false;
bool FANS_STATE = false;
bool WASHERS_ON = false;
bool WASHERS_STATE = false;

int TIME_ON = 170;
int TIME_OFF = 150;
int TIME_ON_OFF = TIME_ON;
// Timea measurement
unsigned long previousTime = 0;
unsigned long previousTimewashers = 0;

// Fixed network settings
IPAddress localIp(192, 168, 0, 160);
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
  pinMode(ON, OUTPUT);
  pinMode(FAN_1, OUTPUT);
  pinMode(FAN_2, OUTPUT);
  pinMode(FAN_3, OUTPUT);
  pinMode(FAN_4, OUTPUT);
  pinMode(FAN_5, OUTPUT);
  pinMode(FAN_6, OUTPUT);
  matrix.begin();

  //WiFi setup
  WiFi.config(localIp, dns, gateway, subnet);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    matrix.renderBitmap(frame_err, 8, 12);
  }
  matrix.renderBitmap(frame_ok, 8, 12);
  for (int i=0;i<=20;i++)
  {
    digitalWrite(ON,1); //blinking shows the wifi connection was succesful
    delay(100);
    digitalWrite(ON,0); 
    delay(100);
  }
  server.begin();
}

void loop() 
{
///////////////////
  //restart_fans();
  if (FANS_ON == true)
      {
        unsigned long currentTime = millis();
        if (currentTime - previousTime >= TIME_ON_OFF) 
        {
            previousTime = currentTime; // Satart5in time
            toggle_fans();
        }
      }
  if (WASHERS_ON == true)
      {
        unsigned long currentTimewashers = millis();
        if (currentTimewashers - previousTimewashers >= TIME_ON_OFF) 
        {
            previousTimewashers = currentTimewashers; // Satart5in time
            toggle_washers();
        }
      }
  WiFiClient client = server.available();
  
  if (client) {
    while (client.connected()) 
    { 
       
      ///////
      if (client.available()) {
        String cmd = client.readStringUntil('\n');
        cmd.trim();
        // ==== COMMANDS ==== //
        if (cmd == "1") {   // 1 sample fans on
          matrix.renderBitmap(frame_1, 8, 12);
          digitalWrite(ON, 1);
          FANS_ON = true;
        }
        else if (cmd == "2") {  // 2 sample fans off
          matrix.renderBitmap(frame_2, 8, 12);
          digitalWrite(ON, 0);
          restart_fans();
        }
        if (cmd == "3") {   // 3 electrode washers on
          matrix.renderBitmap(frame_1, 8, 12);
          digitalWrite(ON, 1);
          WASHERS_ON = true;
        }
        else if (cmd == "4") {  // 4 ele ctrode washers off
          matrix.renderBitmap(frame_2, 8, 12);
          digitalWrite(ON, 0);
          restart_washers();
        }
        else {
          matrix.renderBitmap(frame_err, 8, 12);
        }
      }
    //logic here
    }
    client.stop();
  }
}

// ==== SAMPLE FANS CONTROL ==== //
void FansON() 
{
  digitalWrite(FAN_1,1);
  digitalWrite(FAN_2,1);
  digitalWrite(FAN_3,1);
} 
void FansOFF() 
{
    digitalWrite(FAN_1,0);
    digitalWrite(FAN_2,0);
    digitalWrite(FAN_3,0);
}

void WashersON()
{
  digitalWrite(FAN_4,1);
  digitalWrite(FAN_5,1);
  digitalWrite(FAN_6,1);
}
void WashersOFF()
{
  digitalWrite(FAN_4,0);
  digitalWrite(FAN_5,0);
  digitalWrite(FAN_6,0);

}
void toggle_fans()
  {
    if (FANS_STATE == false)
      {
        FansON();
        FANS_STATE=true;
        TIME_ON_OFF=TIME_ON;
      }
    else
      {
        FansOFF();
        FANS_STATE=false;
        TIME_ON_OFF=TIME_OFF;
      }
  }

void toggle_washers()
  {
    if (WASHERS_STATE == false)
      {
        WashersON();
        WASHERS_STATE=true;
        TIME_ON_OFF=TIME_ON;
      }
    else
      {
        WashersOFF();
        WASHERS_STATE=false;
        TIME_ON_OFF=TIME_OFF;
      }
  }

void restart_fans()
  {
    FansOFF();
    FANS_ON= false;
    FANS_STATE=false;
    TIME_ON_OFF=TIME_ON;
  }

void restart_washers()
  {
    WashersOFF();
    WASHERS_ON= false;
    WASHERS_STATE=false;
    TIME_ON_OFF=TIME_ON;
  }


