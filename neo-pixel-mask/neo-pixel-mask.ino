#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

int pin = 1;
int sensorPin = 3;
int pixels = 12;
int sensorValue = 0;
int rgbValue = 0;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(pixels, pin, NEO_GRB + NEO_KHZ800);

void setup() {
  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif
  // End of trinket special code
  //Serial.begin(9600);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);
  //Serial.println(sensorValue);
  if(sensorValue < 255) {
    colorWipe(strip.Color(sensorValue, 0, 0));
  } else if(sensorValue > 255 && sensorValue < 510) {
    rgbValue = sensorValue - 255;
    colorWipe(strip.Color(0, rgbValue, 0));
  } else if (sensorValue > 510 && sensorValue < 765) {
    rgbValue = sensorValue - 510;
    colorWipe(strip.Color(0, 0, rgbValue));
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(20);
  }
}

