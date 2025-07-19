//==========================================================
//
//     FILE : ServoMotor.h
//
//  PROJECT : Any (SMAC Framework)
//
//    NOTES : This firmware is used to incrementally move a hobby servo motor
//            using I2C communication between an MCU development board such as
//            an Arduino NANO and the Adafruit PCA9685 16-Channel Servo driver board.
//            https://www.adafruit.com/product/815
//
//            Include the Adafruit library for this board:
//            https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library
//
//            This class allows for multiple motors to run simultaneously.
//            You can also directly send the driver a pulse width for this motor.
//
//   AUTHOR : Bill Daniels
//            Copyright 2024-2025, D+S Tech Labs, Inc.
//            MIT License
//
//==========================================================
//
//  This class uses the Adafruit PWM Servo Driver class to operate a servo motor.
//  The range of motion of the servo is defined by lower and upper pulse width values.
//  Those values are specified in the constructor of this class along with the angle
//  they represent (in degrees).  All target positions are then specified as an angle.
//  Gradual motion is accomplished by moving the servo in single 1-degree "steps".
//
//  All control can be performed either by directly calling methods or by executing
//  a command string, from a UI for example.  To keep the motor running, the Run()
//  method must be called continuously from your loop() method.  The Run() method
//  determines when its time to take a step, depending on the speed requested
//  (SLOW, MEDIUM or FAST).
//
//  The Run() method returns a string (char *) with one of the following:
//
//    OK  - Idle or still running
//    RCa - Run Complete; final position is aº (1 or more digits)
//    REL - Range Error; tried to exceed lower limit (minimum angle)
//    REU - Range Error; tried to exceed upper limit (maximum angle)
//
//  Your app should normally wait until the motor is finished with a previous Rotate method/command
//  before issuing a new Rotate command.  If a Rotate command is called while the motor is already
//  running, then the current rotation is interrupted and the new Rotate command is executed from
//  the motor's current position.
//
//─────────────────────────────────────────────────────────────────────────────
//
//  This class also has a method for operating the servo by executing String commands.
//  Use the ExecuteCommand() method passing the following 2-char commands:
//
//    RA... = ROTATE ABSOLUTE  - Rotates motor to an Absolute target angle (in degrees)
//    RR... = ROTATE RELATIVE  - Rotates motor a positive or negative increment from its current angle
//    RH    = ROTATE HOME      - Rotate to HOME position
//    ES    = E-STOP           - Stops the motor immediately
//    GV    = GET VERSION      - Returns this firmware's current version
//
//    ───────────────────────────────────────────────────
//     Command String Format: (no spaces between fields)
//    ───────────────────────────────────────────────────
//                               cc  s  aaa<lf>
//                               │   │   │
//      Command/Query ───────────┘   │   │
//        [2-chars]                  │   │
//          RA = ROTATE ABSOLUTE     │   │
//          RR = ROTATE RELATIVE     │   │
//          RH = ROTATE HOME         │   │
//          ES = E-STOP              │   │
//          GV = GET VERSION         │   │
//                                   │   │
//      Speed [1-char] ──────────────┘   │
//          S = Slow speed               │
//          M = Medium speed             │
//          F = Fast speed               │
//                                       │
//      Absolute or Relative Angle ──────┘ ──────┐
//        [1 or more digits plus sign]           ├─── For ROTATE commands only
//        No padding necessary             ──────┘
//
//  You can also directly set a pulse width for the motor driver
//  using the PW command:
//
//     PWnum - Set pulse width to num
//
//  This class may also be queried for position, range limits and firmware version with the following commands:
//
//     GA = GET ABSOLUTE position - Returns the motor's current position (angle)
//     GR = GET RELATIVE position - Returns the motor's current position relative to its last targeted position
//     GV = GET VERSION           - Returns this firmware's current version
//
//  The returned result is a string with the following format:
//     APca... = Absolute Position of motor on channel c is aº (angle)
//     RPca... = Relative Position of motor on channel c is aº from its last targeted position
//
//  ───────────────────────────────────────────────
//   Examples: (quotes are not included in string)
//  ───────────────────────────────────────────────
//   "RAM50" - ROTATE ABSOLUTE        - Rotate the motor at MEDIUM speed to the angle of 50º
//   "RRS-5" - ROTATE RELATIVE        - Rotate the motor at SLOW speed 5º less than its current position
//   "ES"    - EMERGENCY STOP         - Immediately stop the motor and cancel rotation command
//   "GR"    - GET RELATIVE POSITION  - Get the current relative position of the motor
//
//─────────────────────────────────────────────────────────────────────────────────────────────────

