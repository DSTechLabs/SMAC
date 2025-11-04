//=========================================================
//
//     FILE : Button.h
//
//  PROJECT : SMAC Framework - Example 2
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

#ifndef BUTTON_H
#define BUTTON_H

//--- Includes --------------------------------------------

#include "Device.h"


//=========================================================
//  class Button
//=========================================================

class Button : public Device
{
  protected:
    int  buttonPin    = 5;   // default pin
    int  currentState = 99;  // force sending initial state
    int  newState;

  public:
    Button (const char *inName, int inButtonPin);

    ProcessStatus  DoImmediate ();  // override default processing
};

#endif
