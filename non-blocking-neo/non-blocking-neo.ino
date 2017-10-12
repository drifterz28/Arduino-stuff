// StrandTest from AdaFruit implemented as a state machine
// pattern change by push button
// By Mike Cook Jan 2016

#define PINforControl   0 // pin connected to the small NeoPixels strip
#define NUMPIXELS1      17 // number of LEDs on strip

#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS1, PINforControl, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(NUMPIXELS1, PINforControl, NEO_GRB + NEO_KHZ800);

unsigned long patternInterval = 20 ; // time between steps in the pattern
unsigned long lastUpdate = 0 ; // for millis() when last update occoured
unsigned long intervals [] = { 20, 20, 50, 100, 100, 100 } ; // speed for each pattern

long pattern;
void setup() {
  strip.begin(); // This initializes the NeoPixel library.
  wipe(); // wipes the LED buffers
  pattern = random(0, 6);
}

void loop() {
  if (millis() - lastUpdate > patternInterval) {
    updatePattern(pattern);
  }
}

void  updatePattern(long pat) { // call the pattern currently being created
  switch (pat) {
    case 0:
      rainbow();
      break;
    case 1:
      rainbowCycle();
      break;
    case 2:
      theaterChaseRainbow();
      break;
    case 3:
      colorWipe(strip.Color(255, 0, 0)); // red
      colorWipe(strip2.Color(255, 0, 0)); // red
      break;
    case 4:
      colorWipe(strip.Color(0, 255, 0)); // green
      colorWipe(strip2.Color(0, 255, 0)); // green
      break;
    case 5:
      colorWipe(strip.Color(0, 0, 255)); // blue
      colorWipe(strip2.Color(0, 0, 255)); // blue
      break;
  }
}

void rainbow() { // modified from Adafruit example to make it a state machine
  static uint16_t j = 0;
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel((i + j) & 255));
    strip2.setPixelColor(i, Wheel((i + j) & 255));
  }
  strip.show();
  strip2.show();
  j++;
  if (j >= 256) j = 0;
  lastUpdate = millis(); // time for next change to the display

}
void rainbowCycle() { // modified from Adafruit example to make it a state machine
  static uint16_t j = 0;
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    strip2.setPixelColor(i, Wheel2(((i * 256 / strip2.numPixels()) + j) & 255));
  }
  strip.show();
  strip2.show();
  j++;
  if (j >= 256 * 5) j = 0;
  lastUpdate = millis(); // time for next change to the display
}

void theaterChaseRainbow() { // modified from Adafruit example to make it a state machine
  static int j = 0, q = 0;
  static boolean on = true;
  if (on) {
    for (int i = 0; i < strip.numPixels(); i = i + 3) {
      strip.setPixelColor(i + q, Wheel( (i + j) % 255)); //turn every third pixel on
      strip2.setPixelColor(i + q, Wheel2( (i + j) % 255)); //turn every third pixel on
    }
  }
  else {
    for (int i = 0; i < strip.numPixels(); i = i + 3) {
      strip.setPixelColor(i + q, 0);      //turn every third pixel off
      strip2.setPixelColor(i + q, 0);      //turn every third pixel off
    }
  }
  on = !on; // toggel pixelse on or off for next time
  strip.show(); // display
  strip2.show();
  q++; // update the q variable
  if (q >= 3 ) { // if it overflows reset it and update the J variable
    q = 0;
    j++;
    if (j >= 256) j = 0;
  }
  lastUpdate = millis(); // time for next change to the display
}

void colorWipe(uint32_t c) { // modified from Adafruit example to make it a state machine
  static int i = 0;
  strip.setPixelColor(i, c);
  strip2.setPixelColor(i, c);
  strip.show();
  strip2.show();
  i++;
  if (i >= strip.numPixels()) {
    i = 0;
    wipe(); // blank out strip
  }
  lastUpdate = millis(); // time for next change to the display
}


void wipe() { // clear all LEDs
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
    strip2.setPixelColor(i, strip.Color(0, 0, 0));
  }
}

uint32_t Wheel2(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip2.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip2.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip2.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
