//=========================================================
//
//     FILE : MyNode.cpp
//
//    PROJECT : SMAC Framework - Example 1
//
//   AUTHOR : Bill Daniels
//            Copyright 2021-2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

//--- Includes --------------------------------------------

#include "MyNode.h"
#include "LightSensor.h"
#include "Button.h"
#include "LED.h"

//--- Constructor -----------------------------------------

MyNode::MyNode (const char *name, unsigned char nodeID)
       :Node (name, nodeID)
{
  // Create and add all "Devices" to this Node using addDevice()
  // Each call to addDevice() will assign a 2-digit Device ID ("00", "01", "02", ...)
  // The Device ID is used by the SMAC Interface to visualize data

  addDevice (new LightSensor ("Light Sensor" , 6));  // Gets assigned Device ID 00
  addDevice (new Button      ("Little Button", 5));  // Gets assigned Device ID 01
  addDevice (new LED         ("Yellow LED"   , 4));  // Gets assigned Device ID 02
}
