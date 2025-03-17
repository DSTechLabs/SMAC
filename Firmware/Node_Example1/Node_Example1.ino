//=============================================================================
//
//       FILE : Node_Example1.ino
//
//    PROJECT : SMAC Framework - Example 1
//
//      NOTES : This is the firmware for the SMAC Node of Example 1.
//
//              About this template:
//              - The SMAC System uses Espressif's ESP-NOW protocol between Node Modules and the Relayer Module.
//              - Node and Device are the base classes from which your custom classes are derived.
//              - This "template" creates a child Node (MyNode) with a single Device, a LightSensor.
//              - Node Modules first attempt to connect to the Relayer Module.
//              - Once connected, the LightSensor Device "measures" a value and outputs a "Data String" with its value.
//              - The above operation is performed periodically to maintain continuous data.
//              - All Device data can be visualized with gauges and graphs using the SMAC Interface (a Chrome browser app).
//              - The SMAC System is bidirectional. You can send commands to both Nodes and individual Devices.
//              - Commands can be sent directly from the SMAC Interface using buttons, dials, sliders, etc.
//              - The Node and Device base classes handle standard commands and child classes can handle custom commands.
//
//              Classes in this example:
//
//              ∙ Node
//                  │
//                  └── MyNode -- An example Node Module
//
//              ∙ Device
//                  │
//                  └── LightSensor -- Demo to show how sensor data can be sent to the SMAC Interface
//
//  DEBUGGING : Set the global <Debugging> to true to see debugging info in Serial Monitor.
//              Be sure to set <Debugging> to false for production builds!
//
//     AUTHOR : Bill Daniels
//              Copyright 2021-2025, D+S Tech Labs, Inc.
//              All Rights Reserved
//
//=============================================================================

//--- Includes --------------------------------------------

#include <Arduino.h>
#include <Preferences.h>
#include "common.h"
#include "MyNode.h"

//--- Globals ---------------------------------------------

bool         Debugging = false;  // ((( Set to false for production builds )))
esp_err_t    ESPNOW_Result;
char         SerialMessage[MAX_MESSAGE_LENGTH];
char         NextChar;
int          SMLength = 0;
Preferences  MCUPreferences;  // Non-volatile memory
uint8_t      RelayerMAC[MAC_SIZE];  // MAC Address of the Relayer Module stored in non-volatile memory.
                                    // This is set using the <SetMAC.html> tool in the SMAC_Interface folder.
                                    // { 0x7C, 0xDF, 0xA1, 0xE0, 0x92, 0x98 }
bool         WaitingForRelayer = true;
RingBuffer   *CommandBuffer;
DPacket      DataPacket;
CPacket      CommandPacket;
char         DataString[MAX_MESSAGE_LENGTH];


//=========================================================
// Declare an instance for your custom Node
//=========================================================
//
// ┌─────────── The class name of your child Node
// │
// │        ┌── Don't change this
// │        │
MyNode  *ThisNode;
//=========================================================


//--- Declarations ----------------------------------------

void checkSerialInput ();
void processSerialMessage ();


//=========================================================
//  setup
//=========================================================

void setup()
{
  // Init built-in RGB LED, start off red
  neopixelWrite (STATUS_LED_PIN, STATUS_LED_BRIGHTNESS, 0, 0);

  // Init serial comms
  SerialMessage[0] = 0;
  Serial.begin (SERIAL_BAUDRATE);  // for debugging and <SetMAC> tool
  while (!Serial);

  Serial.println ("--- Program Start ----------------------");

  // Load the Relayer Module's MAC Address from non-volatile memory
  Serial.println ("Loading Relayer MAC Address ...");
  MCUPreferences.begin    ("RelayerMAC", false);
  MCUPreferences.getBytes ("RelayerMAC", RelayerMAC, sizeof(RelayerMAC));
  MCUPreferences.end      ();

  // Init Command buffer (a circular FIFO buffer)
  CommandBuffer = new RingBuffer (FIFO);

  Serial.println ("Starting the Node ...");



  //=======================================================
  // Create an instance of your custom Node here.
  // The 1st param is a name for your Node (to show in the SMAC Interface).
  // The 2nd param is the Node's unique ID (0-19).
  // Node ID's cannot be duplicated in your SMAC System.
  // --- Do not use the same ID for other Nodes ---
  //=======================================================
  ThisNode = new MyNode ("My First Node", 0);
  //=======================================================



  // PING the Relayer once per second until it responds with PONG
  Serial.println ("PINGing Relayer ...");
  strcpy (DataPacket.deviceID, "00");
  strcpy (DataPacket.value, "PING");
  unsigned long  nowSec, lastSec = 0L;
  WaitingForRelayer = true;
  while (WaitingForRelayer)
  {
    DataPacket.timestamp = millis ();
    nowSec = DataPacket.timestamp / 1000L;

    if (nowSec > lastSec)
    {
      ThisNode->SendDataPacket ();
      lastSec = nowSec;
    }

    // Check for Set MAC Tool
    checkSerialInput ();
  }

  // Relayer responded, All good, Go green
  Serial.println ("Relayer responded to PING");
  neopixelWrite (STATUS_LED_PIN, 0, STATUS_LED_BRIGHTNESS, 0);

  Serial.println ("Node running ...");
}


