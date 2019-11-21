#pragma once

#include "IStepper.h"


class A4988 : public IStepper {
public: 

  A4988();

  void setSpeed(long rpm) override;

  void step(int steps) override;

  void rest() override;

private:

  int m_pinEN, m_pinDir, m_pinStep;

  unsigned long m_stepperDelay;

  int m_dir; // forward: 1, backward: -1

  


};