//=========================================================
//
//     FILE : LED.h
//
//  PROJECT : SMAC Framework - Example 2
//
//   AUTHOR : Bill Daniels
//            Copyright 2021-2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

#ifndef LED_H
#define LED_H

//--- Includes --------------------------------------------

#include "Device.h"


//=========================================================
//  class LED
//=========================================================

class LED : public Device
{
  protected:
    int   ledPin = 4;  // default pin

  public:
    LED (const char *inName, int inLEDPin);

    ProcessStatus  ExecuteCommand (char *command, char *params=NULL);  // override
};

#endif
