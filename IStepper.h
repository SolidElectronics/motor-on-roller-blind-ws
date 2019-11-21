#pragma once

class IStepper
{
public: 

  virtual void setSpeed(long rpm) = 0;

  virtual void step(int steps) = 0;

  virtual void rest() = 0;

};