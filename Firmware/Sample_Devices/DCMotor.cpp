//=========================================================
//
//     FILE : DCNotor.cpp
//
//  PROJECT : Any (SMAC Framework)
//
//   AUTHOR : Bill Daniels
//            Copyright 2024, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

//--- Includes --------------------------------------------

#include <Arduino.h>
#include <string.h>
#include "DCMotor.h"

//--- Constructor ------------------------------------------

DCMotor::DCMotor (const char *inName, int PWMPin1, int PWMPin2, int LLSwitchPin, int ULSwitchPin)
        :Device (inName)
{
  // No Periodic Processing
  periodicEnabled = false;

  // Save the specified pins for the both driver types
  pwmPin1     = PWMPin1;
  pwmPin2     = PWMPin2;
  llSwitchPin = LLSwitchPin;
  ulSwitchPin = ULSwitchPin;

  // Set pin modes
  pinMode (pwmPin1, OUTPUT);
  pinMode (pwmPin2, OUTPUT);

  if (llSwitchPin != 0)
    pinMode (llSwitchPin, INPUT_PULLUP);

  if (ulSwitchPin != 0)
    pinMode (ulSwitchPin, INPUT_PULLUP);

  // Maker sure motor is stopped
  analogWrite (pwmPin1, 0);
  analogWrite (pwmPin2, 0);
  state = DCMOTOR_STOPPED;

  // Default ramping
  SetRamp (4);

  // Set Version for this Device
  strcpy (version, "2025.07.21a");  // no more than 11 chars
}

//--- Run -------------------------------------------------
// Must be called by DoImmediate()

DCMotor_RunReturn DCMotor::Run ()
{
  // Is the motor RUNNING
  if (state != DCMOTOR_STOPPED)
  {
    // Check limit switches, if specified
    if ((llSwitchPin != 0) && digitalRead (llSwitchPin) == LOW)
    {
      // Lower limit switch triggered
      EStop ();
      return DCMOTOR_LIMIT_SWITCH_LOWER;
    }

    if ((ulSwitchPin != 0) && digitalRead (ulSwitchPin) == LOW)
    {
      // Upper limit switch triggered
      EStop ();
      return DCMOTOR_LIMIT_SWITCH_UPPER;
    }

    // Is motor still ramping up/down?
    if (state != DCMOTOR_AT_SPEED)
    {
      // Is it time to adjust speed?
      now = micros ();
      if (now >= nextPWMMicros)
      {
        // Adjust speed
        currentPWM += pwmIncrement;  // +1 for ramping up, -1 for ramping down

        if (currentDirection == DCMOTOR_CW)
          analogWrite (pwmPin2, currentPWM);
        else
          analogWrite (pwmPin1, currentPWM);

        // Has the motor reached target speed?
        if (currentPWM == targetPWM)
        {
          if (targetPWM == 0)
            state = DCMOTOR_STOPPED;
          else
            state = DCMOTOR_AT_SPEED;
        }
        else
          // Still ramping, set new speed
          nextPWMMicros = now + rampPeriod;
      }
    }
  }

  return DCMOTOR_OKAY;
}

//--- SetRamp ---------------------------------------------

void DCMotor::SetRamp (int ramp)
{
  // Must be 0..9
  if (ramp >= 0 && ramp <= 9)
  {
    // Set run time for each ramping speed in microseconds
    rampPeriod = 2000L * ramp + 2000L;
  }
}

//--- Go --------------------------------------------------

void DCMotor::Go (DCMotor_Direction dir, int speed)
{
  // We need to handle this command from any MotorState.
  // If STOPPED, we just ramp up to speed.
  // If running in the same direction, we alter our speed
  //   and either ramp up or down to the new speed.
  // If running in the opposite direction, we need to
  //   ramp down to a stop, then ramp back up to speed
  //   in the opposite direction.

  // Check speed param
  if      (speed > 100) speed = 100;
  else if (speed <   0) speed = 0;

  if (state != DCMOTOR_STOPPED)
  {
    //--- Already running ---
    if (dir == currentDirection)
    {
      //--- Same direction ---
      targetPWM = speed * 250 / 100;  // new target speed

      // Do we need to ramp up or down to the new speed?
      if (targetPWM > currentPWM)
      {
        pwmIncrement = 1;    // ramp up
        state = DCMOTOR_RAMPING_UP;
      }
      else if (targetPWM < currentPWM)
      {
        pwmIncrement = -1;    // ramp down
        state = DCMOTOR_RAMPING_DOWN;
      }

      return;  // Ignore same speed request
    }
    else
    {
      //--- Reverse direction ---
      Stop ();
      while (state != DCMOTOR_STOPPED) Run ();  // Wait for motor to stop

      // Fall into code below to begin new motion
    }
  }

  if (speed != 0)
  {
    // Begin ramping up to speed from STOPPED:
    // Speed is specified as a percentage (0-100) of max pulse width value of 250
    // 50% speed would be a pulse width of 125

    // Set direction
    currentDirection = dir;
    if (currentDirection == DCMOTOR_CW)
      digitalWrite (pwmPin1, LOW);  // pwmPin2 will be the PWM pin
    else
      digitalWrite (pwmPin2, LOW);  // pwmPin1 will be the PWM pin

    currentPWM   = 0;                  // starting from stopped
    targetPWM    = speed * 250 / 100;  // pwm is % of 250
    pwmIncrement = 1;                  // ramping up

    // Begin motion
    nextPWMMicros = micros ();
    state = DCMOTOR_RAMPING_UP;
  }
}

