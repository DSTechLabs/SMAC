//=========================================================
//
//     FILE : Distance.h
//
//  PROJECT : Any (SMAC Framework)
//
//   AUTHOR : Bill Daniels
//            Copyright 2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

#ifndef Distance_H
#define Distance_H

//--- Includes --------------------------------------------

#include "Device.h"

//--- Defines ---------------------------------------------

#define TRIGGER_PIN  37
#define ECHO_PIN     36


//=========================================================
//  class Distance
//=========================================================

class Distance : public Device
{
  protected:
    int  triggerPin = TRIGGER_PIN;  // default
    int  echoPin    = ECHO_PIN;     // default

    float  duration;
    float  distance;  // in meters

  public:
    Distance (const char *inName, int inTriggerPin=TRIGGER_PIN, int inEchoPin=ECHO_PIN);

    ProcessStatus  DoPeriodic ();  // override
};

#endif
