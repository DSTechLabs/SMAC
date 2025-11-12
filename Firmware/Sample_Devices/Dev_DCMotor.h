//=========================================================
//
//     FILE : Dev_DCMotor.h
//
//  PROJECT : Any (SMAC Framework)
//
//   AUTHOR : Bill Daniels
//            Copyright 2024, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================
//
//  This class assumes you are using a standard DC motor driver with two signal pins
//  specified in the constructor.  Motor direction is specified as clockwise (CW) or
//  counter-clockwise (CCW).  Motor speed is specified as a percentage (0 - 100)%.
//
//  All control can be performed either by directly calling methods or by executing
//  a command string, from a UI for example.
//
//  Your app should normally wait until the motor is finished with a previous motion command
//  before issuing a new command.  If a motion command is called while the motor is already
//  running, then the current motion is interrupted and the new motion begins from the
//  motor's current position.
//
//  This class overrides the DoImmediate() and ExecuteCommand() methods of the base Device class.
//  The ExecuteCommand() method handles the following Device commands:
//
//    SRMP|r = SET RAMP     - Sets the speed ramping factor for smooth motor start and stop
//    GOFW|s = GO FORWARD   - Moving forward, ramp the motor up/down to the specified speed
//    GOBW|s = GO BACKWARD  - Moving backward, ramp the motor up/down to the specified speed
//    STOP   = STOP         - Ramp the motor down until it stops
//    ESTP   = E-STOP       - Stops the motor immediately
//    GSTA   = GET STATE    - Get the current state of the motor (see the DCMotorState enum)
//
//    where:
//      r is the ramp rate (0-9)
//      s is speed (0-100)%
//
//=============================================================================

#ifndef DEV_DCMOTOR_H
#define DEV_DCMOTOR_H

//--- Includes --------------------------------------------

#include "Device.h"

//--- Enums -----------------------------------------------

enum DCMotor_State
{
  DCMOTOR_STOPPED,
  DCMOTOR_RAMPING_UP,
  DCMOTOR_AT_SPEED,
  DCMOTOR_RAMPING_DOWN
};

enum DCMotor_Direction
{
  DCMOTOR_CW,  // clockwise
  DCMOTOR_CCW  // counter-clockwise
};

enum DCMotor_RunReturn
{
  DCMOTOR_OKAY,                // Idle or still running
  DCMOTOR_LIMIT_SWITCH_LOWER,  // Lower limit switch triggered
  DCMOTOR_LIMIT_SWITCH_UPPER   // Upper limit switch triggered
};


//=========================================================
//  class Dev_DCMotor
//=========================================================

class Dev_DCMotor : public Device
{
  private:
    int  pwmPin1, pwmPin2;          // PWM signal pins
    int  llSwitchPin, ulSwitchPin;  // Limit switch pins

    DCMotor_State      state            = DCMOTOR_STOPPED;  // Current motor state
    DCMotor_Direction  currentDirection = DCMOTOR_CW;       // Currently running clockwise or counter-clockwise
    int                currentPWM       = 0;                // Current speed (PWM value)
    int                targetPWM        = 0;                // Upper speed to run motor to be AT_SPEED (PWM value)
    int                pwmIncrement     = 1;                // +1 (ramping up) or -1 (ramping down)
    unsigned long      rampPeriod       = 10000L;           // Delay time for ramping speed up/down (rate of acceleration)
    unsigned long      nextPWMMicros;                       // Target micros for next speed increment
    unsigned long      now;

  public:
    Dev_DCMotor (const char *inName, int PWMPin1, int PWMPin2, int LLSwitchPin=0, int ULSwitchPin=0);

    DCMotor_RunReturn  Run ();  // Keeps the motor running (must be called from your loop() function with no delay)

    void           SetRamp  (int ramp);                          // Sets the run time for each ramping speed (0 - 9)
    void           Go       (DCMotor_Direction dir, int speed);  // Ramp up to speed
    void           Stop     ();                                  // Ramp down to stop
    void           EStop    ();                                  // Immediately stops motor
    DCMotor_State  GetState ();                                  // Returns current state of motor

    //--- Overrides ---
    ProcessStatus  DoImmediate    ();  // Device class override
    ProcessStatus  ExecuteCommand (char *command, char *params);  // Device class override
};

#endif
