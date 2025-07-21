//-------------------------------------------------------===
//
//   FILE   : StepperMotor.cpp
//
//  PROJECT : Any (SMAC Framework)
//
//    NOTES : This firmware is used to operate a single digital bipolar stepper driver/motor
//            from an MCU development board such as an Arduino or ESP32.
//            It incorporates both soft and hard limits (by specifying limit switch pins).
//
//   AUTHOR : Bill Daniels (bill@dstechlabs.com)
//            See LICENSE.md
//
//-------------------------------------------------------===

#include <Arduino.h>
#include <string.h>
#include "StepperMotor.h"

//-------------------------------------------------------===
//  Constructor
//-------------------------------------------------------===
StepperMotor::StepperMotor (const char *inName, int enablePin, int directionPin, int stepPin, int llSwitchPin, int ulSwitchPin)
             :Device (inName)
{
  // No Periodic Processing needed
  periodicEnabled = false;

  // Save the specified driver and limit switch pins
  EnablePin    = enablePin;
  DirectionPin = directionPin;
  StepPin      = stepPin;
  LLSwitchPin  = llSwitchPin;
  ULSwitchPin  = ulSwitchPin;

  // Set pin modes
  pinMode (EnablePin   , OUTPUT);
  pinMode (DirectionPin, OUTPUT);
  pinMode (StepPin     , OUTPUT);

  if (LLSwitchPin >= 0)
    pinMode (LLSwitchPin, INPUT_PULLUP);

  if (ULSwitchPin >= 0)
    pinMode (ULSwitchPin, INPUT_PULLUP);

  // Initialize pins
  digitalWrite (EnablePin   , HIGH);  // HIGH = Off (disabled)
  digitalWrite (DirectionPin, LOW);
  digitalWrite (StepPin     , LOW);

	// Set initial motor state and step position/timing
	Homed             = false;
  State             = STEPPER_DISABLED;
  StepIncrement     = 1L;
  AbsolutePosition  = 0L;
  DeltaPosition     = 0L;
  TargetPosition    = 0L;
  LowerLimit        = -2000000000L;
  UpperLimit        =  2000000000L;
  RampSteps         = 0L;
  RampDownStep      = 0L;
  Velocity          = 0L;
  VelocityIncrement = RampScale * 5L;  // Default ramp scale of 5
  NextPosition      = 0L;
  NextStepMicros    = -1L;
  MaxVelocity       = 0;
  TargetOrSteps     = 0;
  TotalSteps        = 0;
  StepCount         = 0;

  // Set version
  strcpy (version, "2025.07.15b");  // no more than 11 chars
}

//--- Run -------------------------------------------------

Stepper_RunReturn StepperMotor::Run ()
{
  // Is the motor RUNNING and is it time for it to step
  if (Homed && (State == STEPPER_RUNNING) && (micros() >= NextStepMicros))
  {
    // Is the motor at the target position?
    if (AbsolutePosition == TargetPosition)
    {
      // Yes, stop motor and indicate completion
      State = STEPPER_ENABLED;
      return STEPPER_RUN_COMPLETE;
    }

    // No, so continue motion
    NextPosition = AbsolutePosition + StepIncrement;  // +1 for clockwise rotations, -1 for counter-clockwise

    // Check next position against range limits
    // If out of range, stop motor and return range error
    if (NextPosition < LowerLimit)
    {
      State = STEPPER_ENABLED;
      return STEPPER_RANGE_ERROR_LOWER;
    }

    if (NextPosition > UpperLimit)
    {
      State = STEPPER_ENABLED;
      return STEPPER_RANGE_ERROR_UPPER;
    }

    // Perform a single step
    doStep ();

    // Set current position
    AbsolutePosition = NextPosition;
    DeltaPosition   += StepIncrement;

    // Check limit switches, if specified
    if ((LLSwitchPin >= 0) && digitalRead (LLSwitchPin) == LOW)
    {
      // Lower limit switch triggered
      State = STEPPER_ENABLED;
      return STEPPER_LIMIT_SWITCH_LOWER;
    }

    if ((ULSwitchPin >= 0) && digitalRead (ULSwitchPin) == LOW)
    {
      // Upper limit switch triggered
      State = STEPPER_ENABLED;
      return STEPPER_LIMIT_SWITCH_UPPER;
    }

    // Adjust velocity if ramping
    StepCount = abs(DeltaPosition);
    if (StepCount <= RampSteps)
    {
      // Ramping up
      Velocity += VelocityIncrement;
    }
    else if (StepCount > RampDownStep)
    {
      // Ramping down
      Velocity -= VelocityIncrement;
    }

    // Set time for next step
    if (Velocity > 0L)
      NextStepMicros += 1000000L / Velocity;
  }

  return STEPPER_OKAY;
}

