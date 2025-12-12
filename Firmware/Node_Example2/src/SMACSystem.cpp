//=============================================================================
//
//       FILE : System.cpp
//
//    PROJECT : SMAC Framework - Example 1
//
//      NOTES : This is the PIO firmware for the SMAC Node of Example 1.
//
//              About this template:
//              - The SMAC System uses Espressif's ESP-NOW protocol between Node Modules and the Relayer Module.
//              - Device is the base class from which your custom Devices are derived.
//              - This "template" creates a Node with a single Device, a LightSensor.
//              - Node Modules first attempt to connect to the Relayer Module.
//              - Once connected, the LightSensor Device "measures" a value and outputs a "Data String" with its value.
//              - The above operation is performed periodically to maintain continuous data.
//              - All Device data can be visualized with gauges and graphs using the SMAC Interface (a Chrome browser app).
//              - The SMAC System is bidirectional. You can send commands to both Nodes and individual Devices.
//              - Commands can be sent directly from the SMAC Interface using buttons, dials, sliders, etc.
//              - The Node and Device base classes handle standard commands and child classes can handle custom commands.
//
//              Devices in this example:
//
//                LightSensor -- Demo to show how sensor data can be sent to the SMAC Interface
//
//  DEBUGGING : Set the global <Debugging> to true to see debugging info in Serial Monitor.
//              Be sure to set <Debugging> to false for production builds!
//
//     AUTHOR : Bill Daniels
//              Copyright 2025, D+S Tech Labs, Inc.
//              All Rights Reserved
//
//=============================================================================

//--- Includes --------------------------------------------

#include "SMACSystem.h"

// Place your Device includes here
#include "LightSensor.h"
#include "Button.h"
#include "LED.h"


//--- Constructor -----------------------------------------

SMACSystem::SMACSystem ()
{
  // SMAC Systems can have up to 20 Nodes.
  // Set the Name and NodeID for the ESP32 module (0-19).
  // The NodeID's for a SMAC Systems with multiple Nodes
  // must be unique and cannot be duplicated.

  //--- Create the Node Instance ---
  //                           ┌───────────── The Name of your Node
  //                           │         ┌─── The Unique Node Index (0-19)
  //                           │         │
  ThisNode = new Node ("My Second Node", 0);



  //=======================================================
  // Create or Start/Begin any infrastructure your Devices
  // may need. Then, if necessary, pass those references
  // to your Devices' constructors.
  //=======================================================
  // ...



  //=======================================================
  // Add all Devices to your Node
  //=======================================================
  ThisNode->AddDevice (new LightSensor ("Light Sensor" , 6));  // Gets assigned Device ID 00
  ThisNode->AddDevice (new Button      ("Little Button", 5));  // Gets assigned Device ID 01
  ThisNode->AddDevice (new LED         ("Yellow LED"   , 4));  // Gets assigned Device ID 02
}

//--- AuxLoop ---------------------------------------------

void SMACSystem::AuxLoop ()
{
  // If you have any "loop()" code that is outside the SMAC System, place it here.
  // It will be called in the main loop() function.


}
