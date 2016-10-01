#include "HX711.h"

// HX711.DOUT	- pin #A1
// HX711.PD_SCK	- pin #A0

HX711 scale(4, 5);		// parameter "gain" is ommited; the default value 128 is used by the library

void setup() {
  Serial.begin(9600);
  Serial.println("HX711 Demo");

  Serial.println("Before setting up the scale:");
  Serial.print("read: \t\t");
  Serial.println(scale.read());			// print a raw reading from the ADC

  Serial.print("read average: \t\t");
  Serial.println(scale.read_average(20));  	// print the average of 20 readings from the ADC

  Serial.print("get value: \t\t");
  Serial.println(scale.get_value(5));		// print the average of 5 readings from the ADC minus the tare weight (not set yet)

  Serial.print("get units: \t\t");
  Serial.println(scale.get_units(5), 1);	// print the average of 5 readings from the ADC minus tare weight (not set) divided 
						// by the SCALE parameter (not set yet)  

  scale.set_scale(-646.38);// this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();// reset the scale to 0
}

void loop() {
  //Serial.print("one reading:\t");
  //Serial.print(scale.get_units(), 1);
  Serial.print("\t| average:\t");
  Serial.println(scale.get_units(10));
  // 1lbs 11379.00 = 11379 byOz 711.18
  // 3lbs 31171.00 = 10390.33 = 649.39
  // 4lbs 41713.00 = 10428.25 = 651.76
  // 6lbs 60654.00 = 10109 = 631.81
  // 10lbs 103422.00 = 10342.2 = 646.38
  delay(2000);
}
