//I am Using a 128 x 64 Graphics LCD for ths project.
//Very Cool and perfect result
//Pictures are attached.

#include "U8glib.h"

U8GLIB_ST7920_128X64_4X u8g(10); // SPI Com: SCK = en = 13, MOSI = rw = 11, CS = di = 10, HW SPI

//calculations
//tire radius ~ 13.5 inches
//circumference = pi*2*r =~85 inches
//max speed of 35mph =~ 616inches/second
//max rps =~7.25

#define reed A0 //pin connected to read switch

//storage variables
int reedVal;
long timer; // time between one full rotation (in ms)
int mph;
float radius = 13.5; // tire radius (in inches)
float circumference;
int kph;

int rpm;

int maxReedCounter = 10; //min time (in ms) of one rotation (for debouncing)
int reedCounter;

// A Splash Screen, To be displayed for the first 6 seconds, the time can be adjusted in the loop().
void draw_splash(void) {
  u8g.setFont(u8g_font_helvB12);
  u8g.drawStr(22, 13, "WELCOME");
  u8g.setFont(u8g_font_8x13);
  u8g.drawStr(22, 25, "PLEASE WEAR");
  u8g.drawStr(22, 38, "YOUR HELMET");
  u8g.drawStr(19, 50, "Designed By:");
  u8g.drawStr(9, 63, "Shahid Khattak");
}

void draw_main(void) {
  u8g.setFont(u8g_font_timR10);
  u8g.drawStr(4, 12, "Speed Km / h");
  u8g.drawLine(0, 16, 84, 16);

  u8g.setFont(u8g_font_fur42n);
  u8g.setPrintPos(0, 62);
  u8g.print(kph);

  u8g.setFont(u8g_font_fub11);
  u8g.drawStr(88, 12, "RPM");

  u8g.setFont(u8g_font_8x13);
  //u8g.drawStr(86, 29,"01234");
  u8g.setPrintPos(86, 29);
  u8g.print(rpm);

}

void setup(void) {
  // flip screen, if required
  u8g.setRot180(); // comment out this line if you do not want to flip the screen
  reedCounter = maxReedCounter;
  circumference = 2 * 3.14 * radius;
  pinMode(reed, INPUT);
  pinMode(gear_n, INPUT);

  // TIMER SETUP- the timer interrupt allows precise timed measurements of the reed switch
  //for more info about configuration of arduino timers see http://arduino.cc/playground/Code/Timer1
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
}

ISR(TIMER1_COMPA_vect) {
  //Interrupt at freq of 1kHz to measure reed switch
  reedVal = digitalRead(reed); //get val of A0
  if (reedVal) { //if reed switch is closed
    if (reedCounter == 0) {
      //min time between pulses has passed
      mph = (56.8 * float(circumference)) / float(timer); //calculate miles per hour
      rpm = (float(60000)) / (float(timer));
      timer = 0;                    //reset timer
      reedCounter = maxReedCounter; //reset reedCounter
      kph = mph * 1.609344;
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
    kph = 0; //if no new pulses from reed switch- tire is still, set mph, rpm and kph to 0
    rpm = 0;
  } else {
    timer += 1; //increment timer
  }
}

void loop(void) {
  if (millis() < 6000) { // The time in milli seconds, for you want to show the splash screen 6000 = 6 Seconds
    u8g.firstPage();
    do {
      draw_splash();
    } while (u8g.nextPage());
  } else {
    u8g.firstPage();
    do {
      draw_main();
    } while (u8g.nextPage());
    // rebuild the picture after some delay
    delay(50);
  }
}

