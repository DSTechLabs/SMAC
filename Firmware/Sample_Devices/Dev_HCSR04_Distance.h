//=========================================================
//
//     FILE : Distance.h
//
//  PROJECT : Any SMAC Device using the HC-SR04
//            Ultrasonic Distance Sensor
//
//    NOTES : Measurements assume:
//              75ºF, 50% humidity and at sea level
//              Speed of sound ~ 346.65 meters/sec
//
//   AUTHOR : Bill Daniels
//            Copyright 2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

#ifndef DISTANCE_H
#define DISTANCE_H

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
  private:
    const unsigned long  minDuration = 230UL;    // Lower limit ~4cm
    const unsigned long  maxDuration = 23078UL;  // Upper limit ~4m
    const unsigned long  timeout     = 30000UL;  // Echo timeout
    unsigned long        duration;

  protected:
    int    triggerPin   = TRIGGER_PIN;  // default
    int    echoPin      = ECHO_PIN;     // default
    float  prevDistance = 10.0f;
    float  newDistance  = 10.0f;

    float  getDistance ();

  public:
    Distance (const char *inName, int inTriggerPin=TRIGGER_PIN, int inEchoPin=ECHO_PIN);

    ProcessStatus  DoPeriodic ();  // override
};

#endif
