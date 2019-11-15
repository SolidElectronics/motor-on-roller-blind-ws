#pragma once

#include "Arduino.h"
#include <Stepper.h>

namespace philsson {
namespace blind {

class Blind {
public:

  enum Mode {
    REST,
    AUTO,
    MANUAL,
  };

  enum Direction {
    UP,
    DOWN,
  };

  Blind();

  //! Set position and configuration
  //! Possibly set at startup from memory
  //! @param currentStep The step the blinder is at
  //!                    or was at upon last save
  //! @param maxStep     Num of steps from closed to open
  //! @param inverted    Dir of motor
  //! @param speedUp     RPM of the blinder Upwards
  //! @param speedDown   RPM of the blinder Downwards
  void correctData(long currentStep,
                   long maxStep,
                   bool inverted,
                   long speedUp,
                   long speedDown);

  //! Move blind to position
  //! @param position [0, 100] from Open to Closed
  void setPosition(uint8_t position);

  //! @return the current relative position [0, 100]
  uint8_t getPosition();

  //! @return the current target position [0, 100]
  uint8_t getTargetPosition();

  //! @return Steps to current position
  long getStep();

  //! @return Num of steps from closed to open
  long getMaxStep();

  //! Start retracting the blind. It will not stop until stopped
  void moveUp();

  //! Start unrolling the blind. It will not stop until stopped
  void moveDown();

  void setInverted(bool inverted);

  bool getInverted();

  //! Store current position as Open
  void setOpen();

  //! Store current position as Closed
  void setClosed();

  //! Turn off power to coils
  void restCoils();

  void stop();

  void setPosUpdateCallback(void (*callback)(int, int));

  void setReachedTargetCallback(void (*callback)(void));

  //! Main loop of this component
  //! This function will drive the movement of the blind
  //! depending on the current state and objective
  void run();

  //! Set the speed of the blinder
  //! @param rpm Revolutions per minute of the motor
  //!            Defaults to 5. A too high value will just stall
  void setSpeed(long rpm);

  void setSpeed(long rpm, Direction dir);

  long getSpeed(Direction dir);

private:

  //! Step the motor. If not allowed to step (Not in [0, 100]) return false
  //! @param steps     Amount of steps
  //! @param direction Direction
  //! @param mode      MANUAL will not stop. AUTO will stop at limit
  bool step(long steps, Direction dir, Mode mode);

  void calculatePosition();

  // Stepper_28BYJ_48 m_stepper;
  Stepper m_stepper;

  // Relative position in percent
  uint8_t m_position;
  // Relative target position in percent
  uint8_t m_targetPosition;

  // Absolute position in steps
  long m_posStep;
  // Absolute target position in steps
  long m_targetPosStep;

  // Num of steps from closed to open
  long m_maxStep;

  long m_speedUp, m_speedDown;

  bool m_inverted;

  Mode m_mode;

  Direction m_dir;

  void (*m_posUpdateCallback)(int, int);
  void (*m_reachedTargetCallback)(void);
};

}
}