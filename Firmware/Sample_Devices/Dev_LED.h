//=========================================================
//
//     FILE : Dev_LED.h
//
//  PROJECT : Any (SMAC Framework)
//
//   AUTHOR : Bill Daniels
//            Copyright 2021-2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

#ifndef DEV_LED_H
#define DEV_LED_H

//--- Includes --------------------------------------------

#include "Device.h"


//=========================================================
//  class Dev_LED
//=========================================================

class Dev_LED : public Device
{
  protected:
    int  ledPin = 4;  // default pin

  public:
    Dev_LED (const char *inName, int inLEDPin);

    ProcessStatus  ExecuteCommand ();  // override
};

#endif
