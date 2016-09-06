#include <Q2HX711.h>

const byte hx711_data_pin1 = 5;
const byte hx711_clock_pin1 = 4;

//const byte hx711_data_pin2 = 5;
//const byte hx711_clock_pin2 = 4;

Q2HX711 hx711_1(hx711_data_pin1, hx711_clock_pin1);
//Q2HX711 hx711_2(hx711_data_pin2, hx711_clock_pin2);

void setup() {
  Serial.begin(9600);
}

void loop() {
  // built with 3.3v logic
  // ~8387 is the reading when the sensor is not hooked up
  // ~8170 seems to be 0lbs
  // ~8224 seems to be 6lbs
  // ~8250 1 gallon of water in jug... 8.36lbs
  Serial.println(hx711_1.read() / 100);
  delay(1000);
}
