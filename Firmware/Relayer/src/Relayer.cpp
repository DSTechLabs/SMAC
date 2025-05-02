//=========================================================
//
//     FILE : Relayer.cpp
//
//  PROJECT : SMAC Framework
//              │
//              └── Publish
//                    │
//                    └── Firmware
//                          │
//                          └── Relayer
//
//   AUTHOR : Bill Daniels
//            Copyright 2021-2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

//--- Includes --------------------------------------------

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "Relayer.h"

//--- Globals ---------------------------------------------

esp_err_t            ESPNOW_Result;
esp_now_peer_info_t  PeerInfo = {};                  // Required to set new ESP-NOW peers
uint8_t              NodeMACs[MAX_NODES][MAC_SIZE];  // Each node MAC address is 6 bytes (xx:xx:xx:xx:xx:xx)
bool                 ProcessingData = false;         // Used to block from reentering espnow_ProcessDataString()
int                  NodeIndex;                      // Global for performance

//--- Declarations ----------------------------------------

void espnow_ProcessDataString (const esp_now_recv_info_t *info, const uint8_t *dataString, int dataLength);

extern bool  newNode;

//--- Constructor -----------------------------------------

Relayer::Relayer ()
{
  // Init Serial comms with the SMAC Interface
  Serial.begin (SERIAL_BAUDRATE);

  // Init all Nodes as unregistered (using first byte as a flag)
  for (NodeIndex=0; NodeIndex<MAX_NODES; NodeIndex++)
    NodeMACs[NodeIndex][0] = 0xFF;

  // Init peer info structure for adding new peers
  PeerInfo.channel = 0;
  PeerInfo.encrypt = false;

  // Init ESP-NOW comms with remote Nodes
  // Set this device as a Wi-Fi Station
  if (!WiFi.mode (WIFI_STA))
  {
    Serial.println ("ERROR: Unable to set WiFi mode.");
    return;
  }



  // // Set WiFi rate ???  5-GHz ???
  // // Load this Node's MAC address
  // uint8_t  macAddress[MAC_SIZE+2];
  // for (int i=0; i<MAC_SIZE+2; i++) macAddress[i] = 0;  // Clear result first
  // esp_efuse_mac_get_default (macAddress);  // System-programmed by Espressif, 6 bytes
  // // --OR--
  // macAddress = WiFi.macAddress ();
  // esp_now_rate_config_t  rateConfig = { ... };
  // esp_now_set_peer_rate_config (macAddress, &rateConfig);



  // Init ESP-NOW protocol
  ESPNOW_Result = esp_now_init ();
  if (ESPNOW_Result != ESP_OK)
  {
    Serial.print   ("ERROR: Unable to initialize ESP-NOW protocol: ");
    Serial.println (ESPNOW_Result);
    return;
  }

  // Register receive event
  ESPNOW_Result = esp_now_register_recv_cb ((esp_now_recv_cb_t) &espnow_ProcessDataString);
  if (ESPNOW_Result != ESP_OK)
  {
    Serial.print   ("ERROR: Unable to register ESP-NOW data listener: ");
    Serial.println (ESPNOW_Result);
    return;
  }

  // Init command string
  serial_CommandString[0] = 0;

  // Good to go
  Serial.print   ("Relayer is running: MAC=");
  Serial.println (WiFi.macAddress ());

  startupStatus = true;
}

//--- IsOkay ----------------------------------------------

bool Relayer::IsOkay ()
{
  return startupStatus;
}

//--- Run -------------------------------------------------

void Relayer::Run ()
{
  // Process any Interface commands from Serial port
  serial_CheckInput ();
}

//--- serial_CheckInput -----------------------------------

void Relayer::serial_CheckInput ()
{
  // Check serial port for commands from the Interface
  while (Serial.available ())
  {
    serial_NextChar = (char) Serial.read ();

    if (serial_NextChar != '\r')  // ignore CR's
    {
      if (serial_NextChar == '\n')
      {
        // Command is ready, terminate string and process it
        serial_CommandString[serial_CommandLength] = 0;

        serial_ProcessCommand ();

        // Start new message
        serial_CommandLength = 0;
      }
      else
      {
        // Add char to end of buffer
        if (serial_CommandLength < MAX_MESSAGE_LENGTH - 1)
          serial_CommandString[serial_CommandLength++] = serial_NextChar;
        else  // too long
        {
          Serial.println ("ERROR: Serial message is too long.");

          // Ignore and start new message
          serial_CommandLength = 0;
        }
      }
    }
  }
}

//--- serial_ProcessCommand -------------------------------

