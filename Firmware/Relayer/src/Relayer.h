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

#define SERIAL_BAUDRATE     115200  // Serial comms with the Serial Monitor
#define MAX_MESSAGE_LENGTH     250  // Max message size for ESP-NOW protocol

#define MAX_NODES               20  // Maximum number of ESP-NOW peers
#define MAC_SIZE                 6  // Size of ESP32 MAC Address
#define MAX_ESPNOW_LENGTH      250  // Max length of ESPNOW data
#define MIN_DATA_LENGTH          9  // Minimum Output Data String: nn|dd|t|d
#define MIN_COMMAND_LENGTH      10  // Minimum Input Command String: nn|dd|cccc


//=========================================================
//  class Relayer
//=========================================================

class Relayer
{
  protected:
    bool  startupStatus = false;
    char  dataString[MAX_ESPNOW_LENGTH+1];
    char  serial_CommandString[MAX_ESPNOW_LENGTH+1];
    int   serial_CommandLength = 0;
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
