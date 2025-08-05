//=========================================================
//
//     FILE : Dev_HCSR04_Distance.cpp
//
//  PROJECT : Any (SMAC Framework)
//
//   AUTHOR : Bill Daniels
//            Copyright 2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

//--- Includes --------------------------------------------

#include <Arduino.h>
#include "Dev_HCSR04_Distance.h"

//--- Constructor -----------------------------------------

Dev_HCSR04_Distance::Dev_HCSR04_Distance (const char *inName, int inTriggerPin, int inEchoPin)
                    :Device (inName)
{
  // Save trigger and echo pins
  triggerPin = inTriggerPin;
  echoPin    = inEchoPin;

  // Init pins
	pinMode (triggerPin, OUTPUT);
	pinMode (echoPin   , INPUT );

  // Set default periodic rate
  SetRate (36000);  // 10 samples per second

  // No need for Immediate processing
  immediateEnabled = false;

  // Set Version for this Device
  strcpy (version, "1.0.0");  // no more than 9 chars
}

//--- DoPeriodic (override) -------------------------------

ProcessStatus Dev_HCSR04_Distance::DoPeriodic ()
{
  // Start with trigger pin low
	digitalWrite (triggerPin, LOW);
	delayMicroseconds (2);

  // Trigger a pulse
	digitalWrite (triggerPin, HIGH);
	delayMicroseconds (10);
	digitalWrite (triggerPin, LOW);

  // Listen for echo
  duration = (float) pulseIn (echoPin, HIGH, 20000UL);
  // distance = duration / 5830.9f;   // meters
  distance = duration / 148.1f;    // inches

  // Return distance
  DataPacket.timestamp = millis ();
  sprintf (DataPacket.value, "%.2f", distance);

  return SUCCESS_DATA;
}
