/*-----( Import needed libraries )-----*/
#include <Stepper.h>

/*-----( Declare Constants, Pin Numbers )-----*/
//---( Number of steps per revolution of INTERNAL motor in 4-step mode )---
#define STEPS_PER_MOTOR_REVOLUTION 32   

//---( Steps per OUTPUT SHAFT of gear reduction )---
#define STEPS_PER_OUTPUT_REVOLUTION 32 * 64  //2048  

/*-----( Declare objects )-----*/
// create an instance of the stepper class, specifying
// the number of steps of the motor and the pins it's
// attached to

//The pin connections need to be pins 8,9,10,11 connected
// to Motor Driver In1, In2, In3, In4 

// Then the pins are entered here in the sequence 1-3-2-4 for proper sequencing
Stepper small_stepper(STEPS_PER_MOTOR_REVOLUTION, 8, 10, 9, 11);

int  Steps2Take;

void setup()
{
// Nothing  (Stepper Library sets pins as outputs)
}

void loop()   /*----( LOOP: RUNS CONSTANTLY )----*/
{
//  small_stepper.setSpeed(1);   // SLOWLY Show the 4 step sequence 
//  Steps2Take  =  4;  // Rotate CW
//  small_stepper.step(Steps2Take);
//  delay(2000);

  Steps2Take  =  STEPS_PER_OUTPUT_REVOLUTION;  // Rotate CW 1/2 turn
  small_stepper.setSpeed(800);   
  small_stepper.step(Steps2Take);
  delay(1000);
  
  Steps2Take  =  -STEPS_PER_OUTPUT_REVOLUTION;  // Rotate CCW 1/2 turn  
  small_stepper.setSpeed(800);  // 700 a good max speed??
  small_stepper.step(Steps2Take);
  delay(2000);

}
