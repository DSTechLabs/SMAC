//=========================================================
//
//     FILE : common.h
//
//  PROJECT : SMAC Framework
//
//   AUTHOR : Bill Daniels
//            Copyright 2021-2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

#ifndef COMMON_H
#define COMMON_H

//--- Includes --------------------------------------------

#include <Arduino.h>
#include <esp_now.h>
#include "RingBuffer.h"

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
#define STATUS_LED_PIN         38  // GPIO-48 for v1.0 boards, GPIO-38 for v1.1 boards
#define STATUS_LED_BRIGHTNESS  20  // Not recommended above 64
#define STATUS_LED_BAD         (rgbLedWrite (STATUS_LED_PIN, STATUS_LED_BRIGHTNESS, 0, 0))
#define STATUS_LED_GOOD        (rgbLedWrite (STATUS_LED_PIN, 0, STATUS_LED_BRIGHTNESS, 0))
#endif

//--- Common Stuff ---
#define SERIAL_BAUDRATE      115200
#define SERIAL_MAX_LENGTH        80
#define MAX_VERSION_LENGTH       12  // Suggested format: yyyy.mm.dd(a-z)
#define MAX_NODES                20  // Maximum number of ESP-NOW peers
#define MAX_DEVICES             100  // Maximum number of Devices that can connect to a Node
#define ID_SIZE                   2  // Size of nodeID and deviceID
#define MAX_NAME_LENGTH          32
#define MAX_MESSAGE_LENGTH      250  // Max message size for ESP-NOW protocol
#define MAC_SIZE                  6  // Size of ESP32 MAC Address
#define MAX_VALUE_LENGTH        240
#define MAX_PARAMS_LENGTH       240
#define MIN_COMMAND_LENGTH        7  // Minimum Input Command String: dd|cccc
#define COMMAND_SIZE              4

//--- Types -----------------------------------------------

typedef struct DPacket
{
  char           deviceID[ID_SIZE + 1];
  unsigned long  timestamp;
  char           value[MAX_VALUE_LENGTH + 1];
} DPacket;

typedef struct CPacket
{
  int   deviceIndex;
  char  command[COMMAND_SIZE + 1];
  char  params[MAX_PARAMS_LENGTH + 1];
} CPacket;

enum ProcessStatus
{
  SUCCESS_DATA,    // Process performed successfully, send <DataPacket> to Relayer
  SUCCESS_NODATA,  // Process performed successfully, no data to send to Relayer
  FAIL_DATA,       // Process failed, error code or message stored in value field of <DataPacket>, send <DataPacket> to Relayer
  FAIL_NODATA,     // Process failed, no data to send to Relayer
  NOT_HANDLED      // Command was not handled by base class ExecuteCommand(), child Node or Device class should handle the command
};

//--- Externs ---------------------------------------------

extern bool        Debugging;
extern esp_err_t   ESPNOW_Result;
extern uint8_t     RelayerMAC[];
extern RingBuffer  *CommandBuffer;
extern DPacket     DataPacket;
extern CPacket     CommandPacket;
extern char        DataString[];

#endif
