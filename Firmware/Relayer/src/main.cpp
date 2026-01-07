//=============================================================================
//
//     FILE : main.cpp
//
//  PROJECT : SMAC Framework - Relayer
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
//            ∙ The Relayer passes data and commands between the Interface and
//              Remote Node Modules/Devices.
//
//            ∙ The Relayer connects to Nodes using Espressif's Wireless ESP-NOW WiFi protocol.
//
//            ∙ The Relayer connects to the SMAC Interface through direct USB Serial.
//
//            ∙ Messages from the SMAC Interface to the Relayer are "Command Strings"
//              and are forwarded directly to their target Node/Device for execution.
//
//            ∙ Messages from Nodes/Devices to the Relayer are usually "Data Strings"
//              and are forwarded to the SMAC Interface for near real-time display.
//              However, these Messages may also be Command Strings, which are
//              re-directed to their target Node/Device for execution and copied
//              to the SMAC Interface for Diagnostic display only.  This allows a Node
//              or Device to send a Command to another Node/Device.
//
//            ∙ Each Node in your SMAC system will need to know the Relayer's MAC Address.
//              Use the Set MAC Tool (setMAC.html in the Interface folder) to set the
//              Relayer's MAC address for each Node.
//
//   AUTHOR : Bill Daniels
//            Copyright 2021-2026, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=============================================================================

//--- Includes ---------------------------------------------

#include <Arduino.h>
#include "Relayer.h"

//--- Defines ----------------------------------------------

// Built-in LED on board
//-----------------------
// #define USE_MONO_LED   // Use this for boards with mono color leds
#define USE_RGB_LED       // Use this for boards with RGB leds

//--- Status LED ---
#if defined USE_MONO_LED && defined USE_RGB_LED
#error "ERROR: Only one of USE_MONO_LED or USE_RGB_LED may be defined"
#endif

#if (defined(USE_MONO_LED))
#define STATUS_LED_PIN         LED_BUILTIN
#define STATUS_LED_BAD         (digitalWrite (STATUS_LED_PIN, LOW))
#define STATUS_LED_GOOD        (digitalWrite (STATUS_LED_PIN, HIGH))

#elif (defined(USE_RGB_LED))
#define STATUS_LED_PIN         48  // GPIO-48 for v1.0 boards, GPIO-38 for v1.1 boards, GPIO-18 for S2-DevKit
#define STATUS_LED_BRIGHTNESS  20  // Not recommended above 64
#define STATUS_LED_BAD         (rgbLedWrite (STATUS_LED_PIN, STATUS_LED_BRIGHTNESS, 0, 0))
#define STATUS_LED_GOOD        (rgbLedWrite (STATUS_LED_PIN, 0, STATUS_LED_BRIGHTNESS, 0))
#endif

//--- Globals ----------------------------------------------

Relayer  *TheRelayer;


//==========================================================
//  setup
//==========================================================

void setup()
{
  // Init built-in RGB LED, start off red
  pinMode (STATUS_LED_PIN, OUTPUT);
  STATUS_LED_BAD;

  // Instantiate the Relayer
  TheRelayer = new Relayer ();
  if (TheRelayer->IsOkay ())
  {
    // All good, go green
    STATUS_LED_GOOD;
  }
}


//==========================================================
//  loop
//==========================================================

void loop()
{
  TheRelayer->Run ();
}
