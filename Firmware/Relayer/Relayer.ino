//=============================================================================
//
//     FILE : Relayer.ino
//
//  PROJECT : SMAC Framework
//              │
//              └── Publish
//                    │
//                    └── Firmware
//                          │
//                          └── Relayer
//
//    NOTES : This is the firmware for the SMAC Relayer Module.
//
//            ∙ The MCU board used for the Relayer is the Espressif ESP32-S3-DevKitC-1.
//              The MAC Address for this board is 7C:DF:A1:E0:92:98
//
//            ∙ The Relayer has an RGB "Status" LED:
//              · Off   = No power/program not running
//              · Red   = Error booting (see Serial Monitor for messages)
//              · Green = ESP-NOW and Serial comms started, module ready and running
//
//            ∙ The Relayer passes data and messages between the Interface and
//              Remote Node Modules/Devices.
//
//            ∙ The Relayer connects to Nodes using Espressif's Wireless ESP-NOW WiFi protocol.
//
//            ∙ The Relayer connects to the Interface through direct USB Serial.
//
//            ∙ Messages from the Interface to the Relayer and Nodes
//              are "Command Strings".
//
//            ∙ Messages from Nodes/Devices through the Relayer and to the Interface
//              are "Data Strings".
//
//            ∙ Each Node in your SMAC system will need to know the Relayer's MAC Address.
//              Use the Set MAC Tool (setMAC.html in the Interface folder) to set the
//              Relayer's MAC address for each Node.
//
//   AUTHOR : Bill Daniels
//            Copyright 2021-2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=============================================================================

//--- Includes ---------------------------------------------

#include <Arduino.h>
#include "Relayer.h"

//--- Defines ----------------------------------------------

// For the Espressif ESP32-S3-DevKitC-1 board, the built-in LED
// is a 1-element addressable string of RGB LEDs of type WS2812.
// Use neopixelWrite (pin, red, green, blue) to operate.
#define LED_BUILTIN_PIN         48  // GPIO-48 for Rev1.0 boards, GPIO-38 for Rev1.1 boards
#define LED_BUILTIN_BRIGHTNESS  20  // Set brightness: O-255 (not recommended above 40)

//--- Globals ----------------------------------------------

Relayer  *TheRelayer;
bool     newNode = false;


//==========================================================
//  setup
//==========================================================

void setup()
{
  // Init built-in RGB LED, start off red
  neopixelWrite (LED_BUILTIN_PIN, LED_BUILTIN_BRIGHTNESS, 0, 0);

  // Instantiate the Relayer
  TheRelayer = new Relayer ();
  if (TheRelayer->IsOkay ())
  {
    // All good, go green
    neopixelWrite (LED_BUILTIN_PIN, 0, LED_BUILTIN_BRIGHTNESS, 0);
  }
}


//==========================================================
//  loop
//==========================================================

void loop()
{
  TheRelayer->Run ();
}
