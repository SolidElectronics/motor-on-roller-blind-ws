#include "my28BYJ48.h"

#include "Arduino.h"

#define STEPS 2038 // the number of steps in one revolution (28BYJ-48)

My28BYJ48::My28BYJ48()
: Stepper(STEPS, D8, D6, D7, D5)
{
}

void My28BYJ48::setSpeed(long rpm)
{
  Stepper::setSpeed(rpm);
}

void My28BYJ48::step(int steps)
{
  Stepper::step(steps);
}

void My28BYJ48::rest()
{
  digitalWrite(D5, LOW);
  digitalWrite(D6, LOW);
  digitalWrite(D7, LOW);
  digitalWrite(D8, LOW);
}