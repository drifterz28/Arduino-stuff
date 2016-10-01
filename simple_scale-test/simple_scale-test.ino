#include <HX711.h>

const byte hx711_data_pin1 = 4;
const byte hx711_clock_pin1 = 5;

//const byte hx711_data_pin2 = 5;
//const byte hx711_clock_pin2 = 4;

HX711 scale_one(hx711_data_pin1, hx711_clock_pin1);
//HX711 scale_two(hx711_data_pin2, hx711_clock_pin2);

// ~81808 seems to be 0lbs - subtract value
int zeroValue = 0;

void setup() {
  Serial.begin(9600);
  scale_one.setScale();
  scale_one.tare();
}

void loop() {
  // built with 3.3v logic
  // ~83886 is the reading when the sensor is not hooked up
  // ~81808 zero value
  // ~20 container for 2oz
  // ~8 seems to be 2oz
  // ~4 is about 1oz
  // ~17 4oz
  // ~81880 seems to be 16.9oz water battle
  // ~82296 seems to be 6lbs
  // ~82564 1 gallon of water in jug... 8.36lbs
  Serial.println(scale_one.get_value(5));
  delay(500);
}
