//=========================================================
//
//     FILE : Distance.cpp
//
//  PROJECT : Any SMAC Device using the HC-SR04
//            Ultrasonic Distance Sensor
//
//   AUTHOR : Bill Daniels
//            Copyright 2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

//--- Includes --------------------------------------------

#include <Arduino.h>
#include "Distance.h"

//--- Constructor -----------------------------------------

Distance::Distance (const char *inName, int inTriggerPin, int inEchoPin)
         :Device (inName)
{
  // Save trigger and echo pins
  triggerPin = inTriggerPin;
  echoPin    = inEchoPin;

  // Init pins
	pinMode (triggerPin, OUTPUT);
	pinMode (echoPin   , INPUT );

  // Set default periodic rate
  SetRate (4 * 3600);  // 4 samples per second

  // No need for Immediate processing
  immediateEnabled = false;
}

//--- getDistance  ----------------------------------------

float Distance::getDistance ()
{
  // Send 10 µs trigger pulse
  digitalWrite (triggerPin, LOW ); delayMicroseconds ( 2);
  digitalWrite (triggerPin, HIGH); delayMicroseconds (10);
  digitalWrite (triggerPin, LOW );

  // Measure echo (timeout protects from hangs)
  duration = pulseIn (echoPin, HIGH, timeout);

  // Return only good range
  if (duration > minDuration && duration < maxDuration)
    return duration / 146.5455f;  // inches
    // return duration / 5769.51f;   // meters
    // return duration / 57.6951f;   // centimeters

  return NAN;
}

//--- DoPeriodic (override) -------------------------------

ProcessStatus Distance::DoPeriodic ()
{
  newDistance = getDistance ();

  // Return only valid readings
  if (!isnan (newDistance))
    prevDistance = newDistance;

  ftoa (SMACData.values, 10, prevDistance, 2);
  return SUCCESS_DATA;
}
