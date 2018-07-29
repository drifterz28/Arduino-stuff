#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
ESP8266WebServer server(80);

#define IR_LED 2

IRsend irsend(IR_LED);
const char* ssid = "CenturyLink1356";
const char* password = "5038038883";

void handleRoot() {
  irsend.sendNEC(0x61A0F00F, 32);
  server.send(200, "text/plain", "Done!");
}

void setup() {
  WiFi.begin(ssid, password);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  server.on("/", handleRoot);

  server.begin();
  irsend.begin();
}

void loop() {
  server.handleClient();
}