void Relayer::serial_ProcessCommand ()
{
  // Incoming Command strings from the SMAC Interface have
  // the following format: (fields are separated with the '|' char)
  //
  //   ┌─────────────── 2-char nodeID (00-19)
  //   │  ┌──────────── 2-char deviceID (00-99)
  //   │  │   ┌──────── 4-char command (usually capital letters)
  //   │  │   │     ┌── Optional variable length parameter string
  //   │  │   │     │
  //   nn|dd|CCCC|params

  // Check Command String length
  if (serial_CommandLength < MIN_COMMAND_LENGTH)
    Serial.println ("ERROR: Invalid Command String.");
  else
  {
    // First check if requesting MAC address (from the Set MAC Tool)
    if (strncmp (serial_CommandString+6, "GMAC", 4) == 0)
    {
      Serial.print   ("MAC=");
      Serial.println (WiFi.macAddress ());
    }
    // Then check if the Interface is requesting all Node and Device Info (System Info)
    else if (strncmp (serial_CommandString+6, "SYSI", 4) == 0)
    {
      // Request Node and Device info from all registered Nodes
      for (NodeIndex=0; NodeIndex<MAX_NODES; NodeIndex++)
      {
        if (NodeMACs[NodeIndex][0] != 0xFF)
        {
          sprintf (serial_CommandString, "%02d|00|GNOI", NodeIndex);  // Get Node Info
          espnow_SendCommandString ();

          sprintf (serial_CommandString, "%02d|00|GDEI", NodeIndex);  // Get Device Info
          espnow_SendCommandString ();
        }
      }
    }
    else
    {
      //================================================
      // Relay <CommandString> to specified Node/Device
      //================================================
      espnow_SendCommandString ();
    }
  }
}

//--- espnow_SendCommandString ----------------------------

void Relayer::espnow_SendCommandString ()
{
  // A Command String has four fields separated with the '|' char:
  //
  //   ┌─────────────── 2-char nodeID (00-19)
  //   │  ┌──────────── 2-char deviceID (00-99)
  //   │  │   ┌──────── 4-char command (usually capital letters)
  //   │  │   │     ┌── variable length parameter string (including NULL terminating char)
  //   │  │   │     │
  //   nn|dd|CCCC|params

  // Check if Node exists and command is valid
  NodeIndex = 10*((int)(serial_CommandString[0])-48) + ((int)(serial_CommandString[1])-48);
  if (NodeMACs[NodeIndex][0] == 0xFF)
    Serial.println ("ERROR: Unable to send command; Node does not exist.");
  else
  {
    //========================================
    // Relay command to specified Node/Device
    //========================================
    // Since we are sending directly to a specific Node, we don't need to send the nodeID|
    // So send command string starting with deviceID (offset by +3)
    ESPNOW_Result = esp_now_send (NodeMACs[NodeIndex], (const uint8_t *)serial_CommandString + 3, strlen(serial_CommandString + 3) + 1);
    if (ESPNOW_Result != ESP_OK)
    {
      Serial.print   ("ERROR: Unable to send Command String: ");
      Serial.println (ESPNOW_Result);
    }
  }
}


//=========================================================
// External ESP-NOW "C" Functions
//=========================================================

//--- espnow_ProcessDataString ----------------------------

void espnow_ProcessDataString (const esp_now_recv_info_t *info, const uint8_t *dataString, int dataLength)
{
  // A Data String has four fields separated with the '|' char:
  //
  //   ┌──────────────────── 2-char nodeID (00-19)
  //   │  ┌───────────────── 2-char deviceID (00-99)
  //   │  │     ┌─────────── variable length timestamp (usually millis())
  //   │  │     │        ┌── variable length value string (including NULL terminating char)
  //   │  │     │        │   this can be a numerical value or a text message
  //   │  │     │        │
  //   nn|dd|timestamp|value
  //
  // Data Strings must be NULL terminated!

  // Check if being called again before finishing.
  // If you see this message then reduce the periodic
  // data rate for your devices.
  if (ProcessingData)
  {
    Serial.println ("ERROR: Data overflow.");
    ProcessingData = false;
    return;
  }

  // Lock
  ProcessingData = true;

  // Check data length
  if (dataLength < MIN_DATA_LENGTH)
    Serial.println ("ERROR: Invalid Node/Device data string.");
  else
  {
    // Check if this came from an unregistered Node
    NodeIndex = 10*((int)(dataString[0])-48) + ((int)(dataString[1])-48);
    if (NodeMACs[NodeIndex][0] == 0xFF)
    {
      newNode = true;
      uint8_t *nodeMAC = (*info).src_addr;

      // Haven't heard from this Node yet, so register it as a new peer
      // Save MAC address in array
      memcpy (NodeMACs[NodeIndex], nodeMAC, MAC_SIZE);

      // Register new Node as an esp_now peer
      memcpy (PeerInfo.peer_addr, nodeMAC, MAC_SIZE);
      ESPNOW_Result = esp_now_add_peer (&PeerInfo);
      if (ESPNOW_Result != ESP_OK)
      {
        Serial.print   ("ERROR: Unable to add Node as ESP-NOW peer: ");
        Serial.println (ESPNOW_Result);
        return;
      }
    }
    else if (strcmp ((const char *) dataString + dataLength-5, "PING") == 0)
      newNode = true;

    if (newNode)
    {
      newNode = false;

      // Send message to Interface to indicate
      // that a new Node has connected: NODE|nodeID
      Serial.print   ("NODE|");
      Serial.println (NodeIndex);

      // Send back a "PONG" to the Node
      esp_now_send (NodeMACs[NodeIndex], (const uint8_t *) "PONG", 5);
    }


    //================================
    // Relay Data String to Interface
    //================================
    Serial.println ((char *) dataString);
  }

  // Unlock
  ProcessingData = false;
}
