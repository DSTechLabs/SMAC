//=========================================================
//
//     FILE : ESP32_Servo.h
//
//  PROJECT : A SMAC Framework Device - Servo Motor
//
//    NOTES : A servo motor that is directly driven by
//            the Node's ESP32 using the Arduino
//            ESP32Servo class.
//
//   AUTHOR : Bill Daniels
//            Copyright 2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

#ifndef ESP32_SERVO_H
#define ESP32_SERVO_H

//--- Includes --------------------------------------------

#include "Device.h"
#include <ESP32Servo.h>


//=========================================================
//  class ESP32_Servo
//=========================================================

class ESP32_Servo : public Device
{
  protected:
    Servo  servo;

  public:
    ESP32_Servo (const char *inName, int inServoPin);

    ProcessStatus ExecuteCommand ();  // override default processing
};

#endif
