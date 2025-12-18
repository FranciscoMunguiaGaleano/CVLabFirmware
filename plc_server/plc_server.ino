#include <WiFi.h>
#include "OptaBlue.h"
#include "arduino_secrets.h"   // contains SECRET_SSID and SECRET_PASS  //
using namespace Opta;
// ==== WiFi credentials ==== //
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
// ==== Network (static IP optional) ==== //
IPAddress localIp(192, 168, 0, 158);
IPAddress dns(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
WiFiServer server(80);
// ==== Relay configuration ==== //
const int onboardRelays = 4;  // D1–D4
const int relayPins[onboardRelays] = {RELAY1, RELAY2, RELAY3, RELAY4};
// ==== Setup ==== //
void setup() 
{
  // ==== Setup onboard relays ==== //
  delay(5000);
  for (int i = 0; i < onboardRelays; i++) 
  {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);
  }
  // ==== Connect to Wi-Fi ===== //
  WiFi.config(localIp, dns, gateway, subnet);
  WiFi.begin(ssid, pass);
  // ==== WiFi ==== //
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    //Serial.print(".");
  }
  // ===== Start web server ===== //
  server.begin();
  // ===== Initialize Opta expansion controller =====
  OptaController.begin();
  //Serial.println("OptaController initialized.");
}

void loop() 
{
  WiFiClient client = server.available();
  if (!client) return;
  //Serial.println("New client connected");
  String request = "";
  unsigned long timeout = millis();
  // ---- Read HTTP request fully ----
  while (client.connected() && millis() - timeout < 2000) 
  {
    while (client.available()) 
    {
      char c = client.read();
      request += c;
      // Detect end of HTTP headers
      if (c == '\n' && request.endsWith("\r\n\r\n")) 
      {
        goto end_of_headers;
      }
      timeout = millis(); // reset timeout whenever new data arrives
    }
  }

end_of_headers:
  // ==== Handle relay commands ====
  handleRelayCommands(request);
  delay(1);
  client.stop();
}
// ==== Handle onboard + expansion relays ==== //
void handleRelayCommands(String req) 
{
  req.toLowerCase();  // simplify matching
  bool actionTaken = false;
  // --- Onboard relays ---
  for (int i = 0; i < onboardRelays; i++) 
  {
    String onCmd = "/main"+String(i+1);
    String offCmd = "/mainoff"+String(i+1);
    if (req.indexOf(onCmd) >= 0) 
    {
      digitalWrite(relayPins[i], HIGH);
      actionTaken = true;
    }
    if (req.indexOf(offCmd) >= 0) 
    {
      digitalWrite(relayPins[i], LOW);
      actionTaken = true;
    }
  }

  // --- Opta expansion relays (6 outputs on first expansion module) ---
  DigitalMechExpansion mechExp = OptaController.getExpansion(0);
  mechExp.updateDigitalInputs();  // refresh input states

  for (int i = 0; i < 9; i++) 
  {
    String onCmd = "/ext"+String(i+1);
    String offCmd = "/offext"+String(i+1);
    if (req.indexOf(onCmd) >= 0) 
    {
      mechExp.digitalWrite(i, HIGH);
      mechExp.updateDigitalOutputs();
      actionTaken = true;
    }
    if (req.indexOf(offCmd) >= 0) {
      mechExp.digitalWrite(i, LOW);
      mechExp.updateDigitalOutputs();
      actionTaken = true;
    }
  }
}