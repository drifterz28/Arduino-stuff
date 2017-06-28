#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define LEFTPIN 5
#define RIGHTPIN 6
#define LEDS 4
Adafruit_NeoPixel stripRight = Adafruit_NeoPixel(LEDS, RIGHTPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripLeft = Adafruit_NeoPixel(LEDS, LEFTPIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // put your setup code here, to run once:
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code


  stripRight.begin();
  stripLeft.begin();
  stripRight.show();
  stripLeft.show();
}
int blinkCount = 0;
void loop() {
  int what = 1;
  switch(what) {
    case 1:
      if(blinkCount < 3) {
        colorWipeBoth(0, 0, 0, 50);
        delay(100);
        colorWipeBoth(255, 0, 0, 50);
        delay(100);
        colorWipeBoth(0, 0, 0, 50);
        blinkCount++;
      }
      colorWipeBoth(255, 0, 0, 50);
      break;
    case 2:
      // right
      colorWipeRight(stripRight.Color(255, 110, 0), 100);
      break;
    case 3:
      // left
      colorWipeLeft(stripLeft.Color(255, 110, 0), 100);
      break;
  }

}

void colorWipeBoth(uint32_t r, uint32_t g, uint32_t b, uint8_t wait) {
  stripLeft.Color(r, g, b);
  stripRight.Color(r, g, b);
  for(uint16_t i=0; i<stripRight.numPixels(); i++) {
    stripRight.setPixelColor(i, r, g, b);
    stripLeft.setPixelColor(i, r, g, b);
    stripRight.show();
    stripLeft.show();
    delay(wait);
  }
}

void colorWipeRight(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<stripRight.numPixels(); i++) {
    stripRight.setPixelColor(i, c);
    stripRight.show();
    delay(wait);
  }
}

void colorWipeLeft(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<stripLeft.numPixels(); i++) {
    stripLeft.setPixelColor(i, c);
    stripLeft.show();
    delay(wait);
  }
}
