//==========================================================
//
//   FILE   : ServoMotor.cpp
//
//  PROJECT : Any (SMAC Framework)
//
//   AUTHOR : Bill Daniels
//            Copyright 2024-2025, D+S Tech Labs, Inc.
//            MIT License
//
//==========================================================

#include <Arduino.h>
#include <string.h>
#include "ServoMotor.h"

//----------------------------------------------------------
//  Constructor
//----------------------------------------------------------
ServoMotor::ServoMotor (Adafruit_PWMServoDriver *inServoDriver,
                        int inChannelNum,
                        int inLowerLimitPW, int inLowerLimitAngle,
                        int inUpperLimitPW, int inUpperLimitAngle,
                        int inHomeAngle)
{
  // Save params
  servoDriver     = inServoDriver;
  channelNum      = inChannelNum;
  lowerLimitPW    = inLowerLimitPW;
  lowerLimitAngle = inLowerLimitAngle;
  upperLimitPW    = inUpperLimitPW;
  upperLimitAngle = inUpperLimitAngle;

  // Check params to protect the motor
  if ((lowerLimitPW    >= upperLimitPW) ||
      (lowerLimitAngle >= upperLimitAngle))
  {
    validParams = false;

    Serial.print ("ERROR: Invalid params for motor ");
    Serial.println (channelNum);

    return;
  }

  // Good to go, set conversion factor (angle to pulse width)
  validParams = true;
  angle2PWFactor = (double)(upperLimitPW - lowerLimitPW) / (double)(upperLimitAngle - lowerLimitAngle);

  // Save Home position
  if (inHomeAngle < lowerLimitAngle)
    homeAngle = lowerLimitAngle;
  else if (inHomeAngle > upperLimitAngle)
    homeAngle = upperLimitAngle;
  else
    homeAngle = inHomeAngle;

  // Move motor to its Home position
  servoDriver->setPWM (channelNum, 0, angle2PW (homeAngle));
  absolutePosition = homeAngle;

  // Current state is IDLE;
  state = IDLE;
}

//---------------------------------------------------------
//  Run:
//  Must be continuously called inside your loop function
//  There should be no delay between calls
//
//  Return values:
//    OK  - Idle or still running
//    RCa - Run Complete; final position is aº (1 or more digits)
//    REL - Range Error; tried to exceed lower limit (minimum angle)
//    REU - Range Error; tried to exceed upper limit (maximum angle)
//---------------------------------------------------------
const char * ServoMotor::Run ()
{
  // Is motor RUNNING and it's time for it to step?
  if (validParams && (state == RUNNING) && (micros() >= nextStepMicros))
  {
    // Has motor reached the target position?
    if (absolutePosition == targetPosition)
    {
      // Yes, stop motor and indicate completion
      state = IDLE;
      sprintf (runReturn, "RC%d", absolutePosition);
      return runReturn;
    }

    // No, so continue motion
    nextPosition = absolutePosition + stepIncrement;  // +1º or -1º

    // Check range limits
    // If out of range, stop motor and return range error
    if (nextPosition < lowerLimitAngle)
    {
      state = IDLE;
      strcpy (runReturn, "REL");
      return runReturn;
    }

    if (nextPosition > upperLimitAngle)
    {
      state = IDLE;
      strcpy (runReturn, "REU");
      return runReturn;
    }

    // Perform a single 1º step by adjusting the servo's pulse width
    servoDriver->setPWM (channelNum, 0, angle2PW (nextPosition));

    // Set current position
    absolutePosition = nextPosition;
    deltaPosition   += stepIncrement;

    // Set time for next step
    nextStepMicros += stepPeriod;
  }

  strcpy (runReturn, "OK");
  return runReturn;
}

//--- startRotation ---------------------------------------

void ServoMotor::startRotation ()
{
  // Set Direction
  stepIncrement = targetPosition >= absolutePosition ? 1 : -1;

  // Start rotation
  deltaPosition  = 0L;
  state          = RUNNING;
  nextStepMicros = micros();
}

//--- angle2PW --------------------------------------------

int ServoMotor::angle2PW (int angle)
{
  // Convert angle to pulse width
  return (lowerLimitPW + (int)((double)(angle - lowerLimitAngle) * angle2PWFactor + 0.5));
}

