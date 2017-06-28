int trigger = 0;
int shutter = 10000;

void setup() {
  pinMode(trigger, OUTPUT);
  digitalWrite(trigger, HIGH);
}

void loop() {
  digitalWrite(trigger, LOW);
  delay(50);
  digitalWrite(trigger, HIGH);
  delay(shutter);
}
