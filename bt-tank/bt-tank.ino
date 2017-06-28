#include <SoftwareSerial.h>
 
const int RX_PIN = 10;
const int TX_PIN = 9;
const int BLUETOOTH_BAUD_RATE = 9600;
 
SoftwareSerial bluetooth(RX_PIN, TX_PIN);
 
void setup() {
   Serial.begin(9600);
   bluetooth.begin(BLUETOOTH_BAUD_RATE);
}
//class Move {
//  void rightTrack() {}
//  void leftTrack() {}
//  void forward() {}
//  void backward {}
//}
void loop() {
  
  if (bluetooth.available()) {
    Serial.write(bluetooth.read());
  }
}
