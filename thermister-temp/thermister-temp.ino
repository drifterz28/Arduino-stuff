#include <math.h>

#define thermister A3
#define fanPin 1

//User Inputs (Constant Values)
const int R2 = 1000.0; //Ohms
const int Ro = 10000.0; //Ohms   Thermistor resistance @ 25 degrees C or 298.2K
const int To = 298.2; //Kelvin
const int B = 3950.0; //Kelvin   B value for your Thermistor
const int Vin = 5.1; //Volts     Input Voltage

void setup() {
  pinMode(fanPin, OUTPUT);
  digitalWrite(fanPin, LOW);
}

void loop() {
  int AnalogSensor = analogRead(thermister);
  float Vo = AnalogSensor * (Vin / 1023.0);
  //Find R1
  float R1 = (R2 / Vo) * Vin - R2;
  //Calculate Temperature using the Thermistor equation
  float Tk = pow(((1.0 / To) + (1.0 / B) * log(R1 / Ro)), -1); //Kelvin
  float Tc = Tk - 273.2; //Celcius conversion
  float Tf = (9.0 / 5.1) * Tc + 32; //Celcius to Fahrenheit converstion
  int T = round(Tf);
  if(T >= 80) {
    digitalWrite(fanPin, HIGH);
  } else if(T < 79) {
    digitalWrite(fanPin, LOW);
  }
  delay(2000);
}

