#include "blind.h"

namespace philsson {
namespace blind {

Blind::Blind(IStepper *pStepper)
: m_pStepper(pStepper)
, m_position(0)
, m_targetPosition(0)
, m_posStep(0)
, m_targetPosStep(0)
, m_maxStep(10000)
, m_speedUp(5)
, m_speedDown(5)
, m_inverted(false)
, m_mode(Mode::REST)
, m_dir(Direction::UP)
, m_posUpdateCallback(nullptr)
, m_reachedTargetCallback(nullptr)
{
}

void Blind::correctData(long currentStep, long maxStep, bool inverted, long speedUp, long speedDown)
{
  m_posStep = currentStep;
  m_targetPosStep = m_posStep;
  m_maxStep = maxStep;
  m_inverted = inverted;
  m_speedUp = speedUp;
  m_speedDown = speedDown;
  calculatePosition();
}

void Blind::setPosition(uint8_t position)
{
  m_targetPosStep = m_maxStep * position / 100;
  Serial.printf("New target: %d% at step %d\n", position, m_targetPosStep);
  m_mode = Mode::AUTO;
  m_dir = (m_posStep < m_targetPosStep) ? Direction::DOWN : Direction::UP;
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
  m_pStepper->rest();
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
    // Delay to conserve power if not running the motor.
    delay(25);
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
      step(1, m_dir, m_mode);
    }
    calculatePosition();
    break;
  case Mode::MANUAL:
    step(1, m_dir, m_mode);
    break;
  }
}

bool Blind::step(long steps, Direction dir, Mode mode)
{
  if (dir == Direction::UP)
  {
    steps *= -1;
  }
  // Logic to work even with negative maxStep
  if (mode == Mode::AUTO && (abs(m_posStep + steps) < 0 || abs(m_posStep + steps) > abs(m_maxStep)))
  {
    Serial.printf("Trying to overstep. Step: %d, TargetStep: %d, MaxStep: %d\n", m_posStep,
                  m_targetPosStep, m_maxStep);
    stop();
    return false;
  }

  m_pStepper->setSpeed((dir == Direction::UP) ? m_speedUp : m_speedDown);
  m_pStepper->step(m_inverted ? -steps : steps);
  m_posStep += steps;
  return true;
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

void Blind::setSpeed(long rpm)
{
  m_speedUp = rpm;
  m_speedDown = rpm;
}

void Blind::setSpeed(long rpm, Direction dir)
{
  Serial.printf("Setting new %s direction speed to: %d",
                (dir == Direction::UP) ? "upward" : "downward", rpm);
  switch (dir)
  {
  case Direction::UP:
    m_speedUp = rpm;
    break;
  case Direction::DOWN:
    m_speedDown = rpm;
    break;
  }
}

long Blind::getSpeed(Direction dir)
{
  return (dir == Direction::UP) ? m_speedUp : m_speedDown;
}

} // namespace blind
} // namespace philsson