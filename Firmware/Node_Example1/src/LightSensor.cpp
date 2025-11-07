//=========================================================
//
//     FILE : LightSensor.cpp
//
//  PROJECT : SMAC Framework - Example 1
//
//    NOTES : Analog light sensor using a photoresistor.
//            Derived from Device base class
//
//                             ~180Ω..1MΩ
//                    5KΩ           ░
//               ┌───█████───┬─────(Θ)─────┐
//               │           │             │
//               │           │             │
//               ^           ▼             ▼
//             3.3V         GPIO          GND
//                          pin
//
//   AUTHOR : Bill Daniels
//            Copyright 2021-2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

//--- Includes --------------------------------------------

#include <Arduino.h>
#include "LightSensor.h"

//--- Constructor -----------------------------------------

LightSensor::LightSensor (const char *inName, int inSensorPin)
            :Device (inName)
{
  // Init GPIO pin
  sensorPin = inSensorPin;
  pinMode (sensorPin, INPUT);

  // No need for Immediate Processing
  immediateEnabled = false;

  // Default to 10 samples per second
  SetRate (10.0 * 3600.0);

  // Set Version for this Device
  strcpy (version, "1.0.0");  // no more than 9 chars
}

//--- DoPeriodic (override) -------------------------------

IRAM_ATTR ProcessStatus LightSensor::DoPeriodic ()
{
  // The "Periodic Process" for this device is simply to measure a sample
  int sample = 4095 - analogRead (sensorPin);

  // All data from a Device is returned by populating the global <SMACData.values> string field
  // and returning an appropriate ProcessStatus.  Use itoa() for integers and ftoa() for floats.
  //
  //   itoa (myIntegerValue, SMACData.values, 10);
  //   -OR-
  //   ftoa (SMACData.values, sizeof(SMACData.values), myFloatValue, precision);
  //
  // where precision is the the number of digits to appear after the decimal point.
  // If precision is negative, all digits are converted.
  itoa (sample, SMACData.values, 10);  // SMACData.values must be a terminated string

  // DoPeriodic() must return one of four possible "ProcessStatus" values:
  //
  //   SUCCESS_DATA    - Process performed successfully, send SMACData to Relayer (such as a sensor reading)
  //   SUCCESS_NODATA  - Process performed successfully, no data to send to Relayer
  //   FAIL_DATA       - Process failed, send SMACData to Relayer (such as error code or message)
  //   FAIL_NODATA     - Process failed, no data to send to Relayer

  // For this example, we indicate a successful reading with data to send
  return SUCCESS_DATA;
}
