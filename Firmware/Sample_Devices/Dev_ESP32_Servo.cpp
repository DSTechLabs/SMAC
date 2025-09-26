//=========================================================
//
//     FILE : ESP32_Servo.cpp
//
//  PROJECT : A SMAC Framework Device - Servo Motor
//
//   AUTHOR : Bill Daniels
//            Copyright 2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

//--- Includes --------------------------------------------

#include <Arduino.h>
#include "ESP32_Servo.h"

//--- Constructor -----------------------------------------

ESP32_Servo::ESP32_Servo (const char *inName, int inServoPin)
            :Device (inName)
{
	// Allow allocation of all timers
	ESP32PWM::allocateTimer (0);
	ESP32PWM::allocateTimer (1);
	ESP32PWM::allocateTimer (2);
	ESP32PWM::allocateTimer (3);

	servo.setPeriodHertz (50);             // Standard 50Hz servo
	servo.attach (inServoPin, 500, 2500);  // PW Range for 180º servo
  servo.write (90);                      // Start at 0º (Our range is -90º..90º; need to convert to 0º..180º)

  // No need for Immediate or Periodic Processing
  immediateEnabled = false;
  periodicEnabled  = false;

  // Set Version for this Device
  strcpy (version, "1.0.0");  // no more than 9 chars
}

//--- DoPeriodic (override) -------------------------------

ProcessStatus ESP32_Servo::ExecuteCommand ()
{
  // Command info is held in the global <CommandPacket> structure.
  // This method is only called for commands targeted for this device.

  // First call the base class ExecuteCommand method
  pStatus = Device::ExecuteCommand ();

  // Check if command was handled by the base class
  if (pStatus == NOT_HANDLED)
  {
    // The command was NOT handled by the base class,
    // so handle custom commands:

    //--- Rotate Servo to Angle ---
    if (strcmp (CommandPacket.command, "GOTO") == 0)
    {
      int angle = atoi (CommandPacket.params);

      if (angle >= -90 && angle <= 90)
        servo.write (angle + 90);

      // Indicate successful and no data to return
      pStatus = SUCCESS_NODATA;
    }
  }

  // Return the resulting ProcessStatus
  return pStatus;
}
