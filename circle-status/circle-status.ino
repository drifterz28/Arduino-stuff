#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <WiFiManager.h>

// change these values
int pullDelay = 1; // in minutes
String url = "http://iot-ecommsolution.rhcloud.com/circle"; //url to get
String ApName = "CircleLights";
String ApPass = "0123456789";
int ledCount = 10;

// leave these values
int pin = 0;
int red = 000;
int green = 000;
int blue = 000;
char json[200];
bool shouldSaveConfig = false;
StaticJsonBuffer<200> jsonBuffer;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(ledCount, pin, NEO_GRB + NEO_KHZ800);
ESP8266WiFiMulti WiFiMulti;

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

//callback notifying us of the need to save config
void saveConfigCallback () {
  shouldSaveConfig = true;
}

void setup() {
  Serial.begin(115200);
  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.autoConnect(ApName.c_str(), ApPass.c_str());
  Serial.println("start");
  strip.begin();
  strip.show();
}

void loop() {
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    Serial.println("Connected");
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0 && httpCode == HTTP_CODE_OK) {
      Serial.println(httpCode);
      String payload = http.getString();
      Serial.println(payload);
      payload.toCharArray(json, 200);
      JsonObject& root = jsonBuffer.parseObject(payload);
      red = root["r"].as<String>().toInt();
      blue = root["b"].as<String>().toInt();
      green = root["g"].as<String>().toInt();
      colorWipe(strip.Color(red, green, blue), 40);
    }
    http.end();
  } else {
    Serial.println("Reboot");
    ESP.restart();
  }
  delay(1000 * 60 * pullDelay);
}

