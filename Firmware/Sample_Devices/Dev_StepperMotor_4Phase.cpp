//=========================================================
//
//   FILE   : Dev_StepperMotor_4Phase.cpp
//
//  PROJECT : Any (SMAC Framework)
//
//    NOTES : This firmware is used to operate a single 4-phase (5-wire) stepper driver/motor
//            from an MCU development board such as an Arduino or ESP32.
//            It incorporates both soft and hard limits (by specifying limit switch pins).
//
//   AUTHOR : Bill Daniels (bill@dstechlabs.com)
//            See LICENSE.md
//
//=========================================================

#include <Arduino.h>
#include <string.h>
#include "Dev_StepperMotor_4Phase.h"

//=========================================================
//  Constructor
//=========================================================
Dev_StepperMotor_4Phase::Dev_StepperMotor_4Phase (const char *inName, int inDrivePin1, int inDrivePin2, int inDrivePin3, int inDrivePin4, int llSwitchPin, int ulSwitchPin)
                        :Device (inName)
{
  // No Periodic Processing needed
  periodicEnabled = false;

  // Save the specified driver and limit switch pins
  DrivePin1   = inDrivePin1;
  DrivePin2   = inDrivePin2;
  DrivePin3   = inDrivePin3;
  DrivePin4   = inDrivePin4;
  LLSwitchPin = llSwitchPin;
  ULSwitchPin = ulSwitchPin;

  // Set pin modes
  pinMode (DrivePin1, OUTPUT);
  pinMode (DrivePin2, OUTPUT);
  pinMode (DrivePin3, OUTPUT);
  pinMode (DrivePin4, OUTPUT);

  if (LLSwitchPin >= 0)
    pinMode (LLSwitchPin, INPUT_PULLUP);

  if (ULSwitchPin >= 0)
    pinMode (ULSwitchPin, INPUT_PULLUP);

  // Initialize pins to be disengaged
  digitalWrite (DrivePin1, LOW);
  digitalWrite (DrivePin2, LOW);
  digitalWrite (DrivePin3, LOW);
  digitalWrite (DrivePin4, LOW);

	// Set initial motor state and step position/timing
	Homed             = false;
  State             = STEPPER4_OKAY;
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

  // Set Version for this Device
  strcpy (version, "1.0.0");  // no more than 9 chars
}

//--- Run -------------------------------------------------