//--- Stop ------------------------------------------------

void DCMotor::Stop ()
{
  if (state != DCMOTOR_STOPPED && currentPWM != 0)
  {
    // Begin ramping down to stop:
    targetPWM    = 0;
    pwmIncrement = -1;  // ramping down

    // Begin motion
    nextPWMMicros = micros ();
    state = DCMOTOR_RAMPING_DOWN;
  }
}

//--- EStop -----------------------------------------------

void DCMotor::EStop ()
{
  // Emergency Stop
	analogWrite (pwmPin1, 0);
	analogWrite (pwmPin2, 0);

  state = DCMOTOR_STOPPED;
}

//--- GetState --------------------------------------------

DCMotor_State DCMotor::GetState ()
{
  // Return current motor state
  return state;
}


//=========================================================
//  Device class overrides
//=========================================================

//--- DoImmediate -----------------------------------------

ProcessStatus  DCMotor::DoImmediate ()
{
  Run ();
  return SUCCESS_NODATA;
}

//--- ExecuteCommand --------------------------------------

ProcessStatus DCMotor::ExecuteCommand ()
{
  // Command info is held in the global <CommandPacket> structure.
  // This method is only called for commands targeted for this device.

  // First call the base class ExecuteCommand method
  pStatus = Device::ExecuteCommand ();

  // Check if command was handled by the base class
  if (pStatus == NOT_HANDLED)
  {
    // The command was NOT handled by the base class, so handle custom commands.
    // We only need to look at CommandPacket.command and CommandPacket.params fields.

    //--- E-Stop ---
    if (strcmp (CommandPacket.command, "ESTP") == 0)
    {
      EStop ();
      pStatus = SUCCESS_NODATA;
    }

    //--- Set Ramp ---
    else if (strcmp (CommandPacket.command, "SRMP") == 0)
    {
      // Check for value
      if (strlen (CommandPacket.params) < 1)
      {
        strcpy (DataPacket.value, "Missing ramp value 0-9");
        pStatus = FAIL_DATA;
      }
      else
      {
        int ramp = atoi (CommandPacket.params);

        // Check specified ramp value
        if (ramp >= 0 && ramp <= 9)
          SetRamp (ramp);

        pStatus = SUCCESS_NODATA;
      }
    }

    //--- GO Forward/Backward ---
    else if (strncmp (CommandPacket.command, "GO", 2) == 0)
    {
      // Check for speed
      if (strlen (CommandPacket.params) > 0)
      {
        Go (CommandPacket.command[2] == 'F' ? DCMOTOR_CW : DCMOTOR_CCW, atoi (CommandPacket.params));
        pStatus = SUCCESS_NODATA;
      }
      else
      {
        strcpy (DataPacket.value, "Missing speed");
        pStatus = FAIL_DATA;
      }
    }

    //--- STOP ---
    else if (strcmp (CommandPacket.command, "STOP") == 0)
    {
      Stop ();
      pStatus = SUCCESS_NODATA;
    }

    //--- GET STATE ---
    else if (strcmp (CommandPacket.command, "GSTA") == 0)
    {
      switch (state)
      {
        case DCMOTOR_STOPPED      : strcpy (DataPacket.value, "ST"); break;
        case DCMOTOR_RAMPING_UP   : strcpy (DataPacket.value, "RU"); break;
        case DCMOTOR_RAMPING_DOWN : strcpy (DataPacket.value, "RD"); break;
        case DCMOTOR_AT_SPEED     : strcpy (DataPacket.value, "AS"); break;

        default : strcpy (DataPacket.value, "??"); break;
      }

      pStatus = SUCCESS_DATA;
    }

    else
    {
      strcpy (DataPacket.value, "Unknown command");
      pStatus = SUCCESS_DATA;
    }
  }

  return pStatus;
}
