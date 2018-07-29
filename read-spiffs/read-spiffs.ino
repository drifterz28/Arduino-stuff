#include "FS.h"
char* db_name = "/temps.db";
void setup() {
  Serial.begin(9600);
  SPIFFS.begin();
  Serial.println("test");
  // put your setup code here, to run once:
  SPIFFS.remove(db_name);
  Serial.println("test");
}

void loop() {}
