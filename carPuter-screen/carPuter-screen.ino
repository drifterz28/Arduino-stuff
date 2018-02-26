const int buttonPin = 2;
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin

void setup() {
  // put your setup code here, to run once:
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  Serial.begin(9600);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  buttonState = digitalRead(buttonPin);
  Serial.println(buttonState);
  if(buttonState == LOW) {
    digitalWrite(4, LOW); 
    digitalWrite(5, LOW);
  }
//  digitalWrite(4, LOW); 
//  delay(500);
//  digitalWrite(5, HIGH);   // turn the LED on (HIGH is the voltage level)
//  delay(3500);
//  digitalWrite(5, LOW);
//  delay(500);
//  digitalWrite(4, HIGH);    // turn the LED off by making the voltage LOW
//  delay(3500);
}