//--- RotateAbsolute --------------------------------------

void ServoMotor::RotateAbsolute (int targetAngle, MotorSpeed inSpeed)
{
  targetPosition = targetAngle;
  stepPeriod = inSpeed;

  startRotation ();
}

//---- RotateRelative -------------------------------------

void ServoMotor::RotateRelative (int angle, MotorSpeed inSpeed)
{
  // Adjust target position by angle
  if (angle != 0)
  {
    targetPosition = absolutePosition + angle;
    stepPeriod = inSpeed;

    startRotation ();
  }
}

//--- RotateHome ------------------------------------------

void ServoMotor::RotateHome ()
{
  targetPosition = homeAngle;
  stepPeriod = MEDIUM;

  startRotation ();
}

//--- EStop -----------------------------------------------

void ServoMotor::EStop ()
{
  // Emergency Stop
  state = IDLE;
}

//--- SetPulseWidth ---------------------------------------

void ServoMotor::SetPulseWidth (int pulseWidth)
{
  // Directly set a pulse width for this channel
  if (pulseWidth >= lowerLimitPW && pulseWidth <= upperLimitPW)
    servoDriver->setPWM (channelNum, 0, pulseWidth);
}

//--- GetState --------------------------------------------

MotorState ServoMotor::GetState ()
{
  // Return current motor state
  return state;
}

//--- GetAbsolutePosition ---------------------------------

long ServoMotor::GetAbsolutePosition ()
{
  // Return current position from HOME
  return absolutePosition;
}

//--- GetRelativePosition ---------------------------------

long ServoMotor::GetRelativePosition ()
{
  // Return number of steps moved from last position
  return deltaPosition;
}

//--- GetVersion ------------------------------------------

const char * ServoMotor::GetVersion ()
{
  return version;
}


//---------------------------------------------------------
//  ExecuteCommand
//---------------------------------------------------------

const char * ServoMotor::ExecuteCommand (const char *packet)
{
  char  command[3];
  int   targetOrAngle;

  // Initialize return string
  runReturn[0] = 0;

  // Command string must be at least 2 chars
  if (strlen(packet) < 2)
  {
    strcpy (runReturn, "Bad command");
    return runReturn;
  }

  // Set 2-Char Command and parse all commands
  strncpy (command, packet, 2);
  command[2] = 0;

  //-------------------------------------------------------
  //  Emergency Stop (ESTOP)
  //  I check this first for quick processing.
  //  When an E-Stop is called, you must re-Enable the
  //  driver for motion to resume.
  //-------------------------------------------------------
  if (strcmp (command, "ES") == 0)
    EStop();

  //-------------------------------------------------------
  //  Rotate Commands
  //-------------------------------------------------------
  else if (strcmp (command, "RA") == 0 || strcmp (command, "RR") == 0)
  {
    // Rotate command must be at least 4 chars
    if (strlen (packet) < 4)
      strcpy (runReturn, "Bad command");
    else
    {
      // Parse speed (S-Slow, M-Medium, F-Fast)
      if (packet[2] == 'M')
        stepPeriod = MEDIUM;
      else if (packet[2] == 'F')
        stepPeriod = FAST;
      else
        stepPeriod = SLOW;

      // Parse target/numSteps
      targetOrAngle = atoi (packet + 3);  // Target position or number of steps is remainder of packet

      if (strcmp (command, "RA") == 0)
        RotateAbsolute (targetOrAngle, stepPeriod);
      else
        RotateRelative (targetOrAngle, stepPeriod);
    }
  }

  //--- Rotate Home ---
  else if (strcmp (command, "RH") == 0)
    RotateHome ();

  //--- Set direct pulse width ---
  else if (strcmp (command, "PW") == 0)
    SetPulseWidth (atoi (packet + 3));

  //-------------------------------------------------------
  //  Query Commands
  //-------------------------------------------------------
  else if (strcmp (command, "GA") == 0)
    sprintf (runReturn, "AP%d%d", channelNum, absolutePosition);
  else if (strcmp (command, "GR") == 0)
    sprintf (runReturn, "RP%d%d", channelNum, deltaPosition);
  else if (strcmp (command, "GV") == 0)
    return version;
  else
    strcpy (runReturn, "Unknown command");

  return runReturn;
}
