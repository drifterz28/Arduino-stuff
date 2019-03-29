#include <SoftwareSerial.h>

int sensorPin = A1;    // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor
int relayPin = 0;
int onValue = 550;
int pullDelay = 5; // in seconds
void setup() {
  // declare the ledPin as an OUTPUT:
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
}

void loop() {
  // read the value from the sensor:
  sensorValue = analogRead(sensorPin);
  Serial.println(sensorValue);
  if(sensorValue > onValue) {
    digitalWrite(relayPin, HIGH);
    Serial.println("HIGH");
  }
  if(sensorValue <= onValue) {
    digitalWrite(relayPin, LOW);
    Serial.println("LOW");
  }
  delay(pullDelay * 1000);
}
