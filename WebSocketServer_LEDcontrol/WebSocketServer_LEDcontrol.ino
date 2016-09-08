
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h>

#define PIN 0
Adafruit_NeoPixel strip = Adafruit_NeoPixel(95, PIN, NEO_GRB + NEO_KHZ800);

#define USE_SERIAL Serial
ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer server = ESP8266WebServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {
  switch (type) {
    case WStype_DISCONNECTED:
      USE_SERIAL.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        USE_SERIAL.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        // send message to client
        webSocket.sendTXT(num, "Connected");
      }
      break;
    case WStype_TEXT:
      USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
      if (payload[0] == '#') {
        // we get RGB data
        // decode rgb data
        uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);
        int red = ((rgb >> 16) & 0xFF);
        int green = ((rgb >> 8) & 0xFF);
        int blue = ((rgb >> 0) & 0xFF);
        colorWipe(strip.Color(red, green, blue), 0);
      }
      break;
  }
}

void setup() {
  USE_SERIAL.begin(115200);

  //USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  strip.begin();
  strip.show();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFiMulti.addAP("-2.4", "");
  WiFi.softAP("esp8266-ajkdafhsdjlk", "nothingyoucanfindout", 1, 1);
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }
  USE_SERIAL.print("IP address: ");
  USE_SERIAL.println(WiFi.localIP()); // just so you know
  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  if (MDNS.begin("esp8266")) {
    USE_SERIAL.println("MDNS responder started");
  }

  // handle index
  server.on("/", []() {
    // send index.html
    server.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>RGB Cololrs</title><meta name=\"viewport\" content=\"user-scalable=no, initial-scale=1, maximum-scale=1, minimum-scale=1, width=device-width\"><link rel=\"stylesheet\" href=\"https://drifterz28.github.io/Arduino-stuff/assets/rgb-led.css\"/></head><body><div class=\"container\"><h1>LED Control</h1><button class=\"off-btn\">Off</button><div class=\"slide\">Red: <input id=\"r\" class=\"slider\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" value=\"0\"/></div><div class=\"slide\">Green:<input id=\"g\" class=\"slider\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" value=\"0\"/></div><div class=\"slide\">Blue:<input id=\"b\" class=\"slider\" type=\"range\" min=\"0\" max=\"255\" step=\"1\" value=\"0\"/></div><div class=\"color\">Color Display</div></div><script src=\"https://drifterz28.github.io/Arduino-stuff/assets/rgb-led.js\"></script></body></html>");
  });

  server.begin();

  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
}

void loop() {
  webSocket.loop();
  server.handleClient();
}
