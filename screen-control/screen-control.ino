const int inputTrigger = 5;
const int screenSensor = 2;
const int motorTrigger1 = 3;
const int motorTrigger2 = 4;

int triggerState = 0;
int screenState = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(motorTrigger1, OUTPUT);
  pinMode(motorTrigger2, OUTPUT);
  pinMode(inputTrigger, INPUT_PULLUP);
  pinMode(screenSensor, INPUT);
  digitalWrite(motorTrigger2, LOW);
  digitalWrite(motorTrigger1, LOW);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  triggerState = digitalRead(inputTrigger);
  screenState = digitalRead(screenSensor);
  Serial.print("triggerState: ");
  Serial.println(triggerState);
  Serial.print("screenState: ");
  Serial.println(screenState);
  delay(500);
  if(triggerState == HIGH && screenState == LOW) {
    // turn LED on:
    digitalWrite(motorTrigger2, LOW);
    digitalWrite(motorTrigger1, HIGH);
  }
  if(triggerState == HIGH && screenState == HIGH) {
    // turn LED on:
    digitalWrite(motorTrigger1, LOW);
  }
  if(triggerState == LOW && screenState == HIGH) {
    // turn LED off:
    digitalWrite(motorTrigger1, LOW);
    digitalWrite(motorTrigger2, HIGH);
  }
  if(triggerState == LOW && screenState == LOW) {
    // turn LED on:
    digitalWrite(motorTrigger2, LOW);
  }
}
