#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "fauxmoESP.h"
 
#define SERIAL_BAUDRATE  115200
#define WIFI_SSID "SSID"
#define WIFI_PASS "PASSWORD"
fauxmoESP fauxmo;


// -----------------------------------------------------------------------------
// Wifi
// -----------------------------------------------------------------------------
 
void wifiSetup() {
 
    // Set WIFI module to STA mode
    WiFi.mode(WIFI_STA);
 
    // Connect
    Serial.printf("[WIFI] Connecting to %s ", WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
 
    // Wait
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(100);
    }
    Serial.println();
 
    // Connected!
    Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
}

void setup() {
 
    // Init serial port and clean garbage
    Serial.begin(SERIAL_BAUDRATE);
    Serial.println();
    Serial.println();
    pinMode(2, OUTPUT);
    digitalWrite(2, 1);
    // Wifi
    wifiSetup();
 
    // Fauxmo
    fauxmo.addDevice("Tree Lights");
    fauxmo.addDevice("Christmas Tree Lights");
    fauxmo.onMessage([](const char * device_name, bool state) {
        Serial.printf("[MAIN] %s state: %s\n", device_name, state ? "ON" : "OFF");
        Serial.println(state);
         if(!state) {
          digitalWrite(2, 1);
         } else {
          digitalWrite(2, 0);
         }
    });
 
}
 
void loop() {}