//--- startRotation ---------------------------------------

void StepperMotor::startRotation ()
{
  // Determine number of steps in ramp and set starting speed
  if (VelocityIncrement == 0L)
  {
    // Immediate full speed, no ramping
    RampSteps = 0L;
    Velocity  = MaxVelocity;  // Start at full velocity
  }
  else
  {
    // Ramp up
    RampSteps = MaxVelocity / VelocityIncrement;
    if (RampSteps == 0L)
      Velocity = MaxVelocity;  // Start at slow value
    else
      Velocity = 0L;  // Start from a stand-still
  }

  // Set on what step to start ramping down
  if (TotalSteps > 2L * RampSteps)
    RampDownStep = TotalSteps - RampSteps;  // Normal trapezoid velocity
  else
    RampDownStep = RampSteps = TotalSteps / 2L;  // Stunted triangle velocity

  // Set Direction
  if (TargetPosition >= AbsolutePosition)
  {
    StepIncrement = 1L;
    digitalWrite (DirectionPin, LOW);
  }
  else if (TargetPosition < AbsolutePosition)
  {
    StepIncrement = -1L;
    digitalWrite (DirectionPin, HIGH);
  }

  // Start rotation
  DeltaPosition  = 0L;
  NextStepMicros = micros() + 10L;  // Direction must be set 10-microseconds before stepping
  State          = STEPPER_RUNNING;
}

//--- doStep ----------------------------------------------

void StepperMotor::doStep ()
{
  // Perform a single step
  digitalWrite (StepPin, HIGH);
  delayMicroseconds (PULSE_WIDTH);
  digitalWrite (StepPin, LOW);


  // // For fast stepping on the Arduino NANO:
  // PORTD |= B00010000;   // Turn on Pulse pin (Arduino NANO Digital Pin 4)
  // delayMicroseconds (PULSE_WIDTH);
  // PORTD &= ~B00010000;  // Turn off Pulse pin


}

//--- Enable ----------------------------------------------

void StepperMotor::Enable ()
{
  // Enable motor driver
  digitalWrite (EnablePin, LOW);
  State = STEPPER_ENABLED;

  // Also, set the current position as HOME
  SetHomePosition ();
}

//--- Disable ---------------------------------------------

void StepperMotor::Disable ()
{
  // Disable motor driver
  digitalWrite (EnablePin, HIGH);

  State = STEPPER_DISABLED;
  Homed = false;  // When motor is free to move, the HOME position is lost
}

//--- FindHome --------------------------------------------

void StepperMotor::FindHome ()
{
  // Seek counter-clockwise to lower limit switch
  // Then back-off until switch releases and set
  // new HOME position

  if (LLSwitchPin >= 0)
  {
    Enable ();

    // Find limit switch
    digitalWrite (DirectionPin, HIGH);
    while (digitalRead (LLSwitchPin) == HIGH)
    {
      doStep ();
      delay (5L);
    }

    // Back off slowly
    digitalWrite (DirectionPin, LOW);
    while (digitalRead (LLSwitchPin) == LOW)
    {
      doStep ();
      delay (50L);
    }

    // A few more steps
    for (int i=0; i<10; i++)
      doStep ();

    SetHomePosition ();
  }
}

//--- SetHomePosition -------------------------------------

void StepperMotor::SetHomePosition ()
{
  // Only if Enabled
  if (State == STEPPER_ENABLED)
  {
    // Set current position as HOME position
    AbsolutePosition = 0L;
    DeltaPosition    = 0L;
    Homed            = true;
  }
}

//--- SetLowerLimit ---------------------------------------

void StepperMotor::SetLowerLimit (long lowerLimit)
{
  // Must be <= 0 and <= UpperLimit
  if (lowerLimit <= 0 && lowerLimit <= UpperLimit)
    LowerLimit = lowerLimit;
}

//--- SetUpperLimit ---------------------------------------

void StepperMotor::SetUpperLimit (long upperLimit)
{
  // Must be >= 0 and >= LowerLimit
  if (upperLimit >= 0 && upperLimit >= LowerLimit)
    UpperLimit = upperLimit;
}

