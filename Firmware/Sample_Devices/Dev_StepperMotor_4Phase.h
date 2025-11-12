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
//=============================================================================
//
//  This class communicates (by GPIO pins) with a mono-polar micro-stepping motor driver
//  that take four drive wires and one contant voltage wire (5-wires).
//
//  Two extra pins may also be specified for upper and lower limit switches.
//
//─────────────────────────────────────────────────────────────────────────────────────────────────
//
//  This class keeps track of the motor's step count from its HOME position, which is zero(0).
//  The Motor's ABSOLUTE position is the number of steps away from HOME.  This number can be
//  positive (clockwise) or negative (counter-clockwise).
//
//     ▐── ····· ──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼──┼── ····· ──▌
//     ▐           │           │           │           │           │                 ▌
//    Lower      -2000       -1000         0          1000        2000             Upper
//    Limit                               HOME                                     Limit
//
//  The motor's RELATIVE position is the number of steps away from its last position.
//
//  LOWER and UPPER range limit values can be specified.
//  The default LOWER and UPPER range limits are set to +2 billion and -2 billion at start up.
//  It is assumed the user will set these limits from a configuration file or other means.
//
//  If a range limit has been reached, then the motor's motion is stopped and a Range Error
//  is returned from the Run() method.
//
//  Two GPIO pins can also be optionally specified for checking upper and lower limit switches.
//
//─────────────────────────────────────────────────────────────────────────────────────────────────
//
//  At startup, this class initializes the motor as not homed (the motor is at an unknown position).
//  Before motion can begin, the motor must be Homed with SetHomePosition() or "SH" command.
//
//─────────────────────────────────────────────────────────────────────────────────────────────────
//
//  Motor rotation velocity follows a trapezoidal shape.
//  A linear ramp-up/ramp-down rate is set with the SetRamp() method or "SR" command.
//
//                                   ┌────────────────────────────────┐    <── full velocity
//  A ramp value of 0                │                                │
//  specifies no ramping:            │                                │
//  (not recommended !!!)            │                                │
//                                 ──┴────────────────────────────────┴──
//
//                                       .────────────────────────.        <── full velocity
//  A ramp value of 5                   /                          \.
//  specifies moderate ramping:        /                            \.
//  This is the default at startup    /                              \.
//                                 ──┴────────────────────────────────┴──
//
//                                           .────────────────.            <── full velocity
//  A ramp value of 9                      /                    \.
//  specifies gradual ramping:           /                        \.
//                                     /                            \.
//                                 ───┴──────────────────────────────┴───
//
//  Use low values (2, 3, ..) for fast accelerations with light loads and high values (.., 8, 9) for slow accelerations
//  with heavy loads.  It is highly recommended to use slow acceleration when moving high inertial loads.
//
//                                            ──────────    <── full velocity
//  If there is not enough time to achieve
//  full velocity, then rotation velocity         /\.
//  follows a "stunted" triangle path:           /  \.
//                                            ──┴────┴──
//
//  Once a ramp value is set, all rotate commands will use that ramp value.
//  The default ramp value at start-up is 5.
//
//─────────────────────────────────────────────────────────────────────────────────────────────────
//
//  All control can be performed either by directly calling methods or by executing a command
//  string, from a UI for example.  To keep the motor running, the Run() method must be called
//  continuously from your loop() method.
//
//  The Run() method returns one the following enum results:
//
//    OKAY                - Idle or still running
//    RUN_COMPLETE        - Motion complete, reached target position normally
//    RANGE_ERROR_LOWER   - Reached lower range limit
//    RANGE_ERROR_UPPER   - Reached upper range limit
//    LIMIT_SWITCH_LOWER  - Lower limit switch triggered
//    LIMIT_SWITCH_UPPER  - Upper limit switch triggered
//
//  Your app should normally wait until the motor is finished with a previous Rotate method/command
//  before issuing a new Rotate command.  If a Rotate command is called while the motor is already
//  running, then the current rotation is interrupted and the new Rotate command is executed from
//  the motor's current position.
//
//─────────────────────────────────────────────────────────────────────────────────────────────────
//
//  This class overrides the DoImmediate() and ExecuteCommand() methods of the base Device class.
//  The ExecuteCommand() method handles the following Device commands:
//
//    FHOM              = FIND HOME             - Seeks counter-clockwise until lower limit switch is triggered, backs off a bit and sets HOME position
//    SHOM              = SET HOME POSITION     - Sets the current position of the motor as its HOME position (Sets Absolute position to zero)
//    SLOW|range        = SET LOWER LIMIT       - Sets the LOWER LIMIT (minimum Absolute Position) of the motor's range
//    SUPP|range        = SET UPPER LIMIT       - Sets the UPPER LIMIT (maximum Absolute Position) of the motor's range
//    SRMP|ramp         = SET RAMP              - Sets the trapezoidal velocity RAMP (up/down) for smooth motor start and stop, ramp = 1-char (0-9)
//    RABS|ssssposition = ROTATE ABSOLUTE       - Rotates motor to an Absolute target position from its HOME position, ssss = 4-char speed (0001-9999)
//    RREL|ssssnumSteps = ROTATE RELATIVE       - Rotates motor clockwise or counter-clockwise any number of steps from its current position, ssss = 4-char speed (0001-9999)
//    RHOM              = ROTATE HOME           - Rotates motor to its HOME position
//    RLOW              = ROTATE TO LOWER LIMIT - Rotates motor to its LOWER LIMIT position
//    RUPP              = ROTATE TO UPPER LIMIT - Rotates motor to its UPPER LIMIT position
//    STOP              = E-STOP                - Stops the motor immediately and releases motor (emergency stop)
//    GSTA              = GET STATE             - Returns current motor state (see StepperMotorState enum)
//    GABS              = GET ABSOLUTE position - Returns the motor's current step position relative to its HOME position
//    GREL              = GET RELATIVE position - Returns the motor's current step position relative to its last targeted position
//    GLOW              = GET LOWER LIMIT       - Returns the motor's Absolute LOWER LIMIT position
//    GUPP              = GET UPPER LIMIT       - Returns the motor's Absolute UPPER LIMIT position
//
//=============================================================================

