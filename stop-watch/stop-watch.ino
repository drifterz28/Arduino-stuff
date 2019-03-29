#include <Adafruit_SSD1306.h>
#include <OneButton.h>
#include <SPI.h>
#include <Wire.h>

Adafruit_SSD1306 display(4);
#define reed A1
OneButton resetBtn(A0, true);
OneButton startStopBtn(4, true);

int reedVal;
long timer; // time between one full rotation (in ms)
float mph;
float lastMph;
float radius = 13; // tire radius (in inches)
float circumference;

int rpm;
long reedCount = 0;
int maxReedCounter = 10; //min time (in ms) of one rotation (for debouncing)
int reedCounter;
float distance = 0;
bool clockRunning = false;
long startTime;
long elapsedTime;
int lastSecond = 1;

void setup() {
  Serial.begin(9600);
  pinMode(reed, INPUT);
  reedCounter = maxReedCounter;
  circumference = 2 * 3.14 * radius;
  resetBtn.attachClick(resetTime);
  startStopBtn.attachClick(startStop);
  cli(); //stop interrupts
    //set timer1 interrupt at 1kHz
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B
  TCNT1 = 0;
  // set timer count for 1khz increments
  OCR1A = 1999; // = (1/1000) / ((1/(16*10^6))*8) - 1
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS11 bit for 8 prescaler
  TCCR1B |= (1 << CS11);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  sei(); //allow interrupts
  //END TIMER SETUP
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("");
  display.println("Let's go for a ride");
  display.display();
  delay(2000);
}

ISR(TIMER1_COMPA_vect) {
  //Interrupt at freq of 1kHz to measure reed switch
  reedVal = digitalRead(reed); //get val of A1
  if (reedVal) { //if reed switch is closed
    if (reedCounter == 0) {
      //min time between pulses has passed
      Serial.println((56.8 * float(circumference)) / float(timer));
      mph = (56.8 * float(circumference)) / float(timer); //calculate miles per hour
      rpm = (float(60000)) / (float(timer));
      timer = 0;                    //reset timer
      reedCounter = maxReedCounter; //reset reedCounter
    }
  } else {
    //if reed switch is open
    if (reedCounter > 0){
      //don't let reedCounter go negative
      reedCounter -= 1; //decrement reedCounter
    }
  }
  if (timer > 1000) {
    mph = 0;
    rpm = 0;
  } else {
    timer += 1; //increment timer
  }
}

void resetTime() {
  Serial.println("reset button");
  clockRunning = false;
  lastSecond = startTime;
  startTime = 0;
  reedCount = 0;
  elapsedTime = 0;
}

void startStop() {
  Serial.println("start stop button");
  if(clockRunning) {
    clockRunning = !clockRunning;
  } else {
    clockRunning = !clockRunning;
    startTime = millis();
  }
}

void loop() {
  int h, m, s;
  unsigned long over;
  startStopBtn.tick();
  resetBtn.tick();
  if(clockRunning) {
    if(millis() - startTime > 1000) {
      startTime = millis();
      elapsedTime++;
    }
    distance = (reedCount * 6.8033) / 5280;
  }

  h = int(elapsedTime / 3600);
  over = elapsedTime % 3600;
  m = int(over / 60);
  over = over % 60;
  s = int(over);
  if(lastSecond != s || lastMph != mph) {
    lastMph = mph;
    lastSecond = s;
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0,0);
    display.print(mph);
    display.setTextSize(1);
    display.print(" mph");
    display.setTextSize( 1);
    display.setCursor(0,16);
    display.print(h < 10 ? "T:0" : "T:");
    display.print(h);
    display.print(m < 10 ? ":0" : ":");
    display.print(m);
    display.print(s < 10 ? ":0" : ":");
    display.println(s);
    display.print("D:");
    display.print(distance);
    display.display();
  }
  
}
