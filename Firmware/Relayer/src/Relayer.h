//=========================================================
//
//     FILE : Relayer.h
//
//  PROJECT : SMAC Framework
//
//   AUTHOR : Bill Daniels
//            Copyright 2021-2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

#ifndef RELAYER_H
#define RELAYER_H

//--- Defines ---------------------------------------------

#define SERIAL_BAUDRATE      115200  // Serial comms with the Serial Monitor
#define MAX_MESSAGE_LENGTH      250  // Max message size for ESP-NOW protocol

#define MAX_NODES                20  // Maximum number of ESP-NOW peers
#define MAC_SIZE                  6  // Size of ESP32 MAC Address
#define COMMAND_SIZE              4  // Size of SMAC Commands
#define VC_OFFSET                 8  // Values or Command Offset into Data or Command String
#define MAX_ESPNOW_LENGTH       250  // Max length of ESP-NOW strings
#define MIN_DATA_LENGTH           9  // Minimum ESP-NOW Data String    : D|nn|dd|values
#define MAX_DATA_LENGTH         238  // Maximum ESP-NOW Data String    : D|nn|dd|values...|timestamp
#define MIN_COMMAND_LENGTH       12  // Minimum ESP-NOW Command String : C|nn|dd|cccc
#define MAX_TIMESTAMP_LENGTH     12  // A '|' char and timestamp is appended to Data strings
                                     // before relaying to the SMAC Interface

//=========================================================
//  class Relayer
//=========================================================

class Relayer
{
  protected:
    bool  startupStatus = false;
    char  commandString[MAX_ESPNOW_LENGTH+1] = "";
    int   commandLength = 0;
    char  serial_NextChar;

    void serial_CheckInput        ();
    void serial_ProcessCommand    ();
    void espnow_SendCommandString ();

  public:
    Relayer      ();
    bool  IsOkay ();
    void  Run    ();
};

#endif
