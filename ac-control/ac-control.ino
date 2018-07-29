#include <Arduino.h>
#include <CheapStepper.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <OneButton.h>
#include <Math.h>
#include "DHT.h"

#define DHTTYPE DHT22   // DHT 22 sensor
#define DHTPIN 2     // sensor pin connected to
DHT dht(DHTPIN, DHTTYPE);

Adafruit_SSD1306 display(4);
CheapStepper stepper(8,9,10,11);
float setTemp = 0;

void setup() {
  Serial.begin(9600);
  dht.begin();
  stepper.set4076StepMode();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("");
  display.println("  AC Control");
  display.display();
  delay(2000);
  display.clearDisplay();
  stopRight.attachClick(stop);
  stopLeft.attachClick(stop);
}

void loop() {
  float temp = dht.readTemperature(true);
}