//=========================================================
//  loop
//  NO NEED TO CHANGE THIS CODE
//=========================================================

void loop()
{
  // Keep the Node running
  ThisNode->Run ();

  // Check for Set MAC Tool
  checkSerialInput ();
}


//=========================================================
//  checkSerialInput
//  NO NEED TO CHANGE THIS CODE
//=========================================================

void checkSerialInput ()
{
  // Check serial port for characters
  while (Serial.available ())
  {
    NextChar = (char) Serial.read ();
    if (NextChar != '\r')  // ignore CR's
    {
      if (NextChar == '\n')
      {
        // Message is ready, terminate string and process it
        SerialMessage[SMLength] = 0;
        processSerialMessage ();

        // Start new message
        SMLength = 0;
      }
      else
      {
        // Add char to end of buffer
        if (SMLength < MAX_MESSAGE_LENGTH - 1)
          SerialMessage[SMLength++] = NextChar;
        else  // too long
        {
          Serial.println ("ERROR: Serial message is too long.");

          // Ignore and start new message
          SMLength = 0;
        }
      }
    }
  }
}


//=========================================================
//  processSerialMessage
//  NO NEED TO CHANGE THIS CODE
//=========================================================

void processSerialMessage ()
{
  // Check if using <Set MAC> tool
  if (strcmp (SerialMessage, "SetRelayerMAC") == 0)
  {
    // Send current setting
    sprintf (DataString, "CurrentMAC=%02x:%02x:%02x:%02x:%02x:%02x", RelayerMAC[0], RelayerMAC[1], RelayerMAC[2], RelayerMAC[3], RelayerMAC[4], RelayerMAC[5]);
    Serial.println (DataString);
  }
  else if (strncmp (SerialMessage, "NewMAC=", 7) == 0)
  {
    if (strlen (SerialMessage) < 24)  // NewMAC=xx:xx:xx:xx:xx:xx
    {
      Serial.print   ("Invalid MAC Address: ");
      Serial.println (SerialMessage);
    }
    else
    {
      // Parse and set new MAC Address (xx:xx:xx:xx:xx:xx)
      for (int i=7, j=0; j<sizeof(RelayerMAC); i+=3, j++)
      {
        strncpy (DataString, SerialMessage+i, 2);  DataString[2] = 0;
        sscanf  (DataString, "%02x", RelayerMAC+j);
      }

      // Store new network credentials in non-volatile <preferences.h>
      MCUPreferences.begin    ("RelayerMAC", false);
      MCUPreferences.putBytes ("RelayerMAC", RelayerMAC, sizeof(RelayerMAC));
      MCUPreferences.end      ();

      // Respond
      Serial.println ("SetRelayerMAC-Success");

      // Blink the Status LED
      for (int i=0; i<10; i++)
      {
        neopixelWrite (STATUS_LED_PIN, STATUS_LED_BRIGHTNESS, STATUS_LED_BRIGHTNESS, STATUS_LED_BRIGHTNESS);
        delay (20);

        neopixelWrite (STATUS_LED_PIN, 0, 0, 0);
        delay (80);
      }

      // Reset the board
      esp_restart();
    }
  }
}
