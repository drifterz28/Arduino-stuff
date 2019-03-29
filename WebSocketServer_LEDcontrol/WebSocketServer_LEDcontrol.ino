
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h>
#include <EEPROM.h>

#include <DNSServer.h>
#include <WiFiManager.h>

char htmlTitle[] = "Living Room";
char ApName[] = "ESP_couch";
char hostName[] = "couch";
char ApPass[] = "0123456789";

#define PIN 0
Adafruit_NeoPixel strip = Adafruit_NeoPixel(300, PIN, NEO_GRB + NEO_KHZ800);

#define USE_SERIAL Serial
ESP8266WiFiMulti WiFiMulti;

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

//int toggleSwitch = 3;

int holdState = 0;
int buttonState = 0;
uint8_t LEDdelay = 10;
int red = 000;
int green = 000;
int blue = 000;
int lastRed = 150;
int lastGreen = 150;
int lastBlue = 150;

uint8_t MAC_array[6];
char MAC_char[18];
//flag for saving data
bool shouldSaveConfig = false;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  int pixelCount = strip.numPixels();
  for (uint16_t i = 0; i < pixelCount; i++) {
    strip.setPixelColor(i, c);
    if(pixelCount - i > i) {
      strip.setPixelColor(pixelCount - i, c);
    }
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
        webSocket.sendTXT(num, String(red) + "," + String(green) + "," + String(blue));
      }
      break;
    case WStype_TEXT:
      USE_SERIAL.printf("[%u] get Text: %s\n", num, payload);
      if (payload[0] == '#') {
        // we get RGB data
        // decode rgb data
        uint32_t rgb = (uint32_t) strtol((const char *) &payload[1], NULL, 16);
        red = ((rgb >> 16) & 0xFF);
        green = ((rgb >> 8) & 0xFF);
        blue = ((rgb >> 0) & 0xFF);
        if(red > 0 || green > 0 || blue > 0) {
          lastRed = red;
          lastGreen = green;
          lastBlue = blue;
        }
        colorWipe(strip.Color(red, green, blue), LEDdelay);
        webSocket.broadcastTXT(String(red) + "," + String(green) + "," + String(blue));
      }
      break;
  }
}

void turnOff() {
  red = 000;
  green = 000;
  blue = 000;
  webSocket.broadcastTXT(String(red) + "," + String(green) + "," + String(blue));
  colorWipe(strip.Color(red, green, blue), LEDdelay);
}

void turnOn() {
  red = lastRed;
  green = lastGreen;
  blue = lastBlue;
  webSocket.broadcastTXT(String(red) + "," + String(green) + "," + String(blue));
  colorWipe(strip.Color(red, green, blue), LEDdelay);
}

void setup() {
  USE_SERIAL.begin(115200/*, SERIAL_8N1, SERIAL_TX_ONLY*/);
  USE_SERIAL.println("Start setup");
  
  delay(4000);
  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.autoConnect(ApName, ApPass);
  
  Serial.println("WiFi connected");
  
  Serial.print("Get MAC address: ");
  Serial.println(MAC_char);
  
  USE_SERIAL.print("IP address: ");
  USE_SERIAL.println(WiFi.localIP()); // just so you know
  
  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  lastRed = EEPROM.read(1);
  lastGreen = EEPROM.read(2);
  lastBlue = EEPROM.read(3);
  
  if (MDNS.begin(hostName)) {
    USE_SERIAL.println("MDNS responder started");
  }

  // handle index
  server.on("/", []() {
    // send index.html
    server.send(200, "text/html", "<!DOCTYPE html><html><head><meta charset=\"utf-8\"><title>" + String(htmlTitle) + "</title><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><link rel=\"stylesheet\" href=\"https://drifterz28.github.io/Arduino-stuff/assets/rgb-led.css\"/><script src=\"https://unpkg.com/react@15.3.1/dist/react.js\"></script><script src=\"https://unpkg.com/react-dom@15.3.1/dist/react-dom.js\"></script><script src=\"https://unpkg.com/babel-core@5.8.38/browser.min.js\"></script></head><body><div class=\"container\"></div><script type=\"text/babel\" src=\"https://drifterz28.github.io/Arduino-stuff/assets/rgb-led.js\"></script></body></html>");
  });

  server.on("/on", []() {
    turnOn();
    server.send(200, "text/plain", "On");
  });
  
  server.on("/off", []() {
    turnOff();
    server.send(200, "text/plain", "Off");
  });
  
  server.begin();
  strip.begin();
  strip.show();
  // Add service to MDNS
  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);
//  pinMode(toggleSwitch, INPUT_PULLUP);
}

void loop() {
  unsigned long currentMillis = millis();
  webSocket.loop();
  server.handleClient();
//  int newButtonState = digitalRead(toggleSwitch);
//  if(buttonState != newButtonState) {
//    buttonState = newButtonState;
//    if(red == 000 && green == 000 && blue == 000) {
//      turnOn();
//    } else {
//      turnOff();
//    }
//  }
  if ((WiFiMulti.run() != WL_CONNECTED)) {
    WiFi.begin();
  }
  if(currentMillis > 6.048e+8 && red == 000 && green == 000 && blue == 000) {
//    EEPROM.write(1, lastRed);
//    EEPROM.write(2, lastGreen);
//    EEPROM.write(3, lastBlue);
    ESP.restart();
  }
  delay(1);
}
