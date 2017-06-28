// Dropping Spider
/*
   PIR sensor tester
*/

int stopPin = 3;                // choose the pin for the LED
int inputPin = 2;               // choose the input pin (for PIR sensor)
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;                    // variable for reading the pin status
int motorOne = 0;
int motorTwo = 1;

void setup() {
  pinMode(ledPin, OUTPUT);      // declare LED as output
  pinMode(inputPin, INPUT);     // declare sensor as input
  analogWrite(motorOne, 0);
  analogWrite(motorTwo, 0);
}

void loop() {
  val = digitalRead(inputPin);
  if (val == HIGH) {
    if (pirState == LOW) {
      // drop spider down as fast as the motor can.
      analogWrite(motorOne, 255);
      delay(500);
      // stop motor so we dont pull it back up.
      analogWrite(motorOne, 0);
      pirState = HIGH;
    }
  } else {
    if (pirState == HIGH) {
      // will not want it to be part of the loop
      // we have just turned of, pull back up
      //analogWrite(motorTwo, 125);
      //pirState = LOW;
    }
  }
}
