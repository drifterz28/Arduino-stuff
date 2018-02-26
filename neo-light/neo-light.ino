#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN 1

Adafruit_NeoPixel strip = Adafruit_NeoPixel(31, PIN, NEO_GRB + NEO_KHZ800);

int addr = 0;
int value = 0;
int maxOptions = 4;
void setup() {
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  value = EEPROM.read(addr);
  value++;
  if(value > maxOptions) {
    value = 1;
  }
  delay(500);
  EEPROM.write(addr, value);
}
void loop() {
  switch(value) {
    case 0:
      colorWipe(strip.Color(0, 0, 0), 50);
      break;
    case 1:
      colorWipe(strip.Color(0, 255, 0), 50);
      break; 
    case 2:
      colorWipe(strip.Color(0, 0, 255), 50);
      break;
    case 3:
      colorWipe(strip.Color(255, 0, 0), 50);
      break;
    case 4:
      rainbow(100);
      break;
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

