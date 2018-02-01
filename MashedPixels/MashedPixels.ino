#include "FastLED.h"
#include <Wire.h>

//#define USE_MATHIEUMODE
#define USE_EIFFELMODE
#define USE_ANTSMODE
#define USE_CALIBRATEMODE
#define USE_TRADITIONALMODE
#define USE_REDWAVEMODE
#define USE_ICECRYSTALSMODE

//Customize here
#define DATA_PIN 2
#define BUTTON_PIN 3
#define NUM_LEDS 140
#define DELAY 1 // in minutes

#define USE_RESET_AS_BUTTON
#define SERIAL_DEBUG
unsigned long TimerA;
#ifdef SERIAL_DEBUG
#define MODE_EXPORT_MNAME
#endif

//#ifdef USE_MATHIEUMODE
//#include "MathieuMode.h"
//#endif
#ifdef USE_EIFFELMODE
#include "EiffelMode.h"
#endif
#ifdef USE_ANTSMODE
#include "AntsMode.h"
#endif
#ifdef USE_CALIBRATEMODE
#include "CalibrateMode.h"
#endif
#ifdef USE_TRADITIONALMODE
#include "TraditionalMode.h"
#endif
#ifdef USE_REDWAVEMODE
#include "RedWaveMode.h"
#endif
#ifdef USE_ICECRYSTALSMODE
#include "IceCrystalsMode.h"
#endif

#define DATA_SIZE 300
byte *data;
//char files[13][13];

//Menu modes
enum mode_display {
//  MODE_DISPLAY_MATHIEU,
  MODE_DISPLAY_EIFFEL,
  MODE_DISPLAY_ANTS,
  MODE_DISPLAY_CALIBRATE,
  MODE_DISPLAY_TRADITIONAL,
  MODE_DISPLAY_REDWAVE,
  MODE_DISPLAY_ICECRYSTALS,
};

CRGB leds[NUM_LEDS + 1];

byte MAX_MODE = 5;

//MathieuMode mat = MathieuMode(NUM_LEDS);
EiffelMode ef = EiffelMode(false, NUM_LEDS);
AntsMode ant = AntsMode(true, NUM_LEDS);
CalibrateMode cal = CalibrateMode(NUM_LEDS);
TraditionalMode trad = TraditionalMode(NUM_LEDS);
RedWaveMode rw = RedWaveMode(NUM_LEDS);
IceCrystalsMode ice = IceCrystalsMode(NUM_LEDS);

int mode = 0;
boolean file_initialized = false;

void setup() {
  delay(500);
  Serial.begin(9600);
  data = (byte *)malloc(DATA_SIZE);
  Serial.println("init FastLED");
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
//  Serial.println("Init mat");
//  mat.init();
  Serial.println("Init ef");
  ef.init();
  Serial.println("Init ant");
  ant.init();
  Serial.println("Init cal");
  cal.init();
  Serial.println("Init trad");
  trad.init();
  Serial.println("Init rw");
  rw.init();
  Serial.println("Init ice");
  ice.init();

  Serial.print(MAX_MODE);
  Serial.println(" Count");
  Serial.println("done");
}

void loop() {
  
  if (millis() - TimerA >= DELAY * 60000) {
    TimerA = millis();
    Serial.print(MAX_MODE);
    Serial.print(" ");
    Serial.print(mode);
    Serial.print("/");
    mode = mode + 1;
    if(mode >= MAX_MODE) {
      mode = 0;  
    }
    Serial.println(mode);
    switch (mode) {
//      case 0:
//        cal.init();
//        break;
      case 0:
        trad.init();
        break;
      case 1:
        ant.init();
        break;
      case 2:
        ef.init();
        break;
      case 3:
        rw.init();
        break;
      case 4:
        ice.init();
        break;
    }

    delay(1000);
  }
  
  switch (mode) {
//    case 0:
//      cal.loop(leds);
//      FastLED.show();
//      break;
    case 0:
      trad.loop(leds);
      FastLED.show();
      break;
    case 1:
      ant.loop(leds);
      FastLED.show();
      break;
    case 2:
      ef.loop(leds);
      FastLED.show();
      break;
    case 3:
      rw.loop(leds);
      FastLED.show();
      break;
    case 4:
      ice.loop(leds);
      FastLED.show();
      break;
  }
}