Stepper4_RunReturn Dev_StepperMotor_4Phase::Run ()
{
  // Is the motor RUNNING and is it time for it to step
  if (Homed && (State == STEPPER4_RUNNING) && (micros() >= NextStepMicros))
  {
    // Is the motor at the target position?
    if (AbsolutePosition == TargetPosition)
    {
      // Yes, stop motor and indicate completion
      State = STEPPER4_ENABLED;
      return STEPPER4_RUN_COMPLETE;
    }

    // No, so continue motion
    NextPosition = AbsolutePosition + StepIncrement;  // +1 for clockwise rotations, -1 for counter-clockwise

    // Check next position against range limits
    // If out of range, stop motor and return range error
    if (NextPosition < LowerLimit)
    {
      State = STEPPER4_ENABLED;
      return STEPPER4_RANGE_ERROR_LOWER;
    }

    if (NextPosition > UpperLimit)
    {
      State = STEPPER4_ENABLED;
      return STEPPER4_RANGE_ERROR_UPPER;
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
      State = STEPPER4_ENABLED;
      return STEPPER4_LIMIT_SWITCH_LOWER;
    }

    if ((ULSwitchPin >= 0) && digitalRead (ULSwitchPin) == LOW)
    {
      // Upper limit switch triggered
      State = STEPPER4_ENABLED;
      return STEPPER4_LIMIT_SWITCH_UPPER;
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

  return STEPPER4_OKAY;
}

//--- startRotation ---------------------------------------

void Dev_StepperMotor_4Phase::startRotation ()
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
  State          = STEPPER4_RUNNING;
}

//--- doStep ----------------------------------------------

void Dev_StepperMotor_4Phase::doStep ()
{
  // Perform a single step according to current phase
  // Half stepping with two-coils for extra torque
  switch (Phase)
  {
    case 0 : digitalWrite (DrivePin1, HIGH);
             digitalWrite (DrivePin2, HIGH);
             digitalWrite (DrivePin3, LOW );
             digitalWrite (DrivePin4, LOW );
             break;

    case 1 : digitalWrite (DrivePin1, LOW );
             digitalWrite (DrivePin2, HIGH);
             digitalWrite (DrivePin3, HIGH);
             digitalWrite (DrivePin4, LOW );
             break;

    case 2 : digitalWrite (DrivePin1, LOW );
             digitalWrite (DrivePin2, LOW );
             digitalWrite (DrivePin3, HIGH);
             digitalWrite (DrivePin4, HIGH);
             break;

    case 3 : digitalWrite (DrivePin1, HIGH);
             digitalWrite (DrivePin2, LOW );
             digitalWrite (DrivePin3, LOW );
             digitalWrite (DrivePin4, HIGH);
             break;
  }

  // Adjust phase depending on direction
  if (Direction == ROT_CLOCKWISE) ++Phase;
  else --Phase;

  if (Phase > 3) Phase = 0;
  else if (Phase < 0) Phase = 3;
}

//--- Enable ----------------------------------------------

void Dev_StepperMotor_4Phase::Enable ()
{
  // Enable motor driver
  State = STEPPER4_ENABLED;
  doStep ();

  // Also, set the current position as HOME
  SetHomePosition ();
}

//--- Disable ---------------------------------------------

void Dev_StepperMotor_4Phase::Disable ()
{
  // Disable all coils
  digitalWrite (DrivePin1, LOW);
  digitalWrite (DrivePin2, LOW);
  digitalWrite (DrivePin3, LOW);
  digitalWrite (DrivePin4, LOW);

  State = STEPPER4_DISABLED;
  Homed = false;  // When motor is free to move, the HOME position is lost
}

//--- FindHome --------------------------------------------

void Dev_StepperMotor_4Phase::FindHome ()
{
  // Seek counter-clockwise to lower limit switch
  // Then back-off until switch releases and set
  // new HOME position

  if (LLSwitchPin >= 0)
  {
    Enable ();

    // Find limit switch
    Direction = ROT_COUNTER_CLOCKWISE;
    while (digitalRead (LLSwitchPin) == HIGH)
    {
      doStep ();
      delay (5L);
    }

    // Back off slowly
    Direction = ROT_CLOCKWISE;
    while (digitalRead (LLSwitchPin) == LOW)
    {
      doStep ();
      delay (50L);
    }

    // A few more steps
    for (int i=0; i<3; i++)
      doStep ();

    SetHomePosition ();
  }
}

//--- SetHomePosition -------------------------------------

void Dev_StepperMotor_4Phase::SetHomePosition ()
{
  // Only if Enabled
  if (State == STEPPER4_ENABLED)
  {
    // Set current position as HOME position
    AbsolutePosition = 0L;
    DeltaPosition    = 0L;
    Homed            = true;
  }
}

//--- SetLowerLimit ---------------------------------------

void Dev_StepperMotor_4Phase::SetLowerLimit (long lowerLimit)
{
  // Must be <= 0 and <= UpperLimit
  if (lowerLimit <= 0 && lowerLimit <= UpperLimit)
    LowerLimit = lowerLimit;
}

//--- SetUpperLimit ---------------------------------------

void Dev_StepperMotor_4Phase::SetUpperLimit (long upperLimit)
{
  // Must be >= 0 and >= LowerLimit
  if (upperLimit >= 0 && upperLimit >= LowerLimit)
    UpperLimit = upperLimit;
}

//--- SetRamp ---------------------------------------------

void Dev_StepperMotor_4Phase::SetRamp (int ramp)
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

void Dev_StepperMotor_4Phase::RotateAbsolute (long newPosition, int stepsPerSecond)
{
  TargetPosition = newPosition;  // Set Absolute Position
  MaxVelocity    = stepsPerSecond;
  TotalSteps     = abs(TargetPosition - AbsolutePosition);

  startRotation ();
}

//---= RotateRelative -------------------------------------

void Dev_StepperMotor_4Phase::RotateRelative (long numSteps, int stepsPerSecond)
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

void Dev_StepperMotor_4Phase::RotateToHome ()
{
  MaxVelocity    = HOMING_SPEED;
  TargetPosition = 0L;  // HOME position
  TotalSteps     = abs(AbsolutePosition);

  startRotation ();
}

//--- RotateToLowerLimit ----------------------------------

void Dev_StepperMotor_4Phase::RotateToLowerLimit ()
{
  MaxVelocity    = HOMING_SPEED;
  TargetPosition = LowerLimit;
  TotalSteps     = abs(AbsolutePosition - LowerLimit);

  startRotation ();
}

//--- RototaeToUpperLimit ---------------------------------

void Dev_StepperMotor_4Phase::RotateToUpperLimit ()
{
  MaxVelocity    = HOMING_SPEED;
  TargetPosition = UpperLimit;
  TotalSteps     = abs(AbsolutePosition - UpperLimit);

  startRotation ();
}

//--- EStop -----------------------------------------------

void Dev_StepperMotor_4Phase::EStop ()
{
  // Emergency Stop
  // ((( Requires Re-Enable of motor )))
	Disable ();

  State = STEPPER4_ESTOPPED;
  Homed = false;
  TargetPosition = AbsolutePosition;
}

//--- IsHomed ---------------------------------------------

bool Dev_StepperMotor_4Phase::IsHomed ()
{
  // Return homed state
  return Homed;
}

//--- GetState --------------------------------------------

Stepper4_State Dev_StepperMotor_4Phase::GetState ()
{
  // Return current motor state
  return State;
}

//--- GetAbsolutePosition ---------------------------------

long Dev_StepperMotor_4Phase::GetAbsolutePosition ()
{
  // Return current position from HOME
  return AbsolutePosition;
}

//--- GetRelativePosition ---------------------------------

long Dev_StepperMotor_4Phase::GetRelativePosition ()
{
  // Return number of steps moved from last position
  return DeltaPosition;
}

//--- GetLowerLimit ------------------------------------===

long Dev_StepperMotor_4Phase::GetLowerLimit ()
{
  return LowerLimit;
}

//--- GetUpperLimit ---------------------------------------

long Dev_StepperMotor_4Phase::GetUpperLimit ()
{
  return UpperLimit;
}

//--- GetVersion ------------------------------------------

const char * Dev_StepperMotor_4Phase::GetVersion ()
{
  return Version;
}


//=========================================================
//  Device class overrides
//=========================================================

//--- DoImmediate -----------------------------------------

ProcessStatus Dev_StepperMotor_4Phase::DoImmediate ()
{
  switch (Run ())
  {
    case STEPPER4_OKAY :
      pStatus = SUCCESS_NODATA;
      break;

    case STEPPER4_RUN_COMPLETE       :   // Motion complete, reached target position normally
      sprintf (SMACData.values, "RC%d", AbsolutePosition);
      pStatus = SUCCESS_DATA;
      break;

    case STEPPER4_RANGE_ERROR_LOWER  :   // Reached lower range limit
    case STEPPER4_RANGE_ERROR_UPPER  :   // Reached upper range limit
      strcpy (SMACData.values, "RE");
      pStatus = FAIL_DATA;
      break;

    case STEPPER4_LIMIT_SWITCH_LOWER :   // Lower limit switch triggered
    case STEPPER4_LIMIT_SWITCH_UPPER :   // Upper limit switch triggered
      strcpy (SMACData.values, "LS");
      pStatus = FAIL_DATA;
  }

  return pStatus;
}

//--- ExecuteCommand --------------------------------------

ProcessStatus Dev_StepperMotor_4Phase::ExecuteCommand (char *command, char *params)
{
  // This method is only called for commands targeted for this device.

  // First call the base class ExecuteCommand method
  pStatus = Device::ExecuteCommand (command, params);

  // Check if command was handled by the base class
  if (pStatus == NOT_HANDLED)
  {
    // The command was NOT handled by the base class, so handle custom commands.
    // We only need to look at command and params fields.

    // Set default pStatus
    pStatus = SUCCESS_NODATA;

    //--- E-Stop ---
    if (strcmp (command, "STOP") == 0)
      EStop ();

    //--- Enable/Disable ---
    else if (strcmp (command, "ENAB") == 0)
      Enable ();
    else if (strcmp (command, "DISA") == 0)
      Disable ();

    //--- Find Home ---
    else if (strcmp (command, "FHOM") == 0)
      FindHome ();

    //--- Set Home and Lower/Upper Limits ---
    else if (strcmp (command, "SHOM") == 0)
      SetHomePosition ();
    else if (strcmp (command, "SLOW") == 0)
    {
      // Check for value
      if (strlen (params) < 1)
      {
        strcpy (SMACData.values, "Missing lower limit value");
        pStatus = FAIL_DATA;
      }
      else
        SetLowerLimit (atol (params));
    }
    else if (strcmp (command, "SUPP") == 0)
    {
      // Check for value
      if (strlen (params) < 1)
      {
        strcpy (SMACData.values, "Missing upper limit value");
        pStatus = FAIL_DATA;
      }
      else
        SetUpperLimit (atol (params));
    }

    //--- Set Ramp ---
    else if (strcmp (command, "SRMP") == 0)
    {
      // Check for value
      if (strlen (params) < 1)
      {
        strcpy (SMACData.values, "Missing ramp value 0-9");
        pStatus = FAIL_DATA;
      }
      else
        SetRamp ((int)(params[0])-48);
    }

    //--- Rotate commands ---
    else if (strcmp (command, "RABS") == 0 || strcmp (command, "RREL") == 0)
    {
      // Check for speed and target/numSteps value
      // Speed is first four(4) digits of params, (0001 - 9999) steps per second
      if (strlen (params) < 5)
      {
        strcpy (SMACData.values, "Bad rotate command");
        pStatus = FAIL_DATA;
      }
      else
      {
        // Parse speed and target/numSteps
        strncpy (SpeedString, params, 4);
        SpeedString[4] = 0;
        Speed = atoi (SpeedString);
        TargetOrSteps = atol (params + 4);  // Target position or number of steps is remainder of params

        if (strcmp (command, "RABS") == 0)
          RotateAbsolute (TargetOrSteps, Speed);
        else
          RotateRelative (TargetOrSteps, Speed);
      }
    }
    else if (strcmp (command, "RHOM") == 0)
      RotateToHome ();
    else if (strcmp (command, "RLOW") == 0)
      RotateToLowerLimit ();
    else if (strcmp (command, "RUPP") == 0)
      RotateToUpperLimit ();

    else
    {
      //--- Query Commands ---

      // Set default pStatus
      pStatus = SUCCESS_DATA;

      //--- Get State ---
      if (strcmp (command, "GSTA") == 0)
      {
        switch (State)
        {
          case STEPPER4_ENABLED  : strcpy (SMACData.values, "EN"); break;
          case STEPPER4_DISABLED : strcpy (SMACData.values, "DI"); break;
          case STEPPER4_RUNNING  : strcpy (SMACData.values, "RU"); break;
          case STEPPER4_ESTOPPED : strcpy (SMACData.values, "ES"); break;

          default : strcpy (SMACData.values, "ERROR: Invalid State"); break;
        }
      }

      //--- Other query commands ---
      else if (strcmp (command, "GABS") == 0)
        sprintf (SMACData.values, "AP%d", GetAbsolutePosition());
      else if (strcmp (command, "GREL") == 0)
        sprintf (SMACData.values, "RP%d", GetRelativePosition());
      else if (strcmp (command, "GLOW") == 0)
        sprintf (SMACData.values, "LL%d", GetLowerLimit());
      else if (strcmp (command, "GUPP") == 0)
        sprintf (SMACData.values, "UL%d", GetUpperLimit());

      //--- Unknown command ---
      else pStatus = NOT_HANDLED;
    }
  }

  return pStatus;
}
