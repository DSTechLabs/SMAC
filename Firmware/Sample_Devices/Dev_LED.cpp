//=========================================================
//
//     FILE : Dev_LED.cpp
//
//  PROJECT : Any (SMAC Framework)
//
//   AUTHOR : Bill Daniels
//            Copyright 2021-2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

//--- Includes --------------------------------------------

#include <Arduino.h>
#include "Dev_LED.h"

//--- Constructor -----------------------------------------

Dev_LED::Dev_LED (const char *inName, int inLEDPin)
        :Device (inName)
{
  // Init GPIO pin
  ledPin = inLEDPin;
  pinMode (ledPin, OUTPUT);

  // No need for Immediate or Periodic processing
  immediateEnabled = periodicEnabled = false;

  // Set Version for this Device
  strcpy (version, "1.0.0");  // no more than 9 chars
}

//--- ExecuteCommand (override) ---------------------------

ProcessStatus Dev_LED::ExecuteCommand ()
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

    //--- Turn On ---
    if (strcmp (CommandPacket.command, "LEON") == 0)
    {
      // Turn on the LED pin
      digitalWrite (ledPin, HIGH);

      // Indicate successful and no data to return
      pStatus = SUCCESS_NODATA;
    }

    //--- Turn Off ---
    else if (strcmp (CommandPacket.command, "LEOF") == 0)
    {
      // Turn off the LED pin
      digitalWrite (ledPin, LOW);

      // Indicate successful and no data to return
      pStatus = SUCCESS_NODATA;
    }
  }

  // Return the resulting ProcessStatus
  return pStatus;
}
