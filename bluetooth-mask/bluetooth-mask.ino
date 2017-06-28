// Bluetooth Goggles Sketch -- shows the Adafruit Bluefruit LE UART Friend
// can be used even with Trinket or Gemma!

// https://www.adafruit.com/products/2479

// Works in conjunction with Bluefruit LE Connect app on iOS or Android --
// pick colors or use '1' and '2' buttons to select pinwheel or sparkle modes.
// You can try adding more, but space is VERY tight...helps to use Arduino IDE
// 1.6.4 or later; produces slightly smaller code than the 1.0.X releases.

// BLUEFRUIT LE UART FRIEND MUST BE SWITCHED TO 'UART' MODE

#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR_ATtiny85__ // Trinket, Gemma, etc.
 #include <avr/power.h>
#endif

#define RX_PIN    2 // Connect this Trinket pin to BLE 'TXO' pin
#define CTS_PIN   1 // Connect this Trinket pin to BLE 'CTS' pin
#define LED_PIN   0 // Connect NeoPixels to this Trinket pin
#define NUM_LEDS 12 // Two 16-LED NeoPixel rings
#define FPS      60 // Animation frames/second (ish)

SoftwareSerial    ser(RX_PIN, -1);
Adafruit_NeoPixel pixels(NUM_LEDS, LED_PIN);

void setup() {
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  // MUST do this on 16 MHz Trinket for serial & NeoPixels!
  clock_prescale_set(clock_div_1);
#endif
  // Stop incoming data & init software serial
  pinMode(CTS_PIN, OUTPUT); 
  digitalWrite(CTS_PIN, HIGH);
  ser.begin(9600);

  pixels.begin(); // NeoPixel init
  // Flash space is tight on Trinket/Gemma, so setBrightness() is avoided --
  // it adds ~200 bytes.  Instead the color picker input is 'manually' scaled.
}

uint8_t  buf[3],              // Enough for RGB parse; expand if using sensors
         animMode = 0,        // Current animation mode
         animPos  = 0;        // Current animation position
uint32_t color    = 0x400000, // Current animation color (red by default)
         prevTime = 0L;       // For animation timing

void loop(void) {
  int      c;
  uint32_t t;

  // Animation happens at about 30 frames/sec.  Rendering frames takes less
  // than that, so the idle time is used to monitor incoming serial data.
  digitalWrite(CTS_PIN, LOW); // Signal to BLE, OK to send data!
  for(;;) {
    t = micros();                            // Current time
    if((t - prevTime) >= (1000000L / FPS)) { // 1/30 sec elapsed?
      prevTime = t;
      break;                                 // Yes, go update LEDs
    }                                        // otherwise...
    if((c = ser.read()) == '!') {            // Received UART app input?
      while((c = ser.read()) < 0);           // Yes, wait for command byte
      switch(c) {
       case 'B':       // Button (Control Pad)
        if(readAndCheckCRC(255-'!'-'B', buf, 2) & (buf[1] == '1')) {
          buttonPress(buf[0]); // Handle button-press message
        }
        break;
       case 'C':       // Color Picker
        if(readAndCheckCRC(255-'!'-'C', buf, 3)) {
          // As mentioned earlier, setBrightness() was avoided to save space.
          // Instead, results from the color picker (in buf[]) are divided
          // by 4; essentially equivalent to setBrightness(64).  This is to
          // improve battery run time (NeoPixels are still plenty bright).
          color = pixels.Color(buf[0]/4, buf[1]/4, buf[2]/4);
        }
        break;
       case 'Q':       // Quaternion
        skipBytes(17); // 4 floats + CRC (see note below re: parsing)
        break;
       case 'A':       // Accelerometer
#if 0
        // The phone sensors are NOT used by this sketch, but this shows how
        // they might be read.  First, buf[] must be delared large enough for
        // the expected data packet (minus header & CRC) -- that's 16 bytes
        // for quaternions (above), or 12 bytes for most of the others.
        // Second, the first arg to readAndCheckCRC() must be modified to
        // match the data type (e.g. 'A' here for accelerometer).  Finally,
        // values can be directly type-converted to float by using a suitable
        // offset into buf[] (e.g. 0, 4, 8, 12) ... it's not used in this
        // example because floating-point math uses lots of RAM and code
        // space, not suitable for the space-constrained Trinket/Gemma, but
        // maybe you're using a Pro Trinket, Teensy, etc.
        if(readAndCheckCRC(255-'!'-'A', buf, 12)) {
          float x = *(float *)(&buf[0]),
                y = *(float *)(&buf[4]),
                z = *(float *)(&buf[8]);
        }
        // In all likelihood, updates from the buttons and color picker
        // alone are infrequent enough that you could do without any mention
        // of the CTS pin in this code.  It's the extra sensors that really
        // start the firehose of data.
        break;
#endif
       case 'G':       // Gyroscope
       case 'M':       // Magnetometer
       case 'L':       // Location
        skipBytes(13); // 3 floats + CRC
      }
    }
  }
  digitalWrite(CTS_PIN, HIGH); // BLE STOP!

  // Show pixels calculated on *prior* pass; this ensures more uniform timing
  pixels.show();

  // Then calculate pixels for *next* frame...
  switch(animMode) {
   case 1: // Pinwheel mode
    for(uint8_t i=0; i<NUM_LEDS/2; i++) {
      uint32_t c = 0;
      if(((animPos + i) & 7) < 2) c = color; // 4 pixels on...
      pixels.setPixelColor(   i, c);         // First eye
      pixels.setPixelColor(NUM_LEDS-1-i, c); // Second eye (flipped)
    }
    animPos++;
    break;
   case 0: // Sparkle mode
    pixels.setPixelColor(animPos, 0);     // Erase old dot
    animPos = random(NUM_LEDS);           // Pick a new one
    pixels.setPixelColor(animPos, color); // and light it
    break;
  }
}

boolean readAndCheckCRC(uint8_t sum, uint8_t *buf, uint8_t n) {
  for(int c;;) {
    while((c = ser.read()) < 0); // Wait for next byte
    if(!n--) return (c == sum);  // If CRC byte, we're done
    *buf++ = c;                  // Else store in buffer
    sum   -= c;                  // and accumulate sum
  }
}

void skipBytes(uint8_t n) {
  while(n--) {
    while(ser.read() < 0);
  }
}

void buttonPress(char c) {
  pixels.clear(); // Clear pixel data when switching modes (else residue)
  switch(c) {
   case '1':
    animMode = 0; // Switch to pinwheel mode
    break;
   case '2':
    animMode = 1; // Switch to sparkle mode
    break;
   case '3':
    break;
   case '4':
    break;
   case '5': // Up
    break;
   case '6': // Down
    break;
   case '7': // Left
    break;
   case '8': // Right
    break;
  }
}
