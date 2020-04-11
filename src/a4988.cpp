#include "a4988.h"

#include "Arduino.h"

#define STEPS 2038 // the number of steps in one revolution (28BYJ-48)


A4988::A4988()
: m_pinEN(D1)
, m_pinDir(D2)
, m_pinStep(D3)
, m_stepperDelay(0) // TODO: Some good value
, m_dir(1)
{
  pinMode(m_pinEN, OUTPUT);
  pinMode(m_pinDir, OUTPUT);
  pinMode(m_pinStep, OUTPUT);
}

void A4988::setSpeed(long rpm)
{
  m_stepperDelay = 60L * 10e6L / STEPS / rpm;
}

void A4988::step(int steps)
{
  if (steps < 0 && m_dir != -1)
  {
    digitalWrite(m_pinDir, LOW);
    m_dir = -1;
  }
  else if (steps > 0 && m_dir != 1)
  {
    digitalWrite(m_pinDir, HIGH);
    m_dir = 1;
  }

  int stepsLeft = abs(steps);
  digitalWrite(m_pinEN, LOW);

  while (stepsLeft > 0)
  {
    unsigned long now = micros();
    static unsigned long lastStepTime = now;
    static bool runResetPin = true;

    // move only if the appropriate delay has passed:
    if (now - lastStepTime >= m_stepperDelay/2)
    {
      lastStepTime = now;

      // Every second loop we go low
      if (runResetPin)
      {
        digitalWrite(m_pinStep, LOW);
        runResetPin = false;
        continue;
      }
        
      digitalWrite(m_pinStep, HIGH);
      stepsLeft--;
      runResetPin = true;
    }
  }
}

void A4988::rest()
{
  digitalWrite(m_pinEN, HIGH);
}
