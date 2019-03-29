#include <Arduino.h>
#include <CheapStepper.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <OneButton.h>
#include <Math.h>
#include "DHT.h"

#define DHTTYPE DHT22   // DHT 22 sensor
#define DHTPIN 7     // sensor pin connected to
DHT dht(DHTPIN, DHTTYPE);

Adafruit_SSD1306 display(4);
CheapStepper stepper(8,9,10,11);
OneButton upBtn(4, true);
OneButton downBtn(5, true);
float setTemp = 70;

unsigned long updateInterval = 1000;
unsigned long previousMillis = 0;

enum setting {
  off = 0,
  fanHigh = 30,
  acHigh = 120,
  acLow = 250,
  fanLow = 300
};

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
  int speed = round(100 / 24 * 100);
  stepper.setRpm(speed);
  upBtn.attachClick(incrimentUp);
  downBtn.attachClick(incrimentDown);
}

void incrimentUp() {
  Serial.println("up");
  setTemp++;
}
void incrimentDown() {
  setTemp--;
}

void displayTemps(float temp) {
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("setting   |   current");
  display.println(" " + String(setTemp) + "    |     " + String(temp));
  display.display();
}

void loop() {
  downBtn.tick();
  upBtn.tick();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= updateInterval) {
    float temp = dht.readTemperature(true);
    displayTemps(temp);
    if(temp > setTemp + 5) {
      runStepper(acHigh);
    }
    if(temp >= setTemp) {
      runStepper(acLow);
    }
    if(temp < setTemp && temp >= setTemp - 5) {
      runStepper(fanHigh);
    }
    if(temp < setTemp - 5) {
      runStepper(off);
    }
  }
}

void runStepper(int deg) {
  stepper.run();
  int stepsLeft = stepper.getStepsLeft();
  if (stepsLeft == 0) {
    stepper.newMoveDegrees(true, deg);
  }
}