//--- SetRamp ---------------------------------------------

void StepperMotor::SetRamp (int ramp)
{
  // Must be 0..9
  if (ramp >= 0 && ramp <= 9)
  {
    // Set velocity slope (increment)
    if (ramp == 0)
      VelocityIncrement = 0L;  // constant full velocity
    else
      VelocityIncrement = RampScale * (10L - (long)ramp);
  }
}

//--- RotateAbsolute --------------------------------------

void StepperMotor::RotateAbsolute (long newPosition, int stepsPerSecond)
{
  TargetPosition = newPosition;  // Set Absolute Position
  MaxVelocity    = stepsPerSecond;
  TotalSteps     = abs(TargetPosition - AbsolutePosition);

  startRotation ();
}

//---= RotateRelative -------------------------------------

void StepperMotor::RotateRelative (long numSteps, int stepsPerSecond)
{
  // If numSteps is positive (> 0) then motor rotates clockwise, else counter-clockwise
  if (numSteps != 0)
  {
    TargetPosition = AbsolutePosition + numSteps;
    MaxVelocity    = stepsPerSecond;
    TotalSteps     = abs(numSteps);

    startRotation ();
  }
}

//--- RotateToHome ----------------------------------------

void StepperMotor::RotateToHome ()
{
  MaxVelocity    = HOMING_SPEED;
  TargetPosition = 0L;  // HOME position
  TotalSteps     = abs(AbsolutePosition);

  startRotation ();
}

//--- RotateToLowerLimit ----------------------------------

void StepperMotor::RotateToLowerLimit ()
{
  MaxVelocity    = HOMING_SPEED;
  TargetPosition = LowerLimit;
  TotalSteps     = abs(AbsolutePosition - LowerLimit);

  startRotation ();
}

//--- RototaeToUpperLimit ---------------------------------

void StepperMotor::RotateToUpperLimit ()
{
  MaxVelocity    = HOMING_SPEED;
  TargetPosition = UpperLimit;
  TotalSteps     = abs(AbsolutePosition - UpperLimit);

  startRotation ();
}

//--- EStop -----------------------------------------------

void StepperMotor::EStop ()
{
  // Emergency Stop
  // ((( Requires Re-Enable of motor )))
	digitalWrite (StepPin  , LOW );   // Pulse Off
	digitalWrite (EnablePin, HIGH);   // Disengage

  State = STEPPER_ESTOPPED;
  Homed = false;
  TargetPosition = AbsolutePosition;
}

//--- IsHomed ---------------------------------------------

bool StepperMotor::IsHomed ()
{
  // Return homed state
  return Homed;
}

//--- GetState --------------------------------------------

Stepper_State StepperMotor::GetState ()
{
  // Return current motor state
  return State;
}

//--- GetAbsolutePosition ---------------------------------

long StepperMotor::GetAbsolutePosition ()
{
  // Return current position from HOME
  return AbsolutePosition;
}

//--- GetRelativePosition ---------------------------------

long StepperMotor::GetRelativePosition ()
{
  // Return number of steps moved from last position
  return DeltaPosition;
}

//--- GetLowerLimit ------------------------------------===

long StepperMotor::GetLowerLimit ()
{
  return LowerLimit;
}

//--- GetUpperLimit ---------------------------------------

long StepperMotor::GetUpperLimit ()
{
  return UpperLimit;
}

//--- GetVersion ------------------------------------------

const char * StepperMotor::GetVersion ()
{
  return Version;
}


//=========================================================
//  Device class overrides
//=========================================================

//--- DoImmediate -----------------------------------------

ProcessStatus StepperMotor::DoImmediate ()
{
  switch (Run ())
  {
    case STEPPER_OKAY :
      pStatus = SUCCESS_NODATA;
      break;

    case STEPPER_RUN_COMPLETE       :   // Motion complete, reached target position normally
      sprintf (DataPacket.value, "RC%d", AbsolutePosition);
      pStatus = SUCCESS_DATA;
      break;

    case STEPPER_RANGE_ERROR_LOWER  :   // Reached lower range limit
    case STEPPER_RANGE_ERROR_UPPER  :   // Reached upper range limit
      strcpy (DataPacket.value, "RE");
      pStatus = FAIL_DATA;
      break;

    case STEPPER_LIMIT_SWITCH_LOWER :   // Lower limit switch triggered
    case STEPPER_LIMIT_SWITCH_UPPER :   // Upper limit switch triggered
      strcpy (DataPacket.value, "LS");
      pStatus = FAIL_DATA;
  }

  return pStatus;
}

