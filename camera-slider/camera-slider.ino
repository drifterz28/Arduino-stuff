#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Button.h>

#if defined(ARDUINO) && ARDUINO >= 100
#define printByte(args)  write(args);
#else
#define printByte(args)  print(args,BYTE);
#endif

uint8_t heart[8] = {0x0,0xa,0x1f,0x1f,0xe,0x4,0x0};
LiquidCrystal_I2C lcd(0x27,16, 2);

Button left = Button(2, PULLUP);
Button down = Button(3, PULLUP);
Button right = Button(4, PULLUP);
Button top = Button(5, PULLUP);

Button back = Button(6, PULLUP);
Button enter = Button(7, PULLUP);

int sliderLength = 0;
int holdTime = 0;
int totalTime = 0;

bool totalTimeSet = false;
bool holdTimeSet = false;
bool sliderLengthSet = false;

void setup() {
  lcd.init(); 
  lcd.backlight();
  lcd.createChar(3, heart);
  lcd.setCursor(3, 0);
  lcd.print("Made with ");
  lcd.setCursor(7, 1);
  lcd.printByte(3);
  Serial.begin(9600);
  Serial.println("start");
  delay(2000);
  //lcd.noBacklight();
  lcd.clear();
}

void setTime() {
  lcd.print("Total time    ");
  lcd.setCursor(0, 1);
  lcd.print(totalTime);
  lcd.print(" sec    ");
  if (down.isPressed() && totalTime > 0) {
    totalTime = totalTime - 10;
  }
  if (top.isPressed()) {
    totalTime = totalTime + 10;
  }
  if (enter.isPressed()) {
    holdTimeSet = true;
  }
}

void setHold() {
  lcd.print("Hold time    ");
  lcd.setCursor(0, 1);
  lcd.print(holdTime);
  lcd.print(" sec    ");
  if (down.isPressed() && holdTime > 0) {
    holdTime--;
  }
  if (top.isPressed()) {
    holdTime++;
  }
  if (enter.isPressed()) {
    totalTimeSet = true;
  }
}

void setSliderLength() {
  lcd.print("Slider length    ");
  lcd.setCursor(0, 1);
  lcd.print(sliderLength);
  lcd.print(" sec    ");
  if (down.isPressed() && sliderLength > 0) {
    sliderLength--;
  }
  if (top.isPressed()) {
    sliderLength++;
  }
  if (enter.isPressed()) {
    sliderLengthSet = true;
  }
}

void loop() {
  lcd.setCursor(0, 0);
  if(!totalTimeSet) {
    setTime();
  } else if(!holdTimeSet) {
    setHold();
  } else if(!sliderLengthSet) {
    setSliderLength();
  }
  delay(20);
}



