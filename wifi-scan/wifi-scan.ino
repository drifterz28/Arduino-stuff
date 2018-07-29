#include <Button-state.h>
#include "ESP8266WiFi.h"
#include <SPI.h>
//#include <Wire.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(4);

int button = 3; //3
int networkCount = 0;
int page = 0;
int perPage = 4;
float maxPages;
Button Btn(button, true, true, 20);

void setup()   {
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  // initialize with the I2C addr 0x3C (for the 128x32)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
}

void refreshListing() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  Serial.println("Scanning...");
  networkCount = WiFi.scanNetworks();
  if (networkCount == 0) {
    Serial.println("no networks found");
    display.println("no networks found");
  } else {
    maxPages = networkCount / perPage;
    Serial.print("Networks found: ");
    Serial.println(networkCount);
    display.print("Networks found: ");
    display.println(networkCount);
  }
  display.display();
  delay(3000);
}

void loop() {
//  Btn.read();
//  if (Btn.pressedFor(1000)) {
////    refreshListing();
//    Serial.println("long");
//  }
//  if (Btn.wasPressed()) {
//    Serial.println("pressed");
//  }
//  delay(100);
  refreshListing();
  display.clearDisplay();
  display.setTextColor(WHITE, BLACK);
  display.setCursor(0, 0);

  for (int i = 0; i < networkCount; ++i) {
    if (i % 4 == 0) {
      Serial.println("Set zero");
      display.clearDisplay();
      display.setCursor(0, 0);
    }
    // Print SSID and RSSI for each network found
    Serial.print(WiFi.SSID(i));
    display.print(WiFi.SSID(i));
    Serial.print(" ");
    display.print(" ");
    Serial.println(WiFi.RSSI(i));
    display.println(WiFi.RSSI(i));
    display.display();
    delay(1000);
  }
  delay(2000);
}

