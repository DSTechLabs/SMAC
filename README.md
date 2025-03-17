# SMAC Framework (System Monitor and Control)

**SMAC** is a fast, wireless, bidirectional system similar to a
<a href="https://en.wikipedia.org/wiki/SCADA" target="_blank">SCADA System!</a><br>
It allows you to monitor and control your devices from a browser.

<a href="https://dstechlabs.com/GizmoLab/SMAC" target="_blank">Click here to learn all about the **SMAC Framework**</a>, then return here to try your first example.

# Your first example
The three main abilites of **The SMAC Framework** are:
- Telemetry (aquiring data from remote sensors and equipment)
- Control (sending commands to remote devices and equipment)
- Scalable (up to 20 remote modules with up to 100 devices each)

Our first example will demo simple telemetry by displaying a reading from a light sensor (photoresistor).

You will need at least two ESP32 Dev Boards for a SMAC System.
One for a Relayer and at least one for a Remote Module (Node).<br>

Let's start by creating a project folder on your local drive.<br>
Then download a ZIP file of the code in this repo.<br>
(Under the Code Tab look for the green [ Code v ] pulldown button)<br>
Extract it into your project folder.<br>
The Firmware folder holds the ESP32 code for both Nodes and the Relayer.<br>
The Interface folder holds the **SMAC User Interface** web app.

<pre>
MyProject
  │
  ├── Firmware
  │     │
  │     ├── Node_Example1
  │     ├── Node_Example2
  │     ├── Node_Example3
  │     ├── Node_Example4
  │     └── Relayer
  │
  └── Interface (web app)
        │
        ├── Fonts
        ├── Images
        ├── Scripts
        ├── diagnostics.html
        ├── dsStyles.css
        ├── example.html ─── edit for this example
        ├── favIcon.ico
        ├── index.html ───── edit for your project
        ├── setMAC.html
        └── smacStyles.css
</pre><br>

Starting with the Firmware let's turn one of your ESP32's into a **SMAC Relayer**.

I use the Arduino framework in VS Code, but your welcome to use
PlatformIO or the Arduino IDE.  You will notice that the Arduino
support by Microsoft is deprecated, but there is support from
the Arduino Community.  Install the Arduino Community Edition
extension.

Open the Relayer folder and compile/flash to one of your ESP32's.
You should now have a working Relayer and can be reused for all
**SMAC** projects.







## If you like this framework, please <a href="https://buymeacoffee.com/dstechlabs" target="_blank">Buy Me A Coffee ☕</a>
