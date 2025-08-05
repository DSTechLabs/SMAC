//=========================================================
//
//     FILE : Dev_MPU6050_Gyro.h
//
//  PROJECT : Any (SMAC Framework)
//
//    NOTES : sensor_event_t
//            https://adafruit.github.io/Adafruit_CircuitPlayground/html/structsensors__event__t.html
//
//   AUTHOR : Bill Daniels
//            Copyright 2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

#ifndef DEV_MPU6050_GYRO
#define DEV_MPU6050_GYRO

//--- Includes --------------------------------------------

#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include "Device.h"

//--- Defines ---------------------------------------------

#define  GYRO_I2C_ADDRESS  MPU6050_I2CADDR_DEFAULT


//=========================================================
//  class Dev_MPU6050_Gyro
//=========================================================

class Dev_MPU6050_Gyro : public Device
{
  protected:
    Adafruit_MPU6050  *gyro;
    sensors_event_t   accelSensor, gyroSensor, tempSensor;
    float             accelX, accelY, accelZ;
    float             pitch, roll;

  public:
    Dev_MPU6050_Gyro (const char *inName);

    ProcessStatus  DoPeriodic ();  // override
};

#endif
