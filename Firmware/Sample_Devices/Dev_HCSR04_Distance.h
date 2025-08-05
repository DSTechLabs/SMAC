//=========================================================
//
//     FILE : Dev_HCSR04_Distance.h
//
//  PROJECT : Any (SMAC Framework)
//
//   AUTHOR : Bill Daniels
//            Copyright 2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

#ifndef DEV_HCSR04_H
#define DEV_HCSR04_H

//--- Includes --------------------------------------------

#include "Device.h"

//--- Defines ---------------------------------------------

#define TRIGGER_PIN  37
#define ECHO_PIN     36


//=========================================================
//  class Dev_HCSR04_Distance
//=========================================================

class Dev_HCSR04_Distance : public Device
{
  protected:
    int  triggerPin = TRIGGER_PIN;  // default
    int  echoPin    = ECHO_PIN;     // default

    float  duration;
    float  distance;  // in meters

  public:
    Dev_HCSR04_Distance (const char *inName, int inTriggerPin=TRIGGER_PIN, int inEchoPin=ECHO_PIN);

    ProcessStatus  DoPeriodic ();  // override
};

#endif
