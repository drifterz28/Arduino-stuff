#include <SPI.h>
#include <ESP8266WiFi.h> 
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <Adafruit_SSD1306.h>
#include <Button-state.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

char ApName[] = "ESP_octo-print";
char hostName[] = "octo-print";
char ApPass[] = "0123456789";
char apiKey[] = "912398C133F7476DA4D32D9B7C27B08F";
#define PIN 2
int button = 3; //3
uint8_t MAC_array[6];
char MAC_char[18];
//flag for saving data
bool shouldSaveConfig = false;

Adafruit_SSD1306 display(4); // slc 2 sda 0
Button Btn(button, true, true, 20);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, PIN, NEO_GRB + NEO_KHZ800);

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup() {
  Serial.begin(115200/*, SERIAL_8N1, SERIAL_TX_ONLY*/);
  strip.begin();
  strip.show();

  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.autoConnect(ApName, ApPass);
  
  Serial.println("WiFi connected");
  
  Serial.print("Get MAC address: ");
  Serial.println(MAC_char);
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // just so you know
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  
}
// strip.setPixelColor(1, r1, g1, b1);
// strip.show();

void refreshListing() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
//  Serial.println("Scanning...");
//  if (networkCount == 0) {
//    Serial.println("no networks found");
//    display.println("no networks found");
//  } else {
//    maxPages = networkCount / perPage;
//    Serial.print("Networks found: ");
//    Serial.println(networkCount);
//    display.print("Networks found: ");
//    display.println(networkCount);
//  }
  display.display();
  delay(3000);
}

void loop() {
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin("http://192.168.0.124/api/job");
    http.addHeader("X-Api-Key", String(apiKey), true);
    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if(httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if(httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println(payload);
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    delay(100000);
}
