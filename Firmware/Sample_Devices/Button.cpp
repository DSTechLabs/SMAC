//=========================================================
//
//     FILE : Button.cpp
//
//  PROJECT : Any (SMAC Framework)
//
//    NOTES : Button using the MCU's built-in 10k pull-down resistor.
//
//              3.3V
//               ▼
//               │
//               │
//                │
//                │█ Button
//                │
//               │
//               │
//               ├───────────▶ GPIO Pin
//               │
//               █
//               █ 10KΩ pulldown
//               █
//               │
//               │
//               ▼
//              GND
//
//   AUTHOR : Bill Daniels
//            Copyright 2021-2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

//--- Includes --------------------------------------------

#include <Arduino.h>
#include "Button.h"

//--- Constructor -----------------------------------------

Button::Button (const char *inName, int inButtonPin)
       :Device (inName)
{
  // Init GPIO pin for a button
  buttonPin = inButtonPin;
  pinMode (buttonPin, INPUT_PULLDOWN);  // Use pulldown resistor

  // No need for Periodic processing
  periodicEnabled = false;
}

//--- DoImmediate (override) ------------------------------

IRAM_ATTR ProcessStatus Button::DoImmediate ()
{
  // Store the latest reading in newState:
  // Not doing any software debouncing here for simplicity
  // Hardware debouncing should be done with an RC circuit
  newState = digitalRead (buttonPin);

  // Send DataPacket if button has changed state
  if (newState != currentState)
  {
    // Send DataPacket
    DataPacket.timestamp = millis ();
    strcpy (DataPacket.value, (newState==0 ? "0" : "1"));

    currentState = newState;
    return SUCCESS_DATA;
  }

  return SUCCESS_NODATA;
}
