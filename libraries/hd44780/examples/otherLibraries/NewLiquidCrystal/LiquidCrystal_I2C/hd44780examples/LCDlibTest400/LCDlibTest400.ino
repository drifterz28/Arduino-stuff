// ----------------------------------------------------------------------------
// LCDLibTest - LCD library test sketch: NewLiquidCrystal with LiquidCrystal_I2C
// ----------------------------------------------------------------------------
// This sketch is a wrapper sketch for the hd44780 library example.
// Note:
// This is not a normal sketch and should not be used as model or exmaple
// of hd44780 library sketches.
// This sketch is simple wrapper that declares the needed lcd object for the
// hd44780 library sketch.
// It is provided as a convenient way to run a pre-configured sketch for
// the i/o class.
// The source code for this sketch lives in the hd44780 examples.
// hd44780/examples/hd44780examples/LCDlibTest/LCDlibTest.ino
// From IDE:
// [File]->Examples-> hd44780/hd44780examples/LCDlibTest
//

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ugliness to allow this sketch to locate the hd44780 example sketch
// note: hd44780.h is not needed, it is only included to get its directory on the include path
#include <hd44780.h>
#undef hd44780_h // undefine this so the example sketch does not think hd44780 is being used.

#if ARDUINO < 157
#error "This sketch Requires Arduino 1.5.7 or higher"
#endif

// NOTE: uses API that only works on IDE 1.5.7 and up
#define WIRECLOCK 400000L // tell hd44780 example to use this i2c clock rate

#define LCD_COLS 16
#define LCD_ROWS 2

// declare the lcd object
// Note: The i2c address and pin mappings must match the backpack
const uint8_t i2cAddr = 0x27;
const int rs=0, rw=1, en=2, db4=4, db5=5, db6=6, db7=7, bl=3, blpol=POSITIVE;
LiquidCrystal_I2C lcd(i2cAddr, en, rw, rs, db4, db5, db6, db7, bl, blpol);

// tell the hd44780 sketch the lcd object has been declared
#define HD44780_LCDOBJECT

// include the hd44780 library sketch source code
#include <examples/hd44780examples/LCDlibTest/LCDlibTest.ino>
