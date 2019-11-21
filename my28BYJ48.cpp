#include "my28BYJ48.h"

#include "Arduino.h"

#define STEPS 2038 // the number of steps in one revolution (28BYJ-48)

My28BYJ48::My28BYJ48()
: Stepper(STEPS, D4, D3, D2, D1)
{}

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
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);
  digitalWrite(D4, LOW);
}