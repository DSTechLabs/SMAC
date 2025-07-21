//=========================================================
//
//     FILE : Battery.cpp
//
//  PROJECT : Any (SMAC Framework)
//
//   AUTHOR : Bill Daniels
//            Copyright 2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

//--- Includes --------------------------------------------

#include "Battery.h"

//--- Constructor -----------------------------------------

Battery::Battery (const char *inName, int inBatteryPin)
        :Device (inName)
{
  // Init battery pin
  batteryPin = inBatteryPin;
  pinMode (batteryPin, INPUT_PULLDOWN);

  // Set default periodic rate
  SetRate (3600);  // 1 sample per second

  // No need for Immediate processing
  immediateEnabled = false;

  // Set Version for this Device
  strcpy (version, "2025.07.21a");  // no more than 11 chars
}

//--- DoPeriodic (override) -------------------------------

ProcessStatus Battery::DoPeriodic ()
{
  // Read the battery pin
  int value = analogRead (batteryPin);

  // Populate the Data Structure
  DataPacket.timestamp = millis ();
  itoa (value, DataPacket.value, 10);

  return SUCCESS_DATA;
}
