#include "blind.h"

namespace philsson {
namespace blind {

Blind::Blind()
: m_stepper(D1, D3, D2, D4)
, m_position(0)
, m_targetPosition(0)
, m_posStep(0)
, m_targetPosStep(0)
, m_maxStep(10000)
, m_inverted(false)
, m_mode(Mode::REST)
, m_dir(Direction::UP)
, m_posUpdateCallback(nullptr)
, m_reachedTargetCallback(nullptr)
{}

void Blind::correctData(long currentStep,
                        long maxStep,
                        bool inverted)
{
  m_posStep = currentStep;
  m_targetPosStep = m_posStep;
  m_maxStep = maxStep;
  m_inverted = inverted;
  calculatePosition();
}

void Blind::setPosition(uint8_t position)
{
  m_targetPosStep = m_maxStep * position / 100;
  Serial.printf("New target: %d% at step %d\n", position, m_targetPosStep);
  m_mode = Mode::AUTO;
}

uint8_t Blind::getPosition()
{
  return m_position;
}

uint8_t Blind::getTargetPosition()
{
  return m_targetPosition;
}

long Blind::getStep()
{
  return m_posStep;
}

long Blind::getMaxStep()
{
  return m_maxStep;
}

void Blind::moveUp()
{
  m_mode = Mode::MANUAL;
  m_dir = Direction::UP;
}

void Blind::moveDown()
{
  m_mode = Mode::MANUAL;
  m_dir = Direction::DOWN;
}

void Blind::setInverted(bool inverted)
{
  m_inverted = inverted;
}

bool Blind::getInverted()
{
  return m_inverted;
}

void Blind::setOpen()
{
  // Setting Start Position
  m_maxStep = 0;
  m_posStep = 0;
  m_targetPosStep = 0;
  m_mode = Mode::REST;
}

void Blind::setClosed()
{
  // Setting End Position
  m_maxStep = m_posStep;
  m_targetPosStep = m_posStep;
  m_mode = Mode::REST;
}

void Blind::restCoils()
{
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);
  digitalWrite(D4, LOW);
}

void Blind::stop()
{
  m_targetPosStep = m_posStep;
  m_mode = Mode::REST;
  restCoils();
  Serial.printf("Stopped at pos %d and step %d\n", m_position, m_posStep);
}

void Blind::setPosUpdateCallback(void (*callback)(int, int))
{
  m_posUpdateCallback = callback;
}

void Blind::setReachedTargetCallback(void (*callback)(void))
{
  m_reachedTargetCallback = callback;
}

void Blind::run()
{
  switch (m_mode)
  {
  case Mode::REST:
    break;
  case Mode::AUTO:
    if (m_posStep == m_targetPosStep)
    {            
      Serial.println("Reached target position");
      stop();
      if (m_reachedTargetCallback)
      {
        m_reachedTargetCallback();
      }
    }
    else
    {
      long dir = (m_posStep < m_targetPosStep) ? 1 : -1;
      dir *= m_inverted ? -1 : 1;
      step(dir);
    }
    break;
  case Mode::MANUAL:
    long stepTarget = (m_dir == Direction::DOWN) ? 1 : -1;
    m_stepper.step(m_inverted ? -stepTarget : stepTarget);
    m_posStep += stepTarget;
    break;
  }
  calculatePosition();
}

bool Blind::step(long steps)
{
  // Logic to work even with negative maxStep
  if (abs(m_posStep + steps) >= 0 && abs(m_posStep + steps) <= abs(m_maxStep))
  {
    m_stepper.step(steps);
    m_posStep += steps;
  }
  else
  {
    Serial.printf("Trying to overstep. Step: %d, TargetStep: %d, MaxStep: %d\n", 
                  m_posStep, 
                  m_targetPosStep, 
                  m_maxStep);
    stop();
  }
}

void Blind::calculatePosition()
{
  long maxStep = (m_maxStep != 0) ? m_maxStep : 1;
  m_targetPosition = (m_targetPosStep * 100) / maxStep;
  m_position = (m_posStep * 100) / maxStep;

  // Callback
  static uint8_t compPos = m_position;
  if (compPos != m_position)
  {
    compPos = m_position;
    if (m_posUpdateCallback)
    {
      m_posUpdateCallback(m_position, m_targetPosition);
    }
  }
}

} // namespace blind
} // namespace philsson