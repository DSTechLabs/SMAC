//=========================================================
//
//     FILE : Gyro.cpp
//
//  PROJECT : Any (SMAC Framework)
//
//   AUTHOR : Bill Daniels
//            Copyright 2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

//--- Includes --------------------------------------------

#include <Arduino.h>
#include <Wire.h>
#include <math.h>
#include "Gyro.h"

//--- Constructor -----------------------------------------

Gyro::Gyro (const char *inName)
     :Device (inName)
{
  // Init the MPU-6050
  gyro = new Adafruit_MPU6050 ();
  // if (!gyro->begin (GYRO_I2C_ADDRESS, &Wire))
  if (!gyro->begin ())
  {
    Serial.println ("Unable to initialize the MPU-6050 Sensor.");
    return;
  }

  gyro->setAccelerometerRange (MPU6050_RANGE_8_G    );  // 2, 4, 8 or 16 G's
  gyro->setGyroRange          (MPU6050_RANGE_500_DEG);  // 250, 500, 1000 or 2000 degrees per second
  gyro->setFilterBandwidth    (MPU6050_BAND_5_HZ    );  // 5, 10, 21, 44, 94, 184 or 260 Hz

  // Set default periodic rate
  SetRate (36000);  // 10 samples per second

  // No need for Immediate processing
  immediateEnabled = false;

  // Set Version for this Device
  strcpy (version, "2025.07.21a");  // no more than 11 chars
}

//--- DoPeriodic (override) -------------------------------

ProcessStatus Gyro::DoPeriodic ()
{
  // Get sensor readings
  gyro->getEvent (&accelSensor, &gyroSensor, &tempSensor);

  accelX = accelSensor.acceleration.x;
  accelY = accelSensor.acceleration.y;
  accelZ = accelSensor.acceleration.z;

  // Calculate orientation from accel and gyro
  pitch = atan2 (accelX, sqrt(accelY*accelY + accelZ*accelZ));  // radians,  * 57.3 for degrees
  roll  = atan2 (-accelY, accelZ);  // radians,  * 57.3 for degrees

  // Return readings
  DataPacket.timestamp = millis ();
  sprintf (DataPacket.value, "%.2f,%.2f", pitch, roll);

  return SUCCESS_DATA;
}
