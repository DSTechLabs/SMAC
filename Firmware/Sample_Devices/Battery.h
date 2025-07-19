//=========================================================
//
//     FILE : Battery.h
//
//  PROJECT : Any (SMAC Framework)
//
//   AUTHOR : Bill Daniels
//            Copyright 2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

#ifndef BATTERY_H
#define BATTERY_H

//--- Includes --------------------------------------------

#include "Device.h"

//--- Defines ---------------------------------------------

#define BATTERY_PIN  4


//=========================================================
//  class Battery
//=========================================================

class Battery : public Device
{
  protected:
    int  batteryPin;

  public:
    Battery (const char *inName, int inBatteryPin=BATTERY_PIN);

    ProcessStatus  DoPeriodic ();  // override
};

#endif
