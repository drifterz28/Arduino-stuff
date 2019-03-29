#include <Adafruit_SSD1306.h>
#include <OneButton.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>

Adafruit_SSD1306 display(4);

#define REDPIN 11
#define GREENPIN 10
#define BLUEPIN 9

int r = 0;
int g = 0;
int b = 0;

int lastR = 0;
int lastG = 0;
int lastB = 0;
bool isOn = false;

OneButton colorChangeBtn(2, true);
OneButton doors(3, true);

int addr = 0;
int currentColor = 0;
int colorCount = 12;
int fadeSpeed = 50;
unsigned long previousMillis = 0;

char *colors[12][4] = {
  {"Off", 0, 0, 0},
  {"Blue", 0, 0, 255},
  {"Cyan", 0, 255, 255},
  {"Fuchsia", 255, 0, 255},
  {"Green", 0, 128, 0},
  {"Lime", 0, 255, 0},
  {"Maroon", 128, 0, 0},
  {"Navy", 0, 0, 128},
  {"Purple", 128, 0, 128},
  {"Red", 255, 0, 0},
  {"Teal", 0, 128, 128},
  {"White", 255, 255, 255}
};

void colorChnage() {
  currentColor++;
  if(currentColor > colorCount - 1) {
    currentColor = 0;
  }
  display.clearDisplay();
  display.setCursor(20,10);
  display.println(colors[currentColor][0]);
  display.display();
  EEPROM.write(addr, currentColor);
  r = colors[currentColor][1];
  g = colors[currentColor][2];
  b = colors[currentColor][3];
}

void changeBritness() {
  Serial.println("Change britness!!");
}

void toogleAll() {
  isOn = !isOn;
  Serial.print("on status: ");
  Serial.println(isOn);
}

void setup() {
  Serial.begin(9600);
  doors.attachLongPressStart(toogleAll);
  doors.attachLongPressStop(toogleAll);
  colorChangeBtn.attachDuringLongPress(changeBritness);
  colorChangeBtn.attachDoubleClick(toogleAll);
  colorChangeBtn.attachClick(colorChnage);
  currentColor = EEPROM.read(addr);
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(20,10);
  display.println(colors[currentColor][0]);
  display.display();
  r = colors[currentColor][1];
  g = colors[currentColor][2];
  b = colors[currentColor][3];
}

void loop() {
  colorChangeBtn.tick();
  doors.tick();
  
  unsigned long currentMillis = millis();
  int fadeR;
  int fadeG;
  int fadeB;
  if(isOn) {
    if(currentMillis - previousMillis >= fadeSpeed) {
      previousMillis = currentMillis;
      
      for (fadeR = 0; fadeR < r; fadeR++) {
        analogWrite(REDPIN, r);
      }
      for (fadeG = 0; fadeG < g; fadeG++) {
        analogWrite(GREENPIN, g);
      }
      for (fadeB = 0; fadeB < b; fadeB++) {
        analogWrite(BLUEPIN, b);
      }
    }
  } else {
    analogWrite(REDPIN, 0);
    analogWrite(GREENPIN, 0);
    analogWrite(BLUEPIN, 0);
  }
}