#ifndef SM_H
#define SM_H

//--- Includes --------------------------------------------

#include <Adafruit_PWMServoDriver.h>

//--- Defines ---------------------------------------------

#define RUN_RETURN_LENGTH  20

//--- Enums -----------------------------------------------

enum MotorState
{
  IDLE,    // Not running
  RUNNING  // Motor is running (rotating) and is currently executing a Rotate command
};

enum MotorSpeed  // 1º step periods in µs
{
  SLOW   = 20000,
  MEDIUM = 10000,
  FAST   = 5000
};


//---------------------------------------------------------
//  class ServoMotor
//---------------------------------------------------------

class ServoMotor
{
  private:
    const char  version[32] = "Servo Motor version 2025-06-17";

    Adafruit_PWMServoDriver *servoDriver;

    int            channelNum = 0;                // Channel number for 16-channel driver board (0-15)
    int            lowerLimitPW;                  // Pulse width for minimum position
    int            lowerLimitAngle;               // Angle of lowerLimitPW in degrees
    int            upperLimitPW;                  // Pulse width for maximum position
    int            upperLimitAngle;               // Angle of upperLimitPW in degrees
    int            homeAngle;                     // Angle for home position
    bool           validParams = false;           // Protect arm from invalid constructor params
    double         angle2PWFactor;                // Conversion factor
    MotorState     state = IDLE;                  // Default is idle
    int            absolutePosition;              // current position (degrees)
    int            deltaPosition;                 // Number of steps from last position
    int            targetPosition;                // Target position for end of rotation
    MotorSpeed     stepPeriod;                    // Current speed of motion (a single step every n µs)
    int            nextPosition;                  // Position after next step
    unsigned long  nextStepMicros;                // Target micros for next step
    long           stepIncrement;                 // +1º or -1º
    char           runReturn[RUN_RETURN_LENGTH];  // Space for return strings from Run() method

    void startRotation ();
    int  angle2PW (int angle);

  public:
    ServoMotor (Adafruit_PWMServoDriver *inServoDriver,
                int inChannelNum,
                int inLowerLimitPW, int inLowerLimitAngle,
                int inUpperLimitPW, int inUpperLimitAngle,
                int inHomeAngle);

    const char *  Run                 ();  // Keeps the motor running (must be called from your loop() function with no delay)

    void          RotateAbsolute      (int targetAngle, MotorSpeed speed);  // Rotates motor to an Absolute target angle (degrees)
    void          RotateRelative      (int angle, MotorSpeed speed);        // Rotates motor to a Relative angle (+/- degrees from its current position)
    void          RotateHome          ();                                   // Rotates motor to its Home position at FAST speed
    void          EStop               ();                                   // Stops the motor immediately (emergency stop)

    void          SetPulseWidth       (int pulseWidth);                     // Directly set a pulse width for this motor channel

    MotorState    GetState            ();                                   // Returns current state of motor
    long          GetAbsolutePosition ();                                   // Returns the motor's current angle
    long          GetRelativePosition ();                                   // Returns the motor's current angle relative to its last targeted angle
    const char *  GetVersion          ();                                   // Returns this firmware's current version

    const char *  ExecuteCommand      (const char *packet);                 // Execute a servo motor function by string command (see notes above)
};

#endif
