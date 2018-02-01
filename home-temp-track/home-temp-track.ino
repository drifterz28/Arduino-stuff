#include "DHT.h"
#include <FS.h>
#include "HTTPSRedirect.h"
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <Button.h>
#include <ArduinoJson.h>

// for stack analytics
extern "C" {
#include <cont.h>
  extern cont_t g_cont;
}

char minute_delay[3] = "1";
char location_name[] = "Some Room";
char sheet_id[] = "15qgEBCoIykGMVdtLXLkE1Ocjmze-OV-w9h5clfmgvmY";

// DHT sensor setup
const int DhtPin = 2;     // sensor pin connected to
const uint8_t DhtType = DHT22;   // DHT 22 sensor
const int resetBtn = 0;
const int httpsPort = 443;
const int maxConnect = 20;
const bool isFahrenheit = true; // Read temperature as Fahrenheit (isFahrenheit = true)
const char host[] = "script.google.com";
const char ApName[] = "Temp_tracker";
const char fingerprint[] = "2C:94:20:14:D6:16:AA:CB:B4:DB:66:11:5F:99:0E:EA:B8:D3:3E:8B";
bool shouldSaveConfig = false;
unsigned long TimerA;
String payload_base =  "{\"command\": \"appendRow\", \
                    \"sheet_name\": \"Sheet1\", \
                    \"values\": ";
String payload = "";

// Write to Google Spreadsheet
String url = String("/macros/s/") + sheet_id + "/exec?value=Hello";
// Fetch Google Calendar events for 1 week ahead
String url2 = String("/macros/s/") + sheet_id + "/exec?cal";
// Read from Google Spreadsheet
String url3 = String("/macros/s/") + sheet_id + "/exec?read";

unsigned int free_heap_before = 0;
unsigned int free_stack_before = 0;
HTTPSRedirect* client = nullptr;
Button reset(resetBtn);
DHT dht(DhtPin, DhtType);
ESP8266WiFiMulti WiFiMulti;

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Setup");
  dht.begin();
  reset.begin();
  wifiConfigSetup();
  sheetSetup();
  delay(500);
}

void loop() {
//  if (reset.pressed()) {
//    WiFiManager wifiManager;
//    wifiManager.resetSettings();
//  }

  int pullDelay = minute_delay[0] + 0;
  Serial.print("pullDelay ");
  Serial.println(pullDelay);

  if (millis() - TimerA >= pullDelay * 60000) {
    static int errorCount = 0;
    static int connectCount = 0;
    static bool flag = false;

    if (!flag) {
      free_heap_before = ESP.getFreeHeap();
      free_stack_before = cont_get_free_stack(&g_cont);
      client = new HTTPSRedirect(httpsPort);
      flag = true;
      client->setPrintResponseBody(true);
      client->setContentTypeHeader("application/json");
    }

    if (client != nullptr) {
      if (!client->connected()) {
        client->connect(host, httpsPort);
        payload = payload_base + "\"" +
                  free_heap_before + "," + free_stack_before + "\"}";
        client->POST(url2, host, payload, false);
      }
    } else {
      Serial.println("Error creating client object!");
      errorCount = 5;
    }

    if (connectCount > maxConnect) {
      //error_count = 5;
      connectCount = 0;
      flag = false;
      delete client;
      return;
    }

    Serial.println("GET Data from cell 'A1':");
    if (client->GET(url3, host)) {
      ++connectCount;
    } else {
      ++errorCount;
      Serial.println("Error-count while connecting: ");
      Serial.println(errorCount);
    }

    Serial.println("POST append memory data to spreadsheet:");
    payload = payload_base + "\"" +
              ESP.getFreeHeap() + "," + cont_get_free_stack(&g_cont) + "\"}";

    if (errorCount > 3) {
      Serial.println("Halting processor...");
      delete client;
      client = nullptr;
      Serial.printf("Final free heap: %u\n", ESP.getFreeHeap());
      Serial.printf("Final unmodified stack   = %4d\n", cont_get_free_stack(&g_cont));
      Serial.flush();
      ESP.deepSleep(0);
    }
  }
}