#ifndef DEV_STEPPERMOTOR_4PHASE_H
#define DEV_STEPPERMOTOR_4PHASE_H

//--- Includes --------------------------------------------

#include "Device.h"

//--- Defines ---------------------------------------------

#define HOMING_SPEED  3000L
#define PULSE_WIDTH   5     // 5-microseconds (check your driver's pulse width requirement)

//--- Enums -----------------------------------------------

enum Stepper4_State
{
  STEPPER4_ENABLED,   // Motor is enabled
  STEPPER4_DISABLED,  // Motor is disabled (frre to move)
  STEPPER4_RUNNING,   // Motor is running (rotating) and is currently executing a Rotate command
  STEPPER4_ESTOPPED   // Motor is in an E-STOP condition (Emergency Stop), it must be Enabled to resume motion
};

enum Stepper4_Direction
{
  ROT_CLOCKWISE,
  ROT_COUNTER_CLOCKWISE
};

enum Stepper4_RunReturn
{
  STEPPER4_OKAY,                // Idle or still running
  STEPPER4_RUN_COMPLETE,        // Motion complete, reached target position normally
  STEPPER4_RANGE_ERROR_LOWER,   // Reached lower range limit
  STEPPER4_RANGE_ERROR_UPPER,   // Reached upper range limit
  STEPPER4_LIMIT_SWITCH_LOWER,  // Lower limit switch triggered
  STEPPER4_LIMIT_SWITCH_UPPER   // Upper limit switch triggered
};


//=========================================================
//  class Dev_StepperMotor_4Phase
//=========================================================

class Dev_StepperMotor_4Phase : public Device
{
  private:
    Stepper4_State      State     = STEPPER4_DISABLED;
    Stepper4_Direction  Direction = ROT_CLOCKWISE;

