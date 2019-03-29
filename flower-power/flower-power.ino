#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel strip2 = Adafruit_NeoPixel(4, 2, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, 4, NEO_GRB + NEO_KHZ800);

uint8_t LEDdelay = 10;

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  int pixelCount = strip.numPixels();
  for (uint16_t i = 0; i < pixelCount; i++) {
    strip.setPixelColor(i, c);
    if(pixelCount - i > i) {
      strip.setPixelColor(pixelCount - i, c);
    }
    strip.show();
    delay(wait);
  }
}

void colorWipe2(uint32_t c, uint8_t wait) {
  int pixelCount = strip2.numPixels();
  for (uint16_t i = 0; i < pixelCount; i++) {
    strip2.setPixelColor(i, c);
    if(pixelCount - i > i) {
      strip2.setPixelColor(pixelCount - i, c);
    }
    strip2.show();
    delay(wait);
  }
}

void setup() {
  // put your setup code here, to run once:
  strip.begin();
  strip2.begin();
  colorWipe2(strip2.Color(200, 0, 0), LEDdelay);
  colorWipe(strip.Color(0, 200, 0), LEDdelay);
  strip.show();
  strip2.show();
}

void loop() {
  // put your main code here, to run repeatedly:

}
