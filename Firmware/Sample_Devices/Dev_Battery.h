//=========================================================
//
//     FILE : Dev_Battery.h
//
//  PROJECT : Any (SMAC Framework)
//
//   AUTHOR : Bill Daniels
//            Copyright 2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

#ifndef DEV_BATTERY_H
#define DEV_BATTERY_H

//--- Includes --------------------------------------------

#include "Device.h"

//--- Defines ---------------------------------------------

#define BATTERY_PIN  4


//=========================================================
//  class Dev_Battery
//=========================================================

class Dev_Battery : public Device
{
  protected:
    int  batteryPin;

  public:
    Dev_Battery (const char *inName, int inBatteryPin=BATTERY_PIN);

    ProcessStatus  DoPeriodic ();  // override
};

#endif