//--- ExecuteCommand --------------------------------------

ProcessStatus StepperMotor::ExecuteCommand ()
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

    // Set default pStatus
    pStatus = SUCCESS_NODATA;

    //--- E-Stop ---
    if (strcmp (CommandPacket.command, "STOP") == 0)
      EStop ();

    //--- Enable/Disable ---
    else if (strcmp (CommandPacket.command, "ENAB") == 0)
      Enable ();
    else if (strcmp (CommandPacket.command, "DISA") == 0)
      Disable ();

    //--- Find Home ---
    else if (strcmp (CommandPacket.command, "FHOM") == 0)
      FindHome ();

    //--- Set Home and Lower/Upper Limits ---
    else if (strcmp (CommandPacket.command, "SHOM") == 0)
      SetHomePosition ();
    else if (strcmp (CommandPacket.command, "SLOW") == 0)
    {
      // Check for value
      if (strlen (CommandPacket.params) < 1)
      {
        strcpy (DataPacket.value, "Missing lower limit value");
        pStatus = FAIL_DATA;
      }
      else
        SetLowerLimit (atol (CommandPacket.params));
    }
    else if (strcmp (CommandPacket.command, "SUPP") == 0)
    {
      // Check for value
      if (strlen (CommandPacket.params) < 1)
      {
        strcpy (DataPacket.value, "Missing upper limit value");
        pStatus = FAIL_DATA;
      }
      else
        SetUpperLimit (atol (CommandPacket.params));
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
        SetRamp ((int)(CommandPacket.params[0])-48);
    }

    //--- Rotate commands ---
    else if (strcmp (CommandPacket.command, "RABS") == 0 || strcmp (CommandPacket.command, "RREL") == 0)
    {
      // Check for speed and target/numSteps value
      // Speed is first four(4) digits of params, (0001 - 9999) steps per second
      if (strlen (CommandPacket.params) < 5)
      {
        strcpy (DataPacket.value, "Bad rotate command");
        pStatus = FAIL_DATA;
      }
      else
      {
        // Parse speed and target/numSteps
        strncpy (SpeedString, CommandPacket.params, 4);
        SpeedString[4] = 0;
        Speed = atoi (SpeedString);
        TargetOrSteps = atol (CommandPacket.params + 4);  // Target position or number of steps is remainder of params

        if (strcmp (CommandPacket.command, "RABS") == 0)
          RotateAbsolute (TargetOrSteps, Speed);
        else
          RotateRelative (TargetOrSteps, Speed);
      }
    }
    else if (strcmp (CommandPacket.command, "RHOM") == 0)
      RotateToHome ();
    else if (strcmp (CommandPacket.command, "RLOW") == 0)
      RotateToLowerLimit ();
    else if (strcmp (CommandPacket.command, "RUPP") == 0)
      RotateToUpperLimit ();

    else
    {
      //--- Query Commands ---

      // Set default pStatus
      pStatus = SUCCESS_DATA;

      //--- Get State ---
      if (strcmp (CommandPacket.command, "GSTA") == 0)
      {
        switch (State)
        {
          case STEPPER_ENABLED  : strcpy (DataPacket.value, "EN"); break;
          case STEPPER_DISABLED : strcpy (DataPacket.value, "DI"); break;
          case STEPPER_RUNNING  : strcpy (DataPacket.value, "RU"); break;
          case STEPPER_ESTOPPED : strcpy (DataPacket.value, "ES"); break;

          default : strcpy (DataPacket.value, "??"); break;
        }
      }

      //--- Other query commands ---
      else if (strcmp (CommandPacket.command, "GABS") == 0)
        sprintf (DataPacket.value, "AP%d", GetAbsolutePosition());
      else if (strcmp (CommandPacket.command, "GREL") == 0)
        sprintf (DataPacket.value, "RP%d", GetRelativePosition());
      else if (strcmp (CommandPacket.command, "GLOW") == 0)
        sprintf (DataPacket.value, "LL%d", GetLowerLimit());
      else if (strcmp (CommandPacket.command, "GUPP") == 0)
        sprintf (DataPacket.value, "UL%d", GetUpperLimit());

      //--- Unknown command ---
      else
      {
        strcpy (DataPacket.value, "Unknown command");
        pStatus = FAIL_DATA;
      }
    }
  }

  return pStatus;
}
