#include <BnrOneAPlus.h>  // Bot'n Roll ONE A+ library
#include <math.h>  // math library for calculations
#include <SPI.h>  // SPI communication library required by BnrOneAPlus.cpp
BnrOneAPlus one;  // object to control the Bot'n Roll ONE A+

// constants definition
#define SSPIN 2  // Slave Select (SS) pin for SPI communication
#define OFF 0
#define ON 1
#define CHALLENGE_TIME 90  // challenge time in seconds
#define MINIMUM_BATTERY_V 10.5  // safety voltage for discharging the battery

int counter = 0;
int state = 0;
float average = 0.0;
int sensor[8];

void setup() {
  one.spiConnect(SSPIN);                  // start SPI communication module
  one.setMinBatteryV(MINIMUM_BATTERY_V);  // battery discharge protection
  one.stop();                             // stop motors
  one.lcd1(" FUN CHALLENGE  ");           // print on LCD line 1
  one.lcd2(" Press a button ");           // print on LCD line 2
  while (one.readButton() == 0)
    ;
}

void loop() {
  average = 0;
  // Read 8 line sensors and calculate average
  for (int i = 0; i < 8; i++) {
    sensor[i] = one.readAdc(i);
    average += sensor[i] / 8;
  }

  // State machine: each state represents a different task
  // Conditions in each state determine transitions

  switch (state) {
    case 0:  // Move forward
      move_forward_gradient();

      // Stop if obstacle detected
      if (one.readObstacleSensors() > 0) {
        state = 1;
      }

      // Stop if all sensors see black line (avg > 900)
      if (average > 900) {
        state = 1;
      }

      break;

    case 1:  // Move backward
      one.move(-80, -80);

      // Continue until all sensors see white (avg < 100)
      if (average < 100) {
        state = 2;
      }

      break;

    case 2:  // Move backward (robot lifted)
      one.move(-80, -80);

      // Continue until sensors see black again (robot set down)
      if (average > 900) {
        state = 3;
      }

      break;

    case 3:  // Move forward (robot on track)
      move_forward_gradient();

      // Return to forward movement when on white area
      if (average < 100) {
        state = 0;
      }

      break;
  }
} 

void move_forward_gradient() {
  // Move quickly at first and then slow down as time passes
  int speed = 85;
  for (int counter = 0; counter < 50; counter++) {
    speed = pow(2,-counter / 10.0) * 85;  // Exponential decay of speed
    one.move(speed, speed);
    delay(10);
}
}