    // GPIO Pins For Digital Stepper Driver and Limit Switches
    int  DrivePin1, DrivePin2, DrivePin3, DrivePin4, LLSwitchPin, ULSwitchPin;

    bool           Homed = false;      // The motor must be "Homed" before use
    const long     RampScale = 5L;
    long           MaxVelocity;        // Highest velocity while running
    long           TotalSteps;
    long           StepCount;
    long           StepIncrement;      // 1 for clockwise rotations, -1 for counter-clockwise
    long           AbsolutePosition;   // Number of steps from HOME position
    long           DeltaPosition;      // Number of steps from last position
    long           TargetPosition;     // Target position for end of rotation
    long           LowerLimit;         // Minimum step position
    long           UpperLimit;         // Maximum step position
    long           RampSteps;          // Total number of steps during ramping
    long           RampDownStep;       // Step at which to start ramping down
    long           Velocity;           // Current velocity of motor
    long           VelocityIncrement;  // Velocity adjustment for ramping (determined by ramp factor)
    long           NextPosition;       // Position after next step
    unsigned long  NextStepMicros;     // Target micros for next step
    char           SpeedString[5];
    int            Speed;
    long           TargetOrSteps;
    int            Phase = 0;          // Phase is 0..3 for CW rotation, and 3..0 for CCW

    void startRotation ();
    void doStep ();

  public:
    Dev_StepperMotor_4Phase (const char *inName, int inDrivePin1, int inDrivePin2, int inDrivePin3, int inDrivePin4, int llSwitchPin, int ulSwitchPin);

    Stepper4_RunReturn  Run ();  // Keeps the motor running (must be called from your loop() function with no delay)

    void            Enable              ();                                      // Enables the motor driver (energizes the motor)
    void            Disable             ();                                      // Disables the motor driver (releases the motor)

    void            FindHome            ();                                      // "Auto Home": Seek to lower limit switch and set home position just off it
    void            SetHomePosition     ();                                      // Sets the current position of the motor as its HOME position (Sets Absolute position to zero)
    void            SetLowerLimit       (long lowerLimit);                       // Sets the LOWER LIMIT (minimum Absolute Position) of the motor's range
    void            SetUpperLimit       (long upperLimit);                       // Sets the UPPER LIMIT (maximum Absolute Position) of the motor's range
    void            SetRamp             (int ramp);                              // Sets the trapezoidal velocity RAMP (up/down) for smooth motor start and stop

    void            RotateAbsolute      (long absPosition, int stepsPerSecond);  // Rotates motor to an Absolute target position from its HOME position
    void            RotateRelative      (long numSteps, int stepsPerSecond);     // Rotates motor clockwise(+) or counter-clockwise(-) any number of steps from its current position
    void            RotateToHome        ();                                      // Rotates motor to its HOME position
    void            RotateToLowerLimit  ();                                      // Rotates motor to its LOWER LIMIT position
    void            RotateToUpperLimit  ();                                      // Rotates motor to its UPPER LIMIT position
    void            EStop               ();                                      // Stops the motor immediately (emergency stop)

    bool            IsHomed             ();                                      // Returns true or false
    Stepper4_State  GetState            ();                                      // Returns current state of motor
    long            GetAbsolutePosition ();                                      // Returns the motor's current step position relative to its HOME position
    long            GetRelativePosition ();                                      // Returns the motor's current step position relative to its last targeted position
    long            GetLowerLimit       ();                                      // Returns the motor's Absolute LOWER LIMIT position
    long            GetUpperLimit       ();                                      // Returns the motor's Absolute UPPER LIMIT position
    const char *    GetVersion          ();                                      // Returns this firmware's current version

    //--- Device Overrides ---
    ProcessStatus  DoImmediate    ();  // Device class override
    ProcessStatus  ExecuteCommand (char *command, char *params);  // Device class override
};

#endif
