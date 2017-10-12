#include <Wire.h> 
#include <LiquidCrystal_I2C_esp.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <DNSServer.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <Button.h>
#include <Adafruit_NeoPixel.h>

String ApName = "Crypto-checker";
String ApPass = "0123456789";
String url = "http://www.drifterz28.com/crypto?currency=";
int pullDelay = 5; // in minutes
int nextBttn = 2;// Connect your button between pin and GND
int pin = 0;

// shit you don't need to change
int currency = 0;
Button button1(nextBttn);
ESP8266WiFiMulti WiFiMulti;
LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned long TimerA;
//flag for saving data
bool shouldSaveConfig = false;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(1, pin, NEO_GRB + NEO_KHZ800);

//callback notifying us of the need to save config
void saveConfigCallback() {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  lcd.print("Enter config mode");
  Serial.println(WiFi.softAPIP());
  delay(2000);
  lcd.print("SSID:");
  lcd.setCursor(0, 1);
  lcd.print(myWiFiManager->getConfigPortalSSID());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup() {
  // put your setup code here, to run once:
  // initialize the LCD
  lcd.begin(1, 3);  // sda=1, scl=3

  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.print("Starting");

  delay(4000);
  lcd.clear();
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();
  
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.autoConnect(ApName.c_str(), ApPass.c_str());
  lcd.setCursor(0, 0);
  lcd.print("Connected, yeey!");
  delay(2000);
  lcd.print("IP address:");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP()); // just so you know
  delay(2000);
  lcd.clear();
  if (MDNS.begin(ApName.c_str())) {
    lcd.setCursor(0, 0);
    lcd.print("MDNS responder");
    lcd.setCursor(0, 1);
    lcd.print("started");
    delay(2000);
    lcd.clear();
  }
  button1.begin();
  strip.begin();
  strip.show();
  colorWipe(strip.Color(0, 0, 255), 0);
}

void loop() {
  DynamicJsonBuffer jsonBuffer(200);
  if (millis() - TimerA >= pullDelay * 60000) {
    TimerA = millis();
    if((WiFiMulti.run() == WL_CONNECTED)) {
      HTTPClient http;
      http.begin(url + String(currency));
      
      int httpCode = http.GET();
      if(httpCode > 0) {
        if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          JsonObject& root = jsonBuffer.parseObject(payload);
          String price = root["price"].asString();
          int priceStart = 16 - price.length();
          String percentage = root["percentChange"].asString();
          int percentStart = 16 - percentage.length();
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print(root["name"].asString());
          lcd.setCursor(priceStart, 0);
          lcd.print(price);
          lcd.setCursor(0, 1);
          lcd.print(root["priceChange"].asString());
          lcd.setCursor(percentStart, 1);
          lcd.print(percentage);
          
          if(root["direction"] == "down") {
            colorWipe(strip.Color(255, 0, 0), 0);  
          } else if(root["direction"] == "up") {
            colorWipe(strip.Color(0, 255, 0), 0);
          } else {
            colorWipe(strip.Color(0, 0, 255), 0);  
          }
          currency++;
          if (currency > 2) {
            currency = 0;
          }
        }
      }
      http.end();
    }
  }
  if(button1.pressed()) {
    currency++;
    if (currency > 2) {
      currency = 0;
    }
    TimerA = TimerA + pullDelay * 60000;
  }
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}
