//=========================================================
//
//     FILE : Relayer.cpp
//
//  PROJECT : SMAC Framework
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
#include <esp_wifi.h>
#include "Relayer.h"

//--- Globals ---------------------------------------------

char                 Version[] = "2025-11-01";
esp_err_t            ESPNOW_Result;                   // Error code from ESP-NOW functions
esp_now_peer_info_t  PeerInfo = {};                   // Required to set new ESP-NOW peers
uint8_t              NodeMACs[MAX_NODES][MAC_SIZE];   // Each node MAC address is 6 bytes (xx:xx:xx:xx:xx:xx)
bool                 ProcessingESPNOWString = false;  // Used to block from reentering ESPNOW_Receiver()
int                  NodeIndex;                       // Global for performance
bool                 NewNode = false;
char                 DataString[MAX_ESPNOW_LENGTH];
char                 TimestampField[MAX_TIMESTAMP_LENGTH+1] = "|";  // First char must be '|'

//--- Declarations ----------------------------------------

void ESPNOW_Receiver (const esp_now_recv_info_t *info, const uint8_t *espnowString, int stringLength);

//--- Constructor -----------------------------------------

Relayer::Relayer ()
{
  // Init Serial comms
  Serial.begin   (SERIAL_BAUDRATE);
  Serial.print   ("\nSMAC Relayer Version ");
  Serial.println (Version);
  Serial.println ("----------------------------------------");

  // Init all Nodes as unregistered (using first byte as a flag)
  for (NodeIndex=0; NodeIndex<MAX_NODES; NodeIndex++)
    NodeMACs[NodeIndex][0] = 0xFF;

  // Init ESP-NOW comms with remote Nodes
  // Set this device as a Wi-Fi Station
  if (!WiFi.mode (WIFI_STA))
  {
    Serial.println ("ERROR: Unable to set WiFi mode.");
    return;
  }
  delay (100);

  // Init peer info structure for adding new peers
  Serial.print ("WiFi Channel is "); Serial.println (WiFi.channel());
  PeerInfo.channel = 0;  // 0 = Auto-select channel
  PeerInfo.encrypt = false;



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
  ESPNOW_Result = esp_now_register_recv_cb ((esp_now_recv_cb_t) &ESPNOW_Receiver);
  if (ESPNOW_Result != ESP_OK)
  {
    Serial.print   ("ERROR: Unable to register ESP-NOW data listener: ");
    Serial.println (ESPNOW_Result);
    return;
  }

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
        commandString[commandLength] = 0;

        serial_ProcessCommand ();

        // Start new message
        commandLength = 0;
      }
      else
      {
        // Add char to end of buffer
        if (commandLength < MAX_MESSAGE_LENGTH - 1)
          commandString[commandLength++] = serial_NextChar;
        else  // too long
        {
          Serial.println ("ERROR: Serial message to Relayer is too long.");

          // Ignore and start new message
          commandLength = 0;
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
  //   ┌───────────────── 'C' for Command
  //   │ ┌─────────────── 2-char target nodeID (00-19)
  //   │ │  ┌──────────── 2-char target deviceID (00-99)
  //   │ │  │   ┌──────── 4-char command (usually capital letters)
  //   │ │  │   │     ┌── Optional variable length parameter string
  //   │ │  │   │     │
  //   C|nn|dd|CCCC|params

  // Check Command String length
  if (commandLength < MIN_COMMAND_LENGTH)
    Serial.println ("ERROR: Invalid Command String.");
  else
  {
    // First check if requesting MAC address (from the Set MAC Tool)
    if (strncmp (commandString + VC_OFFSET, "GMAC", COMMAND_SIZE) == 0)
    {
      Serial.print   ("MAC=");
      Serial.println (WiFi.macAddress ());
    }
    // Then check if the Interface is requesting all Node and Device Info (System Info)
    else if (strncmp (commandString + VC_OFFSET, "SYSI", COMMAND_SIZE) == 0)
    {
      // Request Node and Device info from all registered Nodes
      for (NodeIndex=0; NodeIndex<MAX_NODES; NodeIndex++)
      {
        if (NodeMACs[NodeIndex][0] != 0xFF)
        {
          sprintf (commandString, "C|%02d|--|GNOI", NodeIndex);  // Get Node Info
          espnow_SendCommandString ();

          sprintf (commandString, "C|%02d|--|GDEI", NodeIndex);  // Get Device Info
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
  // Check if Node exists and command is valid
  NodeIndex = 10*((int)(commandString[2])-48) + ((int)(commandString[3])-48);
  if (NodeIndex < MAX_NODES)
  {
    if (NodeMACs[NodeIndex][0] != 0xFF)
    {
      //========================================
      // Relay command to specified Node/Device
      //========================================
      ESPNOW_Result = esp_now_send (NodeMACs[NodeIndex], (const uint8_t *)commandString, strlen(commandString) + 1);
      if (ESPNOW_Result != ESP_OK)
      {
        Serial.print   ("ERROR: Unable to send Command String: ");
        Serial.println (ESPNOW_Result);
      }

      return;
    }
  }

  Serial.println ("ERROR: Unable to send command; Node does not exist.");
}


//=========================================================
// External "C" Functions
//=========================================================

//--- ESPNOW_Receiver -------------------------------------

IRAM_ATTR void ESPNOW_Receiver (const esp_now_recv_info_t *info, const uint8_t *espnowString, int stringLength)
{
  // The Relayer can receive both Data and Commands from Nodes and Devices.
  // Data strings are relayed to the SMAC Interface with "|timestamp" appended.
  // Command strings are relayed to the target Node and Device, or broadcasted.
  //
  //-------------------------
  //  Data string format:
  //-------------------------
  //
  //   ┌──────────── 1-char packet type ('D' for Data)
  //   │ ┌────────── 2-char source nodeID (00-19)
  //   │ │  ┌─────── 2-char source deviceID (00-99)
  //   │ │  │    ┌── variable length string of values (multiple values are comma delimited)
  //   │ │  │    │
  //   D|nn|dd|values
  //
  // A bar char '|" and a variable length timestamp field is appended to the string
  // before being relayed to the SMAC Interface.
  //
  // Special Data:
  // --------------------------------
  //   PING - A new Node just started and is waiting for a PONG from the Relayer
  //
  //-------------------------
  //  Command string format:
  //-------------------------
  //
  //   ┌───────────────── 1-char packet type ('C' for Command)
  //   │ ┌─────────────── 2-char target nodeID (00-19), if "--" then the command is broadcasted
  //   │ │  ┌──────────── 2-char target deviceID (00-99)
  //   │ │  │   ┌──────── 4-char command (usually capital letters)
  //   │ │  │   │     ┌── optional variable length string of parameters (multiple params are comma delimited)
  //   │ │  │   │     │
  //   C|nn|dd|cccc|params
  //
  // Special Commands:
  // --------------------------------
  //   WFCH|n - A Node has requested every ESP-NOW peer to change their WiFi Channel to n (0-14)
  //            This Command String is broadcasted to all peers including this Relayer


  // ASAP, Set timestamp field that gets appended to Data strings
  ltoa (millis(), TimestampField + 1, 10);  // +1 to skip over '|' char

  // Check if being called again before finishing.
  // If you see this message then reduce the periodic
  // data rate for your devices.
  if (ProcessingESPNOWString)
  {
    Serial.println ("ERROR: ESP-NOW Message overflow.");
    return;
  }

  // Lock
  ProcessingESPNOWString = true;

  // Check minimum string length
  if (stringLength < MIN_DATA_LENGTH)
  {
    Serial.println ("ERROR: Node/Device String too short.");
    ProcessingESPNOWString = false;
    return;
  }

  // Get the Node index
  NodeIndex = 10*((int)(espnowString[2])-48) + ((int)(espnowString[3])-48);
  if (NodeIndex >= MAX_NODES)
  {
    Serial.println ("ERROR: Invalid NodeID in Node message.");
    ProcessingESPNOWString = false;
    return;
  }

  //===================================
  // Handle Data strings
  //===================================
  if ((char) espnowString[0] == 'D')
  {
    NewNode = false;

    // Handle "PING" from a new Node: D|nn|--|PING
    if (strcmp ((const char *) espnowString + VC_OFFSET, "PING") == 0)
    {
      NewNode = true;

      // Send back a "PONG" to the Node
      esp_now_send (NodeMACs[NodeIndex], (const uint8_t *) "PONG", COMMAND_SIZE);
    }
    else
    {
      // Check if this Data string came from an unregistered Node
      if (NodeMACs[NodeIndex][0] == 0xFF)
        NewNode = true;

      //=====================================================
      // Append timestamp and relay Data String to Interface
      //=====================================================
      memcpy (DataString, espnowString, stringLength);
      DataString[stringLength] = 0;
      strcat (DataString, TimestampField);  // TimestampField includes the initial '|' char
      Serial.println (DataString);
    }

    // Add new Node, if any
    if (NewNode)
    {
      // Haven't heard from this Node yet, so register it as a new peer
      // Save MAC address in array
      uint8_t *nodeMAC = (*info).src_addr;
      memcpy (NodeMACs[NodeIndex], nodeMAC, MAC_SIZE);

      // Register new Node as an esp_now peer (if not already a peer)
      if (!esp_now_is_peer_exist (PeerInfo.peer_addr))
      {
        memcpy (PeerInfo.peer_addr, nodeMAC, MAC_SIZE);
        ESPNOW_Result = esp_now_add_peer (&PeerInfo);
        if (ESPNOW_Result != ESP_OK)
        {
          Serial.print   ("ERROR: Unable to add Node as ESP-NOW peer: ");
          Serial.println (ESPNOW_Result);
        }
      }

      // Send special message to Interface to indicate
      // that a new Node has connected: NODE|nn
      sprintf (DataString, "NODE|%02d", NodeIndex);
      Serial.println (DataString);
    }
  }

  //===================================
  // Handle Command strings
  //===================================
  else if ((char) espnowString[0] == 'C')
  {
    // Echo command to Interface (for Diagnostic Monitor)
    memcpy (DataString, espnowString, stringLength);
    DataString[stringLength] = 0;
    Serial.println (DataString);

    // Check data length
    if (stringLength < MIN_COMMAND_LENGTH)
      Serial.println ("ERROR: Node/Device Command String too short.");
    else if (strncmp ((const char *) espnowString + VC_OFFSET, "WFCH", COMMAND_SIZE) == 0)
    {
      // Change WiFi Channel
      uint8_t newChannel = (uint8_t) atoi ((const char *) espnowString + 13);
      Serial.print ("New WiFi Channel rquested; Changing to channel "); Serial.println (newChannel);
      esp_wifi_set_channel (newChannel, WIFI_SECOND_CHAN_NONE);
    }
    else
    {
      // Check if target Node exists
      if (NodeMACs[NodeIndex][0] != 0xFF)
      {
        //=====================================================
        // Relay Command String to target Node/Device
        //=====================================================
        esp_now_send (NodeMACs[NodeIndex], (const uint8_t *) espnowString, stringLength);
      }
    }
  }

  else
    Serial.println ("ERROR: Unknown ESP-NOW Message string.");

  // Unlock
  ProcessingESPNOWString = false;
}
