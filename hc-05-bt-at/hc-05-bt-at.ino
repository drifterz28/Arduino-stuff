#include <SoftwareSerial.h>
SoftwareSerial mySerial(10, 11); // RX, TX

void setup() {
  Serial.begin(38400);
  pinMode(9, OUTPUT); 
  digitalWrite(9, HIGH);
  Serial.println("Enter AT commands:");
  mySerial.begin(9600);

}

void loop() {

  if (mySerial.available())
    Serial.write(Serial.read());
  if (Serial.available())
    mySerial.write(Serial.read());
}
