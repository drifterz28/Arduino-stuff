#include <SoftwareSerial.h>
SoftwareSerial DebugSerial(0, 1); // RX, TX

#define BLYNK_PRINT DebugSerial
#include <BlynkSimpleStream.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "b3ca357851c0430da2af4ef68f12bef0";

void setup()
{
  // Debug console
  DebugSerial.begin(9600);

  // Blynk will work through Serial
  // 9600 is for HC-06. For HC-05 default speed is 38400
  Serial.begin(9600);
  Blynk.begin(auth, Serial);
}

void loop()
{
  Blynk.run();
}
