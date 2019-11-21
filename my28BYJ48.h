#pragma once

#include <Stepper.h>
#include "IStepper.h"

class My28BYJ48 : public IStepper, private Stepper
{
public:
  My28BYJ48();

  void setSpeed(long rpm) override;

  void step(int steps) override;

  void rest() override;
};