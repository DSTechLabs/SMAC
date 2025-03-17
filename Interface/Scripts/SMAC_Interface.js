//=========================================================
//
//     FILE : SMAC_Interface.js
//
//  PROJECT : SMAC Interface
//
//  PURPOSE : Main SMAC Relayer/Node/Device Communications
//
//   AUTHOR : Bill Daniels
//            Copyright 2014-2025, D+S Tech Labs, Inc.
//            All Rights Reserved
//
//=========================================================

//--- Globals ---------------------------------------------

const AppVersion = '── 2025.03.13 ──';
const Debugging  = false;  // Set to false for production use

let TotalPages  = 0;
let PagesLoaded = false;

const SerialPortSettings =
{
  baudRate    : 115200,
  dataBits    : 8,
  stopBits    : 1,
  parity      : 'none',
  bufferSize  : 1024,
  flowControl : 'none'
};

const EOL = '\r\n';

// SerialPort object (to be created if supported)
let SMACPort = undefined;

let DataString = '';

//--- Node Array: ---
const MaxNodes = 20;
let   Nodes = [];  // The index is the nodeID
// Holds Node objects:
// {                                         ┌─
//   name,                                   │  {
//   version,                                │    name,
//   macAddress,                             │    ipEnabled,
//   numDevices,                             │    ppEnabled,
//   devices[],  // array of Device objects ─┤    rate
//   monitor,                                │  }
//   lastMessageTime                         └─
// }


//--- Startup ---------------------------------------------

try
{
  // Set version strings
  $('#title_Version'  ).html (AppVersion);
  $('#dsSplashVersion').html (AppVersion);

  // Check if this browser supports serial communication
  if (!('serial' in navigator) || navigator.serial == undefined)
    $('#smacPageArea').html ('<h1 style="color:#C00000; text-shadow:1px 1px 1px #000000; text-align:center">' +
                             'This browser does not support serial communications.<br>Please use the Chrome or Edge browser.</h1>');
  else
  {
    // Create a SerialPort object
    SMACPort = new SerialPort (async (dataString) => { await ProcessRelayerMessage (dataString); }, RelayerDisconnected);

    // Show the Relayer connection screen
    $('#dsSplash').show ();

    // Close serial port before exit
    window.addEventListener ('beforeunload', async (event) =>
    {
      StopEvent (event);
      event.returnValue = '';

      if (DTInterval != undefined)
        clearInterval (DTInterval);

      await CloseDataLog ();
      await SMACPort.Close ();

      return undefined;
    });

    // Disable right-click context menu for entire SMAC console
    document.addEventListener ('contextmenu', (event) => { event.preventDefault(); });
  }
}
catch (ex)
{
  ShowException (ex);
}

//--- UpdateConnectionBox ---------------------------------

function UpdateConnectionBox ()
{
  try
  {
    if (SMACPort.IsOpen ())
    {
      const cb = $('#connectionBox');
      cb.css ('color', '#F0F0F0');
      cb.css ('background-color', '#008000');
      $('#cbStatus').text ('Connected');
    }
    else
    {
      const cb = $('#connectionBox');
      cb.css ('color', '#808080');
      cb.css ('background-color', '#303030');
      $('#cbStatus').text ('Not Connected');
    }
  }
  catch (ex)
  {
    ShowException (ex);
  }
}

//--- ConnectToRelayer ------------------------------------

async function ConnectToRelayer ()
{
  try
  {
    await ConnectToRelayer_Async ();

    if (SMACPort.IsOpen ())
    {
      // Hide splash
      $('#dsSplash').hide ();





      // GoFullscreen ();





      window.devicePixelRatio = 1.0;  // set pixel density to 1

      // Load All Tabbed Pages
      LoadContentPages (() =>
      {
        // Trigger custom event if browser is resized.
        // This is so custom SMAC Widgets can listen for it and resize themselves.
        window.addEventListener ('resize', (event) => { $(document.body).trigger ('browserResized'); });

        // Show nav buttons, diagnostics button and status bar
        $('#navButtons'   ).css ('display', 'inline-block');
        $('#diagAndCBox'  ).css ('display', 'inline-block');
        $('#smacStatusBar').css ('display', 'flex'        );

        UpdateConnectionBox ();

        // Update current date/time once per minute
        SetDateTime ();
        DTInterval = setInterval (SetDateTime, 60000);

        // Click on the first nav button
        $('.navButton').first().trigger ('pointerdown');

        // Wait for Relayer
        setTimeout (async () =>
        {
          // Request full system info from the Relayer
          await Send_UItoRelayer (0, '00|SYSI');  // System Info command

          // Run 'UserStartup()' (defined by User at bottom of index.html)
          UserStartup ();
        }, 1000);
      });
    }
  }
  catch (ex)
  {
    ShowException (ex);
  }
}

//--- ConnectToRelayer_Async ------------------------------

async function ConnectToRelayer_Async ()
{
  try
  {
    // Select serial port
    await SMACPort.ChoosePort ()
    .then (async () =>
    {
      // Open serial port
      await SMACPort.Open (SerialPortSettings)
      .then (async portOpened =>
      {
        // Check if port is opened
        if (portOpened)
        {
          console.info ('Port is open. Starting Read Loop ...');

          // Start read loop
          // We disconnected if this read loop finishes
          SMACPort.ReadLoop ();
        }
      })
      .catch (ex =>
      {
        // Open() failed
        PopupMessage ('SMAC Interface', 'Unable to connect to Relayer.');
      });
    })
    .catch (ex =>
    {
      // Ignore "NotFoundError" (No port choosen)
    });
  }
  catch (ex)
  {
    ShowException (ex);
  }
}

//--- RelayerDisconnected ---------------------------------

function RelayerDisconnected ()
{
  try
  {
    UpdateConnectionBox ();
    $('#smacPageArea').html ('<div class="relayerDisconnected">Relayer Disconnected</div>');

    setTimeout (() => { location.reload (); }, 1000);
  }
  catch (ex)
  {
    ShowException (ex);
  }
}

//--- SetDateTime -----------------------------------------

function SetDateTime ()
{
  try
  {
    const now = new Date ();

    let value = now.toDateString();
    $("#statusBar_Date").html (value);

    value = now.toTimeString().substring(0, 5);
    $("#statusBar_Time").html (value);






    // Also check if all Nodes are still alive
    // ...





  }
  catch (ex)
  {
    ShowException (ex);
  }
}

//--- ProcessRelayerMessage -------------------------------

async function ProcessRelayerMessage (dataString)
{
  try
  {
    // Normal incoming Data Strings from the Relayer have the
    // following format: (fields are separated with the '|' char)
    //
    //   ┌─────────────────── 2-char nodeID (00-19)
    //   │  ┌──────────────── 2-char deviceID (00-99)
    //   │  │     ┌────────── Variable length timestamp - usually millis()
    //   │  │     │       ┌── Variable length value string
    //   │  │     │       │   (this can be a numerical value or a text/error message)
    //   │  │     │       │
    //   nn|dd|timestamp|value
    //
    // There are two special messages from the Relayer:
    //
    //   NODE|...     Indicates that a new Node has attached to the system
    //   ERROR:...    Indicates a Relayer error

    if (Debugging)
      console.info ('--> ' + dataString);

    // Check if new Node connected: NODE|nodeID
    if (dataString.startsWith ('NODE|'))
    {
      const nodeID    = dataString.substring (5, 7);
      const nodeIndex = parseInt (nodeID);

      if (nodeIndex < MaxNodes)
      {
        PopupBar ('New Node connected to Relayer', 2000);

        if (Debugging)
          console.info ('((( Node ' + nodeID + ' connected )))');

        // Request Node Info: nn|00|timestamp|NOINFO|name|version|macAddress|numDevices
        await Send_UItoRelayer (nodeIndex, '00|GNOI');

        // Request Device Info: nn|dd|timestamp|DEINFO|name0|ipEn0|ppEn0|rate0|name1|ipEn1|ppEn1|rate1|...
        await Send_UItoRelayer (nodeIndex, '00|GDEI');
      }

      return;
    }

    // Error messages from the Relayer have no Node or Device associated with them.
    // Their message starts with 'ERROR:' ...
    // Show any Relayer Errors
    if (dataString.startsWith ('ERROR:'))
    {
      PopupMessage ('SMAC Relayer Error', dataString.substring (6));
      return;
    }

    // All other messages will be a Node/Device Data String: nodeID|deviceID|timestamp|value
    //   nodeID    = 2-digits 00-19
    //   deviceID  = 2-digits 00-99
    //   timestamp = n digits
    //   value     = Device data or message

    const fields = dataString.split ('|');
    if (fields.length < 4)
    {
      Diagnostics.LogToMonitor (0, 'Invalid Data String: ' + dataString);
      return;
    }

    // Parse Data String
    const nodeIndex   = parseInt (fields[0]);
    const deviceIndex = parseInt (fields[1]);
    const timestamp   = BigInt   (fields[2]);
    const value       = dataString.substring (fields[2].length + 7);  // Because value may also have '|' characters and fields.
                                                                      // We need the whole value string including '|'s and fields.
    if (Diagnostics.DataLogging)
      Diagnostics.LogToMonitor (nodeIndex, '──▶ ' + dataString);







    // // Mark last time since we heard from Node
    // Node[nodeIndex].lastMessageTime = timestamp;







    // First, update Widgets with actual device data values
    // They start with a dash or a digit
    if (value[0] == '-' || (value[0] >= '0' && value[0] <= '9'))
    {
      // Update all UI Widgets with device data
      // These events are handled by SMAC Widgets
      $(document.body).trigger ('deviceData', [ nodeIndex, deviceIndex, timestamp, Number(value) ]);
      return;
    }

    //=====================================================================
    // Handle non-numeric (message) values:
    //   NOINFO=
    //   DEINFO=
    //   NONAME=
    //   DENAME=
    //   RATE=
    //   IP Enabled
    //   IP Disabled
    //   IP Performed
    //   PP Enabled
    //   PP Disabled
    //   FILES=
    //   FILE=
    //   ERROR=
    //   PONG
    //=====================================================================

    if (value.startsWith ('NOINFO='))
    {
      // Add new Node if it does NOT exist already
      if (Nodes[nodeIndex] == undefined)
      {
        const newNode = {
                          name       : 'not set',
                          version    : 'not set',
                          macAddress : 'not set',
                          numDevices : 0,
                          devices    : [],          // Device objects { name, rate }
                          monitor    : undefined
                        };

        Nodes[nodeIndex] = newNode;
      }

      // Update Node info fields: name, version, macAddress, numDevices
      Nodes[nodeIndex].name       = fields[3].substring(7);
      Nodes[nodeIndex].version    = fields[4];
      Nodes[nodeIndex].macAddress = fields[5];
      Nodes[nodeIndex].numDevices = parseInt (fields[6]);

      // Update the Diagnostics UI
      Diagnostics.BuildSystem ();
      Diagnostics.LogToMonitor (nodeIndex, 'Node ' + nodeIndex.toString() + ' connected.');



      // // Trigger an event to inform anyone that a new Node was added
      // $(document.body).trigger ('newNode', [ nodeIndex ]);



    }

    else if (value.startsWith ('DEINFO='))
    {
      // Make sure Node exists in Diagnostics
      if (Nodes[nodeIndex] != undefined)
      {
        // One Device per 'DEINFO=...' message
        // Fill four Device fields: name, ipEnabled, ppEnabled, rate

        const deviceArray  = Nodes[nodeIndex].devices;
        const deviceFields = value.substring(7).split ('|');
        deviceArray[deviceIndex] = { name:deviceFields[0], ipEnabled:deviceFields[1], ppEnabled:deviceFields[2], rate:deviceFields[3] };

        Diagnostics.UpdateDevices (nodeIndex);  // Update the UI Device fields of the Node Block in Diagnostics

        // Update status bar Device count
        let totalDevices = 0;
        Nodes.forEach ((node) =>
        {
          if (node != undefined)
            totalDevices += node.numDevices;
        });

        $("#statusBar_Devices").html (totalDevices.toString());
      }
    }

    else if (value.startsWith ('NONAME='))
    {
      Nodes[nodeIndex].name = value.substring(7);
      Diagnostics.UpdateNodeBlock (nodeIndex);
    }

    else if (value.startsWith ('DENAME='))
    {
      Nodes[nodeIndex].devices[deviceIndex].name = value.substring(7);
      Diagnostics.UpdateDevices (nodeIndex);
    }

    else if (value.startsWith ('RATE='))
    {
      Nodes[nodeIndex].devices[deviceIndex].rate = value.substring(5);
      Diagnostics.UpdateDevices (nodeIndex);
    }

    else if (value == 'IP Enabled')
    {
      Nodes[nodeIndex].devices[deviceIndex].ipEnabled = 'Y';
      Diagnostics.UpdateDevices (nodeIndex);
    }

    else if (value == 'IP Disabled')
    {
      Nodes[nodeIndex].devices[deviceIndex].ipEnabled = 'N';
      Diagnostics.UpdateDevices (nodeIndex);
    }

    else if (value == 'PP Enabled')
    {
      Nodes[nodeIndex].devices[deviceIndex].ppEnabled = 'Y';
      Diagnostics.UpdateDevices (nodeIndex);
    }

    else if (value == 'PP Disabled')
    {
      Nodes[nodeIndex].devices[deviceIndex].ppEnabled = 'N';
      Diagnostics.UpdateDevices (nodeIndex);
    }

    else if (value.startsWith ('FILES='))
    {
      // TODO: List of files
    }

    else if (value.startsWith ('FILE='))
    {
      // TODO: File contents
    }

    else if (value.startsWith ('ERROR='))
    {
      // Error messages from a Node or Device post their error in the value field.
      // The message has the format:
      //   nodeID|deviceID|timestamp|ERROR=...

      // Log Node/Device Error message
      Diagnostics.LogToMonitor (nodeIndex, value.substring(6));
    }

    else if (value.startsWith ('PONG'))
    {
      // Log PONG received
      Diagnostics.LogToMonitor (nodeIndex, 'PONG Received');
    }



//  else
//  {
//    // Return device info
//    $(document.body).trigger ('deviceInfo', [ nodeIndex, deviceIndex, timestamp, value ]);
//  }



  }
  catch (ex)
  {
    ShowException (ex);
  }
}

//--- Send_UItoRelayer ------------------------------------

async function Send_UItoRelayer (nodeIndex, deviceCommand)
{
  try
  {
    // Device Command Format: deviceID|command|params
    // where:
    //   deviceID = 2-digits 00-99
    //   command  = 4-chars (caps)
    //   params   = optional parameters (null terminated)

    const nodeID = nodeIndex.toString().padLeft ('0', 2);
    const fullUIMessage = nodeID + '|' + deviceCommand;

    try
    {
      await SMACPort.Send (fullUIMessage + '\n');
    }
    catch (ex)
    {
      console.error (ex);
      return;
    }

    if (Debugging)
      console.info ('<-- ' + fullUIMessage);

    // Log message to the currently viewed monitor
    if (Diagnostics.DataLogging)
      Diagnostics.LogToMonitor (nodeIndex, '◀── ' + fullUIMessage);
  }
  catch (ex)
  {
    ShowException (ex);
  }
}



// //--- BroadcastUIMessage ----------------------------------
//
// async function BroadcastUIMessage (command)
// {
//   try
//   {
//     await Send_UItoRelayer (0, 'Broadcast|' + command);
//   }
//   catch (ex)
//   {
//     ShowException (ex);
//   }
// }
//
// //--- GetFileList -----------------------------------------
//
// async function GetFileList (path, extension)
// {
//   try
//   {
//     await Send_UItoRelayer (0, 'GetFileList|' + path + '|' + extension);
//   }
//   catch (ex)
//   {
//     ShowException (ex);
//   }
// }
//
// //--- GetFileContents -------------------------------------
//
// async function GetFileContents (path)
// {
//   try
//   {
//     await Send_UItoRelayer (0, 'GetFileContents|' + path);
//   }
//   catch (ex)
//   {
//     ShowException (ex);
//   }
// }
//
// //--- PutFileContents -------------------------------------
//
// async function PutFileContents (path, contents)
// {
//   try
//   {
//     await Send_UItoRelayer (0, 'PutFileContents|' + path + '|' + contents);
//   }
//   catch (ex)
//   {
//     ShowException (ex);
//   }
// }



//--- CloseDataLog-----------------------------------------

async function CloseDataLog ()
{
  try
  {
    // Close any Data Log files that are recording




  }
  catch (ex)
  {
    ShowException (ex);
  }
}
