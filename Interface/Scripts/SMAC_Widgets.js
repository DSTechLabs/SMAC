//=============================================================================
//
//     FILE  : SMAC_Widgets.js
//
//   PROJECT : System Monitor and Control (SMAC)
//
//  FUNCTION : Custom HTML elements (Web Components) for SMAC Interface:
//
//               Control Widgets:
//               ────────────────
//               √ smac-panelbutton
//               √ smac-switch (including E-Stop)
//               ∙ smac-dial
//               ∙ smac-spinner
//               ∙ smac-slider     https://www.smashingmagazine.com/2021/12/create-custom-range-input-consistent-browsers/
//               ∙ smac-dualslider
//               ∙ smac-joystick
//
//               Data Widgets:
//               ────────────────
//               √ smac-led
//               √ smac-panellight
//               ∙ smac-flasher
//               √ smac-digital
//               √ smac-odometer
//               √ smac-display
//               √ smac-growbar
//               ∙ smac-gauge
//               √ smac-bargraph
//               √ smac-timegraph
//               ∙ smac-devicegraph
//               √ smac-polargraph
//               ∙ smac-piechart
//               ∙ smac-donutchart
//               ∙ smac-chartgraph
//               √ smac-compass
//               ∙ smac-bubblefield
//               ∙ smac-venndiagram
//               ∙ smac-bloomfield
//               ∙ smac-camera
//
//     NOTES : To cast shadows correctly, some of these Widgets require two images - usually a base image and a knob image.
//
//             Some of these Widgets use the dvCanvas2D class. Load the <dvCanvas2D.js> file before this one.
//
//             To be responsive, Widget width/height/diameter attributes are percentages of the browser's client width.
//
//             All Data Widgets should have the following attributes for showing warnings and alarms:
//               warningLow  = "..."
//               warningHigh = "..."
//               alarmLow    = "..."
//               alarmHigh   = "..."
//
//             Handling right-click context menu popups:
//               this.addEventListener ('contextmenu', (event) => { StopEvent(event); doYourStuff(); });
//
//             After going back and forth, I have decided not to bother with the Shadow DOM (shadowRoot, etc.):
//               ∙ https://www.matuzo.at/blog/2023/pros-and-cons-of-shadow-dom/
//               ∙ https://front-end.social/@chriscoyier/111383513656341191
//
//   AUTHOR  : Bill Daniels
//             Copyright 2020-2025, D+S Tech Labs, Inc.
//             All Rights Reserved
//
//=============================================================================


//--- Globals ---------------------------------------------

const shadowColor = 'rgba(0, 0, 0, 0.6)';


//--- SetAsInlineBlock ------------------------------------

function SetAsInlineBlock (element)
{
  try
  {
    // Set element's style as display:inline-block and vertical-align:top
    let style = element.getAttribute ('style');
    if (style == undefined)
      style = 'display:inline-block; vertical-align:top';
    else
      style = 'display:inline-block; vertical-align:top; ' + style;
    element.setAttribute ('style', style);
  }
  catch (ex)
  {
    ShowException (ex);
  }
}


//=============================================================================
//
//                   C O N T R O L    W I D G E T S
//
//=============================================================================

//=============================================================================
//  <smac-panelbutton> element
//=============================================================================

class SMAC_PanelButton extends HTMLElement
{
  //--- Constructor ---------------------------------------

  constructor ()
  {
    super ();
  }

  //--- Attributes ----------------------------------------

  get onImage   (     ) { return this.OnImage;    }
  set onImage   (value) { this.OnImage = value;   }

  get offImage  (     ) { return this.OffImage;   }
  set offImage  (value) { this.OffImage = value;  }

  get size      (     ) { return this.Size;       }
  set size      (value) { this.Size = value;      }

  get onAction  (     ) { return this.OnAction;   }
  set onAction  (value) { this.OnAction = value;  }

  get offAction (     ) { return this.OffAction;  }
  set offAction (value) { this.OffAction = value; }

  get pressed   (     ) { return this.Pressed;    }

  //--- connectedCallback ---------------------------------

  connectedCallback ()
  {
    try
    {
      // // Create shadow root
      // this.shadow = this.attachShadow ({mode: 'open'});

      // Check for required attributes
      if (!this.hasAttribute ('offImage')) throw '(smac-panelbutton): Missing offImage attribute';
      this.OffImage = this.getAttribute ('offImage');

      if (!this.hasAttribute ('onImage' )) throw '(smac-panelbutton): Missing onImage attribute';
      this.OnImage = this.getAttribute ('onImage');

      SetAsInlineBlock (this);

      // Set optional attributes
      this.Size      = this.hasAttribute ('size'     ) ? this.getAttribute ('size')      : '1em';
      this.OnAction  = this.hasAttribute ('onAction' ) ? this.getAttribute ('onAction' ) : undefined;
      this.OffAction = this.hasAttribute ('offAction') ? this.getAttribute ('offAction') : undefined;

      // Internal flag
      this.Pressed = false;

      // Create image element
      this.img = document.createElement ('img');
      this.img.setAttribute ('draggable', 'false');
      this.img.style = 'user-drag:none; width:' + this.Size + 'vw; filter:drop-shadow(' + this.Size/30 + 'vw ' + this.Size/20 + 'vw 0.4vw var(--dark)); cursor:pointer; vertical-align:top';
      this.img.src = this.OffImage;
      this.img.addEventListener ('pointerdown', (event) => { StopEvent(event); this.buttonDown(this); });
      this.img.addEventListener ('pointerup'  , (event) => { StopEvent(event); this.buttonUp  (this); });
      this.img.addEventListener ('mouseout'   , (event) => { StopEvent(event); this.buttonUp  (this); });

      // this.shadow.append (this.img);
      this.append (this.img);
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- buttonDown ----------------------------------------

  buttonDown = function (thisPanelButton)
  {
    try
    {
      if (!thisPanelButton.Pressed)
      {
        thisPanelButton.Pressed = true;
        thisPanelButton.img.src = thisPanelButton.OnImage;

        if (thisPanelButton.OnAction != undefined)
          eval (thisPanelButton.OnAction);
      }
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- buttonUp ------------------------------------------

  buttonUp = function (thisPanelButton)
  {
    try
    {
      if (thisPanelButton.Pressed)
      {
        thisPanelButton.Pressed = false;
        thisPanelButton.img.src = thisPanelButton.OffImage;

        if (thisPanelButton.OffAction != undefined)
          eval (thisPanelButton.OffAction);
      }
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }
};

customElements.define ('smac-panelbutton', SMAC_PanelButton);


//=============================================================================
//  <smac-switch> element
//=============================================================================

class SMAC_Switch extends HTMLElement
{
  //--- Constructor ---------------------------------------

  constructor ()
  {
    super ();
  }

  //--- Attributes ----------------------------------------

  get size      (     ) { return this.Size;                 }
  set size      (value) { this.Size = value;                }

  get onImage   (     ) { return this.OnImage;              }
  set onImage   (value) { this.OnImage = value;             }

  get offImage  (     ) { return this.OffImage;             }
  set offImage  (value) { this.OffImage = value;            }

  get onAction  (     ) { return this.OnAction;             }
  set onAction  (value) { this.OnAction = value;            }

  get offAction (     ) { return this.OffAction;            }
  set offAction (value) { this.OffAction = value;           }

  get on        (     ) { return this.On;                   }
  set on        (value) { this.On = (value ? true : false); }

  //--- connectedCallback ---------------------------------

  connectedCallback ()
  {
    try
    {
      // // Create shadow root
      // this.shadow = this.attachShadow ({mode: 'open'});

      // Check for required attributes
      if (!this.hasAttribute ('onImage' )) throw '(smac-switch): Missing onImage attribute';
      this.OnImage = this.getAttribute ('onImage');

      if (!this.hasAttribute ('offImage')) throw '(smac-switch): Missing offImage attribute';
      this.OffImage = this.getAttribute ('offImage');

      // Set optional attributes
      this.Size      = this.hasAttribute ('size'     ) ?  this.getAttribute ('size'     )                 : '1em';
      this.OnAction  = this.hasAttribute ('onAction' ) ?  this.getAttribute ('onAction' )                 : undefined;
      this.OffAction = this.hasAttribute ('offAction') ?  this.getAttribute ('offAction')                 : undefined;
      this.On        = this.hasAttribute ('on'       ) ? (this.getAttribute ('on'       ) ? true : false) : false;

      SetAsInlineBlock (this);

      // Create image element
      this.img = document.createElement ('img');
      this.img.setAttribute ('draggable', 'false');
      this.img.style = 'user-drag:none; width:' + this.Size + 'vw; filter:drop-shadow(' + this.Size/30 + 'vw ' + this.Size/20 + 'vw 0.4vw var(--dark)); cursor:pointer; vertical-align:top';
      this.img.src = (this.On ? this.OnImage : this.OffImage);

      this.img.addEventListener ('pointerdown', (event) => { StopEvent(event); this.toggle(this); });

      // Flasher (if this is an E-Stop button)
      this.flasher = undefined;

      // this.shadow.append (this.img);
      this.append (this.img);
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- toggle --------------------------------------------

  toggle = function (thisSwitch)
  {
    try
    {
      if (thisSwitch.On)
      {
        thisSwitch.On = false;
        thisSwitch.img.src = thisSwitch.OffImage;

        // Stop flashing if this is an E-Stop button
        if (thisSwitch.flasher != undefined)
        {
          clearInterval (thisSwitch.flasher);
          thisSwitch.flasher = undefined;

          thisSwitch.classList.remove ('eStopFlasher');
        }

        if (thisSwitch.OffAction != undefined)
          eval (thisSwitch.OffAction);
      }
      else
      {
        thisSwitch.On = true;
        thisSwitch.img.src = thisSwitch.OnImage;

        // Flash Red if this is an E-Stop button
        if (thisSwitch.OnImage.contains ('EStop'))
        {
          thisSwitch.flashEStop (thisSwitch);

          const self = thisSwitch;
          thisSwitch.flasher = setInterval (() => { self.flashEStop(self) }, 600);
        }

        if (thisSwitch.OnAction != undefined)
          eval (thisSwitch.OnAction);
      }
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- flashEStop ----------------------------------------

  flashEStop = function (thisSwitch)
  {
    try
    {
      thisSwitch.classList.add ('eStopFlasher');
      setTimeout (() => { thisSwitch.classList.remove ('eStopFlasher'); }, 300);
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }
};

customElements.define ('smac-switch', SMAC_Switch);


//=============================================================================
//  <smac-dial> element
//=============================================================================

class SMAC_Dial extends HTMLElement
{
  //--- Constructor ---------------------------------------

  constructor ()
  {
    super ();

    // Rebuild this widget on resize
    $(document.body).on ('browserResized', () => { this.build (); });
  }

  //--- Attributes ----------------------------------------

  get size        (     ) { return this.Size;         }
  set size        (value) { this.Size = value;        }

  get baseImage   (     ) { return this.BaseImage;    }
  set baseImage   (value) { this.BaseImage = value;   }

  get handleImage (     ) { return this.HandleImage;  }
  set handleImage (value) { this.HandleImage = value; }

  get angles      (     ) { return this.Angles;       }  // 0|30|60|90|120|150|180|210|240|270|300|330
  set angles      (value) { this.Angles = value;      }

  get labels      (     ) { return this.Labels;       }  // text label for each angle
  set labels      (value) { this.Labels = value;      }

  get labelClass  (     ) { return this.LabelClass;   }
  set labelClass  (value) { this.LabelClass = value;  }

  get position    (     ) { return this.Position;     }
  set position    (value) { this.Position = value;    }

  get action      (     ) { return this.Action;       }
  set action      (value) { this.Action = value;      }

  //--- connectedCallback ---------------------------------

  connectedCallback ()
  {
    try
    {
      // // Create shadow root
      // this.shadow = this.attachShadow ({mode: 'open'});

      // Check for required attributes
      if (!this.hasAttribute ('baseImage')) throw '(smac-dial): Missing baseImage attribute';
      this.BaseImage = this.getAttribute ('baseImage');

      if (!this.hasAttribute ('handleImage')) throw '(smac-dial): Missing handleImage attribute';
      this.HandleImage = this.getAttribute ('handleImage');

      // Set optional attributes
      this.Size       = this.hasAttribute ('size'      ) ?           this.getAttribute ('size'      )  : '5'  ;
      this.Angles     = this.hasAttribute ('angles'    ) ?           this.getAttribute ('angles'    )  : '330,30';
      this.Labels     = this.hasAttribute ('labels'    ) ?           this.getAttribute ('labels'    )  : 'Off,On';
      this.LabelClass = this.hasAttribute ('labelClass') ?           this.getAttribute ('labelClass')  : undefined;
      this.Position   = this.hasAttribute ('position'  ) ? parseInt (this.getAttribute ('position'  )) : 0;
      this.Action     = this.hasAttribute ('action'    ) ?           this.getAttribute ('action'    )  : undefined;

      // Set number of stops from number of angles
      this.angleArray   = this.Angles.split (',');
      this.labelArray   = this.Labels.split (',');
      this.numPositions = this.angleArray.length;

      // Check settings
      if (this.labelArray.length != this.numPositions)
        throw '(smac-dial): Number of labels does not match number of angles';

      SetAsInlineBlock (this);
      this.style.position = 'relative';

      // Create dial images
      this.imgBase = document.createElement ('img');
      this.imgBase.setAttribute ('draggable', 'false');
      this.imgBase.style = 'user-drag:none; width:' + this.Size + 'vw; filter:drop-shadow(' + this.Size/30 + 'vw ' + this.Size/20 + 'vw 0.4vw var(--dark)); cursor:pointer; vertical-align:top';
      this.imgBase.src = this.BaseImage;
      this.append (this.imgBase);

      this.imgHandle = document.createElement ('img');
      this.imgHandle.setAttribute ('draggable', 'false');
      this.imgHandle.style = 'user-drag:none; width:' + this.Size + 'vw; filter:drop-shadow(' + this.Size/30 + 'vw ' + this.Size/20 + 'vw 0.4vw var(--dark)); cursor:pointer; vertical-align:top';
      this.imgHandle.src = this.BaseImage;
      this.append (this.imgHandle);







      // Position Handle centered on Base

      this.dialImage.style.position = 'relative';
      this.dialImage.style.left     = '0';
      this.dialImage.style.top      = '0';
      this.dialImage.style.cursor   = 'pointer';
      this.append (this.dialImage);








      // Create labels
      this.labelElements = [];
      for (let i=0; i<this.numPositions; i++)
      {
        this.labelElements[i] = document.createElement ('div');
        this.labelElements[i].innerHTML = this.labelArray[i];
        this.labelElements[i].classList.add ('smac-dialLabel');
        if (this.LabelClass != undefined) this.labelElements[i].classList.add (this.LabelClass);
        this.labelElements[i].style.fontSize = (Number(this.Size) / 5.0).toString() + 'vw';
        this.labelElements[i].addEventListener ('pointerdown', (event) => { StopEvent(event); this.setDial(event); });
        this.append (this.labelElements[i]);
      }

      // Draw dial
      this.build ();





      // this.addEventListener ('pointerdown', (event) => { StopEvent(event); this.setDial(event); });





    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- build ---------------------------------------------

  build = function ()
  {
    try
    {
      // Set dial size and center
      const dialWidth = Math.round (this.Size * GetBrowserWidth() / 100);
      this.dialImage.style.width = dialWidth.toString() + 'px';



      // const dialHeight = parseInt (window.getComputedStyle (this.dialImage).height);





        // console.info (dialWidth.toString() + ' x ' + dialHeight.toString());





      // const offsetLeft = dialWidth  / 2;
      // const offsetTop  = dialHeight / 2;


      const radius     = dialWidth / 2;  // Math.max (offsetLeft, offsetTop);

      // Position labels
      for (let p=0; p<this.numPositions; p++)
      {
        const radAngle = Number (this.angleArray[p]) * deg2rad - piOver2;

        const labelStyle  = window.getComputedStyle (this.labelElements[p]);
        const labelWidth  = parseInt (labelStyle.width );
        const labelHeight = parseInt (labelStyle.height);

        const left = Math.round (radius + (radius * Math.cos (radAngle)) - labelWidth /2);
        const top  = Math.round (radius + (radius * Math.sin (radAngle)) - labelHeight/2);

        this.labelElements[p].style.left = left.toString () + 'px';
        this.labelElements[p].style.top  = top .toString () + 'px';
      }






    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- SetDial ---------------------------------------------

  setDial = function (event)
  {
    try
    {
//       // Get relative mouse click position
//       const clickSpecs = GetClickSpecs (event);
//
//       // Reduce position (counter-clockwise) if left side was clicked
//       // Increase position (clockwise) if right side was clicked
//       if ((clickSpecs.x > clickSpecs.boundingRect.width / 2) || (clickSpecs.y > clickSpecs.boundingRect.height / 2))
//       {
//         if (this.Position < this.numPositions - 1)
//           ++this.Position;
//       }
//       else
//       {
//         if (this.Position > 0)
//           --this.Position;
//       }
//
//       // Draw dial at new position
//       this.dialImage.src = 'Images/ui_Dial_' + this.angleArray[this.Position] + '.png';







      this.Position = 0;
      const dialInt = setInterval (() =>
      {
        this.dialImage.style.transform = 'rotate(' + this.Position.toString() + 'deg)';
        this.Position += 30;
      }, 100);








      // Execute action, if any
      if (this.Action != undefined)
        eval (this.Action);
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }
};

customElements.define ('smac-dial', SMAC_Dial);






















//=============================================================================
//  <smac-slider> element
//=============================================================================

class SMAC_Slider extends HTMLElement
{
  //--- Constructor ---------------------------------------

  constructor ()
  {
    super ();

    // Rebuild this widget on resize
    $(document.body).on ('browserResized', () => { this.build (); });
  }

  //--- Attributes ----------------------------------------

  get orientation    (     ) { return this.Orientation;     }
  set orientation    (value) { this.Orientation = value;    }

  get width          (     ) { return this.Width;           }
  set width          (value) { this.Width = value;          }

  get height         (     ) { return this.Height;          }
  set height         (value) { this.Height = value;         }

  get minValue       (     ) { return this.MinValue;        }
  set minValue       (value) { this.MinValue = value;       }

  get maxValue       (     ) { return this.MaxValue;        }
  set maxValue       (value) { this.MaxValue = value;       }

  get value          (     ) { return this.Value;           }
  set value          (value) { this.Value = value;          }

  get units          (     ) { return this.Units;           }
  set units          (value) { this.Units = value;          }

  get backColor      (     ) { return this.BackColor;       }
  set backColor      (value) { this.BackColor = value;      }

  get fillColor      (     ) { return this.FillColor;       }
  set fillColor      (value) { this.FillColor = value;      }

  get scaleColor     (     ) { return this.ScaleColor;      }
  set scaleColor     (value) { this.ScaleColor = value;     }

  get scalePlacement (     ) { return this.ScalePlacement;  }
  set scalePlacement (value) { this.ScalePlacement = value; }

  //--- connectedCallback ---------------------------------

  connectedCallback ()
  {
    try
    {
      // // Create shadow root
      // this.shadow = this.attachShadow ({mode: 'open'});

      SetAsInlineBlock (this);

      // Set optional attributes
      this.Orientation    = this.hasAttribute ('orientation'   ) ?         this.getAttribute ('orientation'   )  : 'vertical';
      this.Width          = this.hasAttribute ('width'         ) ? Number (this.getAttribute ('width'         )) : 3;
      this.Height         = this.hasAttribute ('height'        ) ? Number (this.getAttribute ('height'        )) : 20;
      this.MinValue       = this.hasAttribute ('minValue'      ) ? Number (this.getAttribute ('minValue'      )) : 0;
      this.MaxValue       = this.hasAttribute ('maxValue'      ) ? Number (this.getAttribute ('maxValue'      )) : 4095;
      this.Value          = this.hasAttribute ('value'         ) ? Number (this.getAttribute ('value'         )) : this.MinValue;
      this.Units          = this.hasAttribute ('units'         ) ?         this.getAttribute ('units'         )  : '';
      this.BackColor      = this.hasAttribute ('backColor'     ) ?         this.getAttribute ('backColor'     )  : '#303030';
      this.FillColor      = this.hasAttribute ('fillColor'     ) ?         this.getAttribute ('fillColor'     )  : '#A0A0A0';
      this.ScaleColor     = this.hasAttribute ('scaleColor'    ) ?         this.getAttribute ('scaleColor'    )  : undefined;
      this.ScalePlacement = this.hasAttribute ('scalePlacement') ?         this.getAttribute ('scalePlacement')  : 'left';

      // Build this widget
      this.build ();

      //--- React to pointer dragging ---






    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- build ---------------------------------------------

  build = function ()
  {
    try
    {
      // If this widget is horizontal, then its bar height is fixed.
      // If this widget is vertical  , then its bar width  is fixed.
      // This is to show the pointer nicely with no squeezing or stretching.
      let cWidth  = 52;
      let cHeight = 36;

      // Canvas dimensions are percentages of browser's size
      if (this.Orientation == 'horizontal')
        cWidth  = Math.max (Math.round (this.Width  * GetBrowserWidth () / 100), 52);
      else
        cHeight = Math.max (Math.round (this.Height * GetBrowserHeight() / 100), 36);

      // Set initial bar size
      this.BarWidth  = cWidth;
      this.BarHeight = cHeight;
      this.OffsetX   = 0;
      this.OffsetY   = 0;

      // In area and adjust offsets for scale, if specified
      if (this.ScaleColor != undefined)
      {
        if (this.Orientation == 'horizontal')
        {
          this.BarWidth  -= 50;
          this.BarHeight -= 28;

          this.OffsetX = 25;
          if (this.ScalePlacement == 'top')
            this.OffsetY = 28;
        }
        else  // vertical
        {
          this.BarWidth  -= 60;
          this.BarHeight -= 10;

          this.OffsetY = 5;
          if (this.ScalePlacement == 'left')
            this.OffsetX = 60;
        }
      }

      // Create a 2D data visualization canvas (dvCanvas2D)
      this.smacCanvas = new dvCanvas2D (cWidth, cHeight, 'transparent');

      // Check if a canvas already exists from previous build
      if (this.firstChild == undefined)
        this.appendChild  (this.smacCanvas.canvas);
      else
        this.replaceChild (this.smacCanvas.canvas, this.firstChild);  // Replace existing canvas

      // Back and Fill Gradients
      if (this.Orientation == 'horizontal')
      {
        this.BackGrad = this.smacCanvas.cc.createLinearGradient (this.OffsetX, this.OffsetY, this.OffsetX+this.BarWidth, this.OffsetY);
        this.BackGrad.addColorStop (0.0, this.smacCanvas.adjustBrightness (this.BackColor, -30));  // darker
        this.BackGrad.addColorStop (1.0, this.smacCanvas.adjustBrightness (this.BackColor,  30));  // lighter

        this.FillGrad = this.smacCanvas.cc.createLinearGradient (this.OffsetX, this.OffsetY, this.OffsetX+this.BarWidth, this.OffsetY);
        this.FillGrad.addColorStop (0.0, this.smacCanvas.adjustBrightness (this.FillColor, -30));  // darker
        this.FillGrad.addColorStop (1.0, this.smacCanvas.adjustBrightness (this.FillColor,  30));  // lighter
      }
      else
      {
        this.BackGrad = this.smacCanvas.cc.createLinearGradient (this.OffsetX, this.OffsetY, this.OffsetX, this.OffsetY+this.BarHeight);
        this.BackGrad.addColorStop (0.0, this.smacCanvas.adjustBrightness (this.BackColor,  30));  // lighter
        this.BackGrad.addColorStop (1.0, this.smacCanvas.adjustBrightness (this.BackColor, -30));  // darker

        this.FillGrad = this.smacCanvas.cc.createLinearGradient (this.OffsetX, this.OffsetY, this.OffsetX, this.OffsetY+this.BarHeight);
        this.FillGrad.addColorStop (0.0, this.smacCanvas.adjustBrightness (this.FillColor,  30));  // lighter
        this.FillGrad.addColorStop (1.0, this.smacCanvas.adjustBrightness (this.FillColor, -30));  // darker
      }

      this.PointerFont = '16px sans-serif';





      // // Clear bar area
      // this.smacCanvas.drawRectangle (this.OffsetX, this.OffsetY, this.BarWidth, this.BarHeight, this.BackGrad, fill);





      // Manually calculate scale factor if scaleColor not specified
      if (this.ScaleColor == undefined)
        this.ScaleFactor = (this.orientation == 'horizontal' ? this.BarWidth : this.BarHeight) / (this.MaxValue - this.MinValue);
      else
      {
        // drawLinearScale returns the scale factor for you
        if (this.Orientation == 'horizontal')
        {
          if (this.ScalePlacement == 'top')
            this.ScaleFactor = this.smacCanvas.drawLinearScale (this.OffsetX, this.OffsetY  , this.BarWidth, this.BarHeight, ScaleOrientation.HorizTop   , this.MinValue, this.MaxValue, this.Units, this.ScaleColor);
          else  // bottom
            this.ScaleFactor = this.smacCanvas.drawLinearScale (this.OffsetX, this.BarHeight, this.BarWidth, this.BarHeight, ScaleOrientation.HorizBottom, this.MinValue, this.MaxValue, this.Units, this.ScaleColor);
        }
        else  // vertical
        {
          if (this.ScalePlacement == 'right')
            this.ScaleFactor = this.smacCanvas.drawLinearScale (this.BarWidth, this.BarHeight+this.OffsetY, this.BarWidth, this.BarHeight, ScaleOrientation.VertRight, this.MinValue, this.MaxValue, this.Units, this.ScaleColor);
          else  // left
            this.ScaleFactor = this.smacCanvas.drawLinearScale (this.OffsetX , this.BarHeight+this.OffsetY, this.BarWidth, this.BarHeight, ScaleOrientation.VertLeft , this.MinValue, this.MaxValue, this.Units, this.ScaleColor);
        }
      }

      // Set clipping region
      this.smacCanvas.setClipRectangle (this.OffsetX, this.OffsetY, this.BarWidth, this.BarHeight);






      this.updateWidget (200);





    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- updateWidget --------------------------------------

  updateWidget = function (dragPosition)
  {
    try
    {
      // Convert drag position to real value
      this.Value = dragPosition / this.ScaleFactor + this.MinValue;
      const pointerText = this.Value.toPrecision (4);
      const pointerTextSize = this.smacCanvas.getTextSize (pointerText, this.PointerFont);

      this.smacCanvas.drawRectangle (this.OffsetX, this.OffsetY, this.BarWidth, this.BarHeight, this.BackGrad, fill);

      if (this.Orientation == 'horizontal')
      {
        const gx = this.OffsetX + dragPosition;
        const gy = this.OffsetY;

        this.smacCanvas.drawRectangle (this.OffsetX, this.OffsetY, dragPosition, this.BarHeight, this.FillGrad, fill);

        // Draw horizontal pointer
        const vertexArray =
        [
          { x: gx     , y: gy +                  1 },
          { x: gx + 23, y: gy +                  9 },
          { x: gx + 23, y: gy + this.BarHeight - 9 },
          { x: gx     , y: gy + this.BarHeight - 1 },
          { x: gx - 23, y: gy + this.BarHeight - 9 },
          { x: gx - 23, y: gy +                  9 }
        ];
        this.smacCanvas.drawPoly (vertexArray, '#F0F0F0', fill);
        this.smacCanvas.drawPoly (vertexArray, '#202020',    1);

        // Draw value inside pointer
        const pointerTextX = dragPosition - pointerTextSize.width/2;
        const pointerTextY = gy + this.BarHeight/2 - 7;
        this.smacCanvas.drawText (pointerTextX, pointerTextY, pointerText, '#202020', this.PointerFont);
      }
      else
      {
        const gx = this.OffsetX;
        const gy = this.OffsetY + this.BarHeight - dragPosition;

        this.smacCanvas.drawRectangle (this.OffsetX, gy, this.BarWidth, dragPosition, this.FillGrad, fill);

        // Draw vertical pointer
        const vertexArray =
        [
          { x: gx +                 1, y: gy      },
          { x: gx +                 9, y: gy - 10 },
          { x: gx + this.BarWidth - 9, y: gy - 10 },
          { x: gx + this.BarWidth - 1, y: gy      },
          { x: gx + this.BarWidth - 9, y: gy + 10 },
          { x: gx +                 9, y: gy + 10 }
        ];
        this.smacCanvas.drawPoly (vertexArray, '#F0F0F0', fill);
        this.smacCanvas.drawPoly (vertexArray, '#202020',    1);

        // Draw value inside pointer
        const pointerTextX = gx + this.BarWidth/2 - pointerTextSize.width/2;
        const pointerTextY = gy - 7;
        this.smacCanvas.drawText (pointerTextX, pointerTextY, pointerText, '#202020', this.PointerFont);
      }
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }
};

customElements.define ('smac-slider', SMAC_Slider);

























//=============================================================================
//
//                      D A T A    W I D G E T S
//
//=============================================================================

//=============================================================================
//  <smac-led> element
//=============================================================================

class SMAC_LED extends HTMLElement
{
  //--- Constructor ---------------------------------------

  constructor ()
  {
    super ();
  }

  //--- Attributes ----------------------------------------

  get device (     ) { return this.Device;               }
  set device (value) { this.Device = value;              }

  get color  (     ) { return this.Color;                }
  set color  (value) { this.Color = value;               }

  get on     (     ) { return this.On;                   }
  set on     (value) { this.On = (value ? true : false); }

  //--- connectedCallback ---------------------------------

  connectedCallback ()
  {
    try
    {
      // Check for required attribute
      if (!this.hasAttribute ('device'))
        throw '(smac-led): Missing device attribute';

      const deviceInfo = this.getAttribute ('device').replaceAll (' ', '').split (',');
      this.NodeID   = parseInt (deviceInfo[0]);
      this.DeviceID = parseInt (deviceInfo[1]);

      SetAsInlineBlock (this);

      // Set optional attributes
      this.Color = this.hasAttribute ('color') ? this.getAttribute ('color') : 'green';

      // Set class for when LED is on (default is green)
      this.onClass = 'smac-ledGreen';

           if (this.Color == 'red'   ) this.onClass = 'smac-ledRed'   ;
      else if (this.Color == 'blue'  ) this.onClass = 'smac-ledBlue'  ;
      else if (this.Color == 'yellow') this.onClass = 'smac-ledYellow';
      else if (this.Color == 'orange') this.onClass = 'smac-ledOrange';
      else if (this.Color == 'purple') this.onClass = 'smac-ledPurple';
      else if (this.Color == 'white' ) this.onClass = 'smac-ledWhite' ;

      // Initial display
      this.updateWidget (0, 0);

      //--- React to device data ---
      const self = this;
      $(document.body).on ('deviceData', function (event, nodeID, deviceID, timestamp, value)
      {
        // Match this UI widget to its Node and device
        if (nodeID == self.NodeID && deviceID == self.DeviceID)
        {
          // // Show current sampling rate on hover
          // if (!self.hasAttribute ('title'))
          //   if (Nodes[nodeID] != undefined)
          //     if (Nodes[nodeID].devices[deviceID] != undefined)
          //       self.setAttribute ('title', 'Current rate: ' + Nodes[nodeID].devices[deviceID].rate.toString() + ' s/hour');

          // Update this widget
          window.requestAnimationFrame.bind (self.updateWidget (timestamp, value));
        }
      });
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- updateWidget --------------------------------------

  updateWidget = function (timestamp, value)
  {
    try
    {
      // value = '0' : off
      // value = '1' : on

      // Set state and display color
      if (value == '1')
      {
        this.On = true;
        this.classList.remove ('smac-ledOff');
        this.classList.add (this.onClass);
      }
      else
      {
        this.On = false;
        this.classList.remove (this.onClass);
        this.classList.add ('smac-ledOff');
      }
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }
};

customElements.define ('smac-led', SMAC_LED);


//=============================================================================
//  <smac-panellight> element
//=============================================================================

class SMAC_PanelLight extends HTMLElement
{
  //--- Constructor ---------------------------------------

  constructor ()
  {
    super ();
  }

  //--- Attributes ----------------------------------------

  get device (     ) { return this.Device;               }
  set device (value) { this.Device = value;              }

  get color  (     ) { return this.Color;                }
  set color  (value) { this.Color = value;               }

  get on     (     ) { return this.On;                   }
  set on     (value) { this.On = (value ? true : false); }

  //--- connectedCallback ---------------------------------

  connectedCallback ()
  {
    try
    {
      // Check for required attribute
      if (!this.hasAttribute ('device'))
        throw '(smac-panellight): Missing device attribute';

      const deviceInfo = this.getAttribute ('device').replaceAll (' ', '').split (',');
      this.NodeID   = parseInt (deviceInfo[0]);
      this.DeviceID = parseInt (deviceInfo[1]);

      SetAsInlineBlock (this);

      // Set optional attributes
      this.Color = this.hasAttribute ('color') ? this.getAttribute ('color') : 'green';

      // Initially off
      this.On = false;
      this.classList.add ('smac-panellightOff');

      // Set style for when light is on (default is green)
      this.onClass = 'smac-panellightGreen';

           if (this.Color == 'red'   ) this.onClass = 'smac-panellightRed'   ;
      else if (this.Color == 'blue'  ) this.onClass = 'smac-panellightBlue'  ;
      else if (this.Color == 'yellow') this.onClass = 'smac-panellightYellow';
      else if (this.Color == 'orange') this.onClass = 'smac-panellightOrange';
      else if (this.Color == 'purple') this.onClass = 'smac-panellightPurple';
      else if (this.Color == 'white' ) this.onClass = 'smac-panellightWhite' ;

      // Initial display
      this.updateWidget (0, 0);

      //--- React to device data ---
      const self = this;
      $(document.body).on ('deviceData', function (event, nodeID, deviceID, timestamp, value)
      {
        // Match this UI widget to its Node and device
        if (nodeID == self.NodeID && deviceID == self.DeviceID)
        {
          // // Show current sampling rate on hover
          // if (!self.hasAttribute ('title'))
          //   if (Nodes[nodeID] != undefined)
          //     if (Nodes[nodeID].devices[deviceID] != undefined)
          //       self.setAttribute ('title', 'Current rate: ' + Nodes[nodeID].devices[deviceID].rate.toString() + ' s/hour');

          // Update this widget
          window.requestAnimationFrame.bind (self.updateWidget (timestamp, value));
        }
      });
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- updateWidget --------------------------------------

  updateWidget = function (timestamp, value)
  {
    try
    {
      // value = 0 : off
      // value = 1 : on

      // Set state and display color
      if (value == '1')
      {
        this.On = true;
        this.classList.remove ('smac-panellightOff');
        this.classList.add (this.onClass);
      }
      else
      {
        this.On = false;
        this.classList.remove (this.onClass);
        this.classList.add ('smac-panellightOff');
      }
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }
};

customElements.define ('smac-panellight', SMAC_PanelLight);


//=============================================================================
//  <smac-rawvalue> element
//=============================================================================

class SMAC_RawValue extends HTMLElement
{
  //--- Constructor ---------------------------------------

  constructor ()
  {
    super ();
  }

  //--- Attributes ----------------------------------------

  get device (     ) { return this.Device;  }
  set device (value) { this.Device = value; }

  //--- connectedCallback ---------------------------------

  connectedCallback ()
  {
    try
    {
      // Check for required attribute
      if (!this.hasAttribute ('device'))
        throw '(smac-rawvalue): Missing device attribute';

      const deviceInfo = this.getAttribute ('device').replaceAll (' ', '').split (',');
      this.NodeID   = parseInt (deviceInfo[0]);
      this.DeviceID = parseInt (deviceInfo[1]);

      SetAsInlineBlock (this);

      // Set alarm ranges if specified
      this.AlarmLow  = this.hasAttribute ('alarmLow' ) ? Number (this.getAttribute ('alarmLow' )) : undefined;
      this.AlarmHigh = this.hasAttribute ('alarmHigh') ? Number (this.getAttribute ('alarmHigh')) : undefined;
      this.OrgBorder = this.style.border;  // To restore after alarm conditions

      this.innerHTML = '-';  // initial value

      //--- React to device data ---
      const self = this;
      $(document.body).on ('deviceData', function (event, nodeID, deviceID, timestamp, value)
      {
        // Match this UI widget to its Node and device
        if (nodeID == self.NodeID && deviceID == self.DeviceID)
        {
          // // Show current sampling rate on hover
          // if (!self.hasAttribute ('title'))
          //   if (Nodes[nodeID] != undefined)
          //     if (Nodes[nodeID].devices[deviceID] != undefined)
          //       self.setAttribute ('title', 'Current rate: ' + Nodes[nodeID].devices[deviceID].rate.toString() + ' s/hour');

          // Update this widget
          window.requestAnimationFrame.bind (self.updateWidget (timestamp, value));
        }
      });
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- updateWidget --------------------------------------

  updateWidget = function (timestamp, value)
  {
    try
    {
      // Set value
      this.innerHTML = value;
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }
};

customElements.define ('smac-rawvalue', SMAC_RawValue);


//=============================================================================
//  <smac-digital> element
//=============================================================================

class SMAC_Digital extends HTMLElement
{
  //--- Constructor ---------------------------------------

  constructor ()
  {
    super ();

    // Right-click Context Menu
    this.addEventListener ('contextmenu', function (event)
    {
      StopEvent (event);

      // Popup options menu
      PopupBubble (event, 'This is a Popup (Context Menu) test.<br>This should be a new line?');
    });
  }

  //--- Attributes ----------------------------------------

  get device    (     ) { return this.Device;     }
  set device    (value) { this.Device = value;    }

  get alarmLow  (     ) { return this.AlarmLow;   }
  set alarmLow  (value) { this.AlarmLow = value;  }

  get alarmHigh (     ) { return this.AlarmHigh;  }
  set alarmHigh (value) { this.AlarmHigh = value; }

  //--- connectedCallback ---------------------------------

  connectedCallback ()
  {
    try
    {
      // Check for required attribute
      if (!this.hasAttribute ('device'))
        throw '(smac-digital): Missing device attribute';

      const deviceInfo = this.getAttribute ('device').replaceAll (' ', '').split (',');
      this.NodeID   = parseInt (deviceInfo[0]);
      this.DeviceID = parseInt (deviceInfo[1]);

      SetAsInlineBlock (this);

      // Set alarm ranges if specified
      this.AlarmLow  = this.hasAttribute ('alarmLow' ) ? Number (this.getAttribute ('alarmLow' )) : undefined;
      this.AlarmHigh = this.hasAttribute ('alarmHigh') ? Number (this.getAttribute ('alarmHigh')) : undefined;
      this.OrgBorder = this.style.border;  // To restore after alarm conditions

      this.classList.add ('smac-digital');
      this.innerHTML = '-';  // initial value

      //--- React to device data ---
      const self = this;
      $(document.body).on ('deviceData', function (event, nodeID, deviceID, timestamp, value)
      {
        // Match this UI widget to its Node and device
        if (nodeID == self.NodeID && deviceID == self.DeviceID)
        {
          // // Show current sampling rate on hover
          // if (!self.hasAttribute ('title'))
          //   if (Nodes[nodeID] != undefined)
          //     if (Nodes[nodeID].devices[deviceID] != undefined)
          //       self.setAttribute ('title', 'Current rate: ' + Nodes[nodeID].devices[deviceID].rate.toString() + ' s/hour');

          // Update this widget
          window.requestAnimationFrame.bind (self.updateWidget (timestamp, value));
        }
      });
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- updateWidget --------------------------------------

  updateWidget = function (timestamp, value)
  {
    try
    {
      // Set value
      this.innerHTML = value;

      // Check alarm ranges
      if ((this.AlarmLow  != undefined && value <= this.AlarmLow ) ||
          (this.AlarmHigh != undefined && value >= this.AlarmHigh))
        this.style.border = '0.5vw solid #FF0000';
      else
        this.style.border = this.OrgBorder;
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }
};

customElements.define ('smac-digital', SMAC_Digital);


//=============================================================================
//  <smac-display> element
//=============================================================================

class SMAC_Display extends HTMLElement
{
  //--- Constructor ---------------------------------------

  constructor ()
  {
    super ();

    // Right-click context menu
    this.addEventListener ('contextmenu', function (event)
    {
      StopEvent (event);

      // ...
    });
  }

  //--- Attributes ----------------------------------------

  get device    (     ) { return this.Device;    }
  set device    (value) { this.Device = value;   }

  get width     (     ) { return this.Width;     }
  set width     (value) { this.Width = value;    }

  get height    (     ) { return this.Height;    }
  set height    (value) { this.Height = value;   }

  get maxLines  (     ) { return this.MaxLines;  }
  set maxLines  (value) { this.MaxLines = value; }

  //--- connectedCallback ---------------------------------

  connectedCallback ()
  {
    try
    {
      // Check for required attribute
      if (!this.hasAttribute ('device'))
        throw '(smac-display): Missing device attribute';

      const deviceInfo = this.getAttribute ('device').replaceAll (' ', '').split (',');
      this.NodeID   = parseInt (deviceInfo[0]);
      this.DeviceID = parseInt (deviceInfo[1]);

      SetAsInlineBlock (this);

      // Set attributes if specified
      this.Width    = this.hasAttribute ('width'   ) ? Number   (this.getAttribute ('width'   )) : 3;
      this.Height   = this.hasAttribute ('height'  ) ? Number   (this.getAttribute ('height'  )) : 10;
      this.MaxLines = this.hasAttribute ('maxLines') ? parseInt (this.getAttribute ('maxLines')) : 100;
      this.OrgBorder = this.style.border;  // To restore after alarm conditions

      // Display width/height are percentages of browser's size
      // const w = this.Width  * GetBrowserWidth () / 100;
      // const h = this.Height * GetBrowserHeight() / 100;
      this.style.width  = this.Width .toString() + 'vw';
      this.style.height = this.Height.toString() + 'vh';

      this.classList.add ('smac-display');
      this.classList.add ('dsScrollable');
      this.innerHTML = '';  // initial value
      this.numLines  = 0;

      //--- React to device data ---
      const self = this;
      $(document.body).on ('deviceData', function (event, nodeID, deviceID, timestamp, value)
      {
        // Match this UI widget to its Node and device
        if (nodeID == self.NodeID && deviceID == self.DeviceID)
        {
          // // Show current sampling rate on hover
          // if (!self.hasAttribute ('title'))
          //   if (Nodes[nodeID] != undefined)
          //     if (Nodes[nodeID].devices[deviceID] != undefined)
          //       self.setAttribute ('title', 'Current rate: ' + Nodes[nodeID].devices[deviceID].rate.toString() + ' s/hour');

          // Update this widget
          window.requestAnimationFrame.bind (self.updateWidget (timestamp, value));
        }
      });
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- updateWidget --------------------------------------

  updateWidget = function (timestamp, value)
  {
    try
    {
      if (this.numLines < this.MaxLines)
      {
        this.innerHTML += value + '<br>';  // Add new value line
        this.scroll (0, 100000);           // Auto-scroll to bottom
        ++this.numLines;
      }
      else
      {
        this.innerHTML = value + '<br>';  // Reset
        this.numLines  = 1;
      }
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }
};

customElements.define ('smac-display', SMAC_Display);


//=============================================================================
//  <smac-growbar> element
//=============================================================================

class SMAC_Growbar extends HTMLElement
{
  //--- Constructor ---------------------------------------

  constructor ()
  {
    super ();

    // Rebuild this widget on resize
    $(document.body).on ('browserResized', () => { this.build (); });

    // Right-click context menu
    this.addEventListener ('contextmenu', function (event)
    {
      StopEvent (event);

      // ...
    });
  }

  //--- Attributes ----------------------------------------

  get device         (     ) { return this.Device;          }
  set device         (value) { this.Device = value;         }

  get orientation    (     ) { return this.Orientation;     }
  set orientation    (value) { this.Orientation = value;    }

  get width          (     ) { return this.Width;           }
  set width          (value) { this.Width = value;          }

  get height         (     ) { return this.Height;          }
  set height         (value) { this.Height = value;         }

  get minValue       (     ) { return this.MinValue;        }
  set minValue       (value) { this.MinValue = value;       }

  get maxValue       (     ) { return this.MaxValue;        }
  set maxValue       (value) { this.MaxValue = value;       }

  get units          (     ) { return this.Units;           }
  set units          (value) { this.Units = value;          }

  get alarmLow       (     ) { return this.AlarmLow;        }
  set alarmLow       (value) { this.AlarmLow = value;       }

  get alarmHigh      (     ) { return this.AlarmHigh;       }
  set alarmHigh      (value) { this.AlarmHigh = value;      }

  get backColor      (     ) { return this.BackColor;       }
  set backColor      (value) { this.BackColor = value;      }

  get fillColor      (     ) { return this.FillColor;       }
  set fillColor      (value) { this.FillColor = value;      }

  get scaleColor     (     ) { return this.ScaleColor;      }
  set scaleColor     (value) { this.ScaleColor = value;     }

  get scalePlacement (     ) { return this.ScalePlacement;  }
  set scalePlacement (value) { this.ScalePlacement = value; }

  //--- connectedCallback ---------------------------------

  connectedCallback ()
  {
    try
    {
      // // Create shadow root
      // this.shadow = this.attachShadow ({mode: 'open'});

      // Check for required attributes
      if (!this.hasAttribute ('device'))
        throw '(smac-growbar): Missing device attribute';

      const deviceInfo = this.getAttribute ('device').replaceAll (' ', '').split (',');
      this.NodeID   = parseInt (deviceInfo[0]);
      this.DeviceID = parseInt (deviceInfo[1]);

      SetAsInlineBlock (this);

      // Set optional attributes
      this.Orientation    = this.hasAttribute ('orientation'   ) ?         this.getAttribute ('orientation'   )  : 'vertical';
      this.Width          = this.hasAttribute ('width'         ) ? Number (this.getAttribute ('width'         )) : 3;
      this.Height         = this.hasAttribute ('height'        ) ? Number (this.getAttribute ('height'        )) : 20;
      this.MinValue       = this.hasAttribute ('minValue'      ) ? Number (this.getAttribute ('minValue'      )) : 0;
      this.MaxValue       = this.hasAttribute ('maxValue'      ) ? Number (this.getAttribute ('maxValue'      )) : 4095;
      this.Units          = this.hasAttribute ('units'         ) ?         this.getAttribute ('units'         )  : '';
      this.AlarmLow       = this.hasAttribute ('alarmLow'      ) ? Number (this.getAttribute ('alarmLow'      )) : undefined;
      this.AlarmHigh      = this.hasAttribute ('alarmHigh'     ) ? Number (this.getAttribute ('alarmHigh'     )) : undefined;
      this.BackColor      = this.hasAttribute ('backColor'     ) ?         this.getAttribute ('backColor'     )  : '#303030';
      this.FillColor      = this.hasAttribute ('fillColor'     ) ?         this.getAttribute ('fillColor'     )  : '#A0A0A0';
      this.ScaleColor     = this.hasAttribute ('scaleColor'    ) ?         this.getAttribute ('scaleColor'    )  : undefined;
      this.ScalePlacement = this.hasAttribute ('scalePlacement') ?         this.getAttribute ('scalePlacement')  : 'left';
      this.OrgBorder = this.style.border;  // To restore after alarm conditions

      // Build this widget
      this.build ();

      //--- React to device data ---
      const self = this;
      $(document.body).on ('deviceData', function (event, nodeID, deviceID, timestamp, value)
      {
        // Match this UI widget to its Node and device
        if (nodeID == self.NodeID && deviceID == self.DeviceID)
        {
          // // Show current sampling rate on hover
          // if (!self.hasAttribute ('title'))
          //   if (Nodes[nodeID] != undefined)
          //     if (Nodes[nodeID].devices[deviceID] != undefined)
          //       self.setAttribute ('title', 'Current rate: ' + Nodes[nodeID].devices[deviceID].rate.toString() + ' s/hour');

          // Update this widget
          window.requestAnimationFrame.bind (self.updateWidget (Number(value)));
        }
      });
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- build ---------------------------------------------

  build = function ()
  {
    try
    {
      // Canvas dimensions are percentages of browser's size
      const cWidth  = Math.max (Math.round (this.Width  * GetBrowserWidth () / 100), 62);
      const cHeight = Math.max (Math.round (this.Height * GetBrowserHeight() / 100), 30);

      // Create a 2D data visualization canvas (dvCanvas2D)
      this.smacCanvas = new dvCanvas2D (cWidth, cHeight, 'transparent');

      // Check if a canvas already exists from previous build
      if (this.firstChild == undefined)
        this.appendChild  (this.smacCanvas.canvas);
      else
        this.replaceChild (this.smacCanvas.canvas, this.firstChild);  // Replace existing canvas

      // Set initial bar size
      this.BarWidth  = cWidth;
      this.BarHeight = cHeight;
      this.OffsetX   = 0;
      this.OffsetY   = 0;

      // Reduce bar area and adjust offsets for scale, if specified
      if (this.ScaleColor != undefined)
      {
        if (this.Orientation == 'horizontal')
        {
          this.BarWidth  -= 50;
          this.BarHeight -= 28;

          this.OffsetX = 25;
          if (this.ScalePlacement == 'top')
            this.OffsetY = 28;
        }
        else  // vertical
        {
          this.BarWidth  -= 60;
          this.BarHeight -= 10;

          this.OffsetY = 5;
          if (this.ScalePlacement == 'left')
            this.OffsetX = 60;
        }
      }

      // Back and Fill Gradients
      if (this.Orientation == 'horizontal')
      {
        this.BackGrad = this.smacCanvas.cc.createLinearGradient (this.OffsetX, this.OffsetY, this.OffsetX+this.BarWidth, this.OffsetY);
        this.BackGrad.addColorStop (0.0, this.smacCanvas.adjustBrightness (this.BackColor, -30));  // darker
        this.BackGrad.addColorStop (1.0, this.smacCanvas.adjustBrightness (this.BackColor,  30));  // lighter

        this.FillGrad = this.smacCanvas.cc.createLinearGradient (this.OffsetX, this.OffsetY, this.OffsetX+this.BarWidth, this.OffsetY);
        this.FillGrad.addColorStop (0.0, this.smacCanvas.adjustBrightness (this.FillColor, -30));  // darker
        this.FillGrad.addColorStop (1.0, this.smacCanvas.adjustBrightness (this.FillColor,  30));  // lighter
      }
      else
      {
        this.BackGrad = this.smacCanvas.cc.createLinearGradient (this.OffsetX, this.OffsetY, this.OffsetX, this.OffsetY+this.BarHeight);
        this.BackGrad.addColorStop (0.0, this.smacCanvas.adjustBrightness (this.BackColor,  30));  // lighter
        this.BackGrad.addColorStop (1.0, this.smacCanvas.adjustBrightness (this.BackColor, -30));  // darker

        this.FillGrad = this.smacCanvas.cc.createLinearGradient (this.OffsetX, this.OffsetY, this.OffsetX, this.OffsetY+this.BarHeight);
        this.FillGrad.addColorStop (0.0, this.smacCanvas.adjustBrightness (this.FillColor,  30));  // lighter
        this.FillGrad.addColorStop (1.0, this.smacCanvas.adjustBrightness (this.FillColor, -30));  // darker
      }

      // Clear bar area
      this.smacCanvas.drawRectangle (this.OffsetX, this.OffsetY, this.BarWidth, this.BarHeight, this.BackGrad, fill);

      // Manually calculate scale factor if scaleColor not specified
      if (this.ScaleColor == undefined)
        this.ScaleFactor = (this.orientation == 'horizontal' ? this.BarWidth : this.BarHeight) / (this.MaxValue - this.MinValue);
      else
      {
        // drawLinearScale returns the scale factor for you
        if (this.Orientation == 'horizontal')
        {
          if (this.ScalePlacement == 'top')
            this.ScaleFactor = this.smacCanvas.drawLinearScale (this.OffsetX, this.OffsetY  , this.BarWidth, this.BarHeight, ScaleOrientation.HorizTop   , this.MinValue, this.MaxValue, this.Units, this.ScaleColor);
          else  // bottom
            this.ScaleFactor = this.smacCanvas.drawLinearScale (this.OffsetX, this.BarHeight, this.BarWidth, this.BarHeight, ScaleOrientation.HorizBottom, this.MinValue, this.MaxValue, this.Units, this.ScaleColor);
        }
        else  // vertical
        {
          if (this.ScalePlacement == 'right')
            this.ScaleFactor = this.smacCanvas.drawLinearScale (this.BarWidth, this.BarHeight+this.OffsetY, this.BarWidth, this.BarHeight, ScaleOrientation.VertRight, this.MinValue, this.MaxValue, this.Units, this.ScaleColor);
          else  // left
            this.ScaleFactor = this.smacCanvas.drawLinearScale (this.OffsetX , this.BarHeight+this.OffsetY, this.BarWidth, this.BarHeight, ScaleOrientation.VertLeft , this.MinValue, this.MaxValue, this.Units, this.ScaleColor);
        }
      }

      // Set clipping region
      this.smacCanvas.setClipRectangle (this.OffsetX, this.OffsetY, this.BarWidth, this.BarHeight);
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- updateWidget --------------------------------------

  updateWidget = function (value)
  {
    try
    {
      let level = Math.round ((value - this.MinValue) * this.ScaleFactor);
      if (level < 0) level = 0;

      this.smacCanvas.drawRectangle (this.OffsetX, this.OffsetY, this.BarWidth, this.BarHeight, this.BackGrad, fill);

      if (this.Orientation == 'horizontal')
        this.smacCanvas.drawRectangle (this.OffsetX, this.OffsetY, level, this.BarHeight, this.FillGrad, fill);
      else
        this.smacCanvas.drawRectangle (this.OffsetX, this.OffsetY + this.BarHeight - level, this.BarWidth, level, this.FillGrad, fill);

      // Check alarm ranges
      if ((this.AlarmLow  != undefined && value <= this.AlarmLow ) ||
          (this.AlarmHigh != undefined && value >= this.AlarmHigh))
        this.style.border = '0.5vw solid #FF0000';
      else
        this.style.border = this.OrgBorder;
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }
};

customElements.define ('smac-growbar', SMAC_Growbar);


//=============================================================================
//  <smac-gauge> element
//=============================================================================

class SMAC_Gauge extends HTMLElement
{
  //--- Constructor ---------------------------------------

  constructor ()
  {
    super ();

    // Rebuild this widget on resize
    $(document.body).on ('browserResized', () => { this.build (); });

    // Right-click context menu
    this.addEventListener ('contextmenu', function (event)
    {
      StopEvent (event);

      // ...
    });
  }

  //--- Attributes ----------------------------------------

  get device      (     ) { return this.Device;       }
  set device      (value) { this.Device = value;      }

  get diameter    (     ) { return this.Diameter;     }
  set diameter    (value) { this.Diameter = value;    }

  get startAngle  (     ) { return this.StartAngle;   }
  set startAngle  (value) { this.StartAngle = value;  }

  get stopAngle   (     ) { return this.StopAngle;    }
  set stopAngle   (value) { this.StopAngle = value;   }

  get minValue    (     ) { return this.MinValue;     }
  set minValue    (value) { this.MinValue = value;    }

  get maxValue    (     ) { return this.MaxValue;     }
  set maxValue    (value) { this.MaxValue = value;    }

  get units       (     ) { return this.Units;        }
  set units       (value) { this.Units = value;       }

  get alarmLow    (     ) { return this.AlarmLow;     }
  set alarmLow    (value) { this.AlarmLow = value;    }

  get alarmHigh   (     ) { return this.AlarmHigh;    }
  set alarmHigh   (value) { this.AlarmHigh = value;   }

  get backColor   (     ) { return this.BackColor;    }
  set backColor   (value) { this.BackColor = value;   }

  get needleColor (     ) { return this.NeedleColor;  }
  set needleColor (value) { this.NeedleColor = value; }

  get scaleColor  (     ) { return this.ScaleColor;   }
  set scaleColor  (value) { this.ScaleColor = value;  }

  //--- connectedCallback ---------------------------------

  connectedCallback ()
  {
    try
    {
      // // Create shadow root
      // this.shadow = this.attachShadow ({mode: 'open'});

      // Check for required attributes
      if (!this.hasAttribute ('device'))
        throw '(smac-gauge): Missing device attribute';

      const deviceInfo = this.getAttribute ('device').replaceAll (' ', '').split (',');
      this.NodeID   = parseInt (deviceInfo[0]);
      this.DeviceID = parseInt (deviceInfo[1]);

      SetAsInlineBlock (this);

      // Set optional attributes
      this.Diameter    = this.hasAttribute ('diameter'   ) ? Number (this.getAttribute ('diameter'  )) : 30;
      this.StartAngle  = this.hasAttribute ('startAngle' ) ? Number (this.getAttribute ('startAngle')) : -150;
      this.StopAngle   = this.hasAttribute ('stopAngle'  ) ? Number (this.getAttribute ('stopAngle' )) : 150;
      this.MinValue    = this.hasAttribute ('minValue'   ) ? Number (this.getAttribute ('minValue'  )) : 0;
      this.MaxValue    = this.hasAttribute ('maxValue'   ) ? Number (this.getAttribute ('maxValue'  )) : 4095;
      this.Units       = this.hasAttribute ('units'      ) ?         this.getAttribute ('units'     )  : undefined;
      this.AlarmLow    = this.hasAttribute ('alarmLow'   ) ? Number (this.getAttribute ('alarmLow'  )) : undefined;
      this.AlarmHigh   = this.hasAttribute ('alarmHigh'  ) ? Number (this.getAttribute ('alarmHigh' )) : undefined;
      this.BackColor   = this.hasAttribute ('backColor'  ) ?         this.getAttribute ('backColor'  ) : '#303030';
      this.NeedleColor = this.hasAttribute ('needleColor') ?         this.getAttribute ('needleColor') : '#C0C0C0';
      this.ScaleColor  = this.hasAttribute ('scaleColor' ) ?         this.getAttribute ('scaleColor' ) : undefined;
      this.Fill        = this.hasAttribute ('fill'       );
      this.OrgBorder = this.style.border;  // To restore after alarm conditions

      // Build this widget
      this.build ();

      //--- React to device data ---
      const self = this;
      $(document.body).on ('deviceData', function (event, nodeID, deviceID, timestamp, value)
      {
        // Match this UI widget to its Node and device
        if (nodeID == self.NodeID && deviceID == self.DeviceID)
        {
          // // Show current sampling rate on hover
          // if (!self.hasAttribute ('title'))
          //   if (Nodes[nodeID] != undefined)
          //     if (Nodes[nodeID].devices[deviceID] != undefined)
          //       self.setAttribute ('title', 'Current rate: ' + Nodes[nodeID].devices[deviceID].rate.toString() + ' s/hour');

          // Update this widget
          window.requestAnimationFrame.bind (self.updateWidget (Number(value)));
        }
      });
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- build ---------------------------------------------

  build = function ()
  {
    try
    {
      // Canvas dimensions are percentage of browser's client size
      const cSize   = Math.max  (this.Diameter * GetBrowserHeight() / 100.0, 120);
      const cWidth  = Math.ceil (cSize);
      let   cHeight = cWidth;
      if (this.ScaleColor != undefined)
        cHeight -= Math.ceil (8000 / cSize);  // reduce height if scale drawn

      // Create a 2D data visualization canvas (dvCanvas2D)
      this.smacCanvas = new dvCanvas2D (cWidth, cHeight, 'transparent');

        // Check if a canvas already exists from previous build
      if (this.firstChild == undefined)
        this.appendChild  (this.smacCanvas.canvas);
      else
        this.replaceChild (this.smacCanvas.canvas, this.firstChild);  // Replace existing canvas

      // Set gauge radius and offsets
      this.Radius  = Math.floor (cSize / 2);
      this.OffsetX = 0;
      this.OffsetY = 1;

      if (this.ScaleColor != undefined)
      {
        // Adjust for scale
        this.Radius  -= 50;
        this.OffsetX += 50;
        this.OffsetY += Math.floor (48 - 2200/cHeight);
      }

      this.PivotRadius = Math.round (this.Radius / 10);

      // Needle Thickness
      this.NeedleThickness = Math.round (this.PivotRadius / 2);
      if (this.NeedleThickness < 1) this.NeedleThickness = 1;

      // Save center of gauge
      this.CenterX = this.OffsetX + this.Radius;
      this.CenterY = this.OffsetY + this.Radius;

      // Gradients
      this.BackGrad = this.smacCanvas.cc.createRadialGradient (this.CenterX, this.CenterY, 0, this.CenterX, this.CenterY, this.Radius);
      this.BackGrad.addColorStop (0.0 , this.smacCanvas.adjustBrightness (this.BackColor,  40));  // lighter
      this.BackGrad.addColorStop (0.95, this.smacCanvas.adjustBrightness (this.BackColor, -20));  // darker
      this.BackGrad.addColorStop (1.0 , '#404040');  // edge shadow

      this.FillGrad = this.smacCanvas.cc.createRadialGradient (this.CenterX, this.CenterY, 0, this.CenterX, this.CenterY, this.Radius);
      this.FillGrad.addColorStop (0.0 , this.smacCanvas.adjustBrightness (this.NeedleColor,  40));  // lighter
      this.FillGrad.addColorStop (0.95, this.smacCanvas.adjustBrightness (this.NeedleColor, -20));  // darker
      this.FillGrad.addColorStop (1.0 , '#404040');  // edge shadow

      this.PivotGrad = this.smacCanvas.cc.createLinearGradient (this.CenterX, this.CenterY-this.PivotRadius, this.CenterX, this.CenterY+this.PivotRadius);
      this.PivotGrad.addColorStop (0.0, '#F0F0F0');
      this.PivotGrad.addColorStop (1.0, '#808080');

      // Clear gauge area
      this.smacCanvas.drawPie (this.CenterX, this.CenterY, this.Radius, this.BackGrad, this.StartAngle, this.StopAngle);
      this.smacCanvas.drawPie (this.CenterX, this.CenterY, this.PivotRadius, this.PivotGrad, 0, 360);

      // Set scale factor
      if (this.MaxValue <= this.MinValue)
        this.ScaleFactor = 1.0;
      else
        this.ScaleFactor = (this.StopAngle - this.StartAngle) / (this.MaxValue - this.MinValue);

      // Draw scale if specified
      if (this.ScaleColor != undefined)
        this.smacCanvas.drawCircularScale (this.CenterX, this.CenterY, this.Radius, this.MinValue, this.MaxValue, this.StartAngle, this.StopAngle, this.Units, this.ScaleColor);
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- updateWidget --------------------------------------

  updateWidget = function (value)
  {
    try
    {
      // Clamp to min/max values
      value = Clamp (value, this.MinValue, this.MaxValue);

      const valueAngle = (value - this.MinValue) * this.ScaleFactor + this.StartAngle;

      // Clear gauge
      this.smacCanvas.drawPie (this.CenterX, this.CenterY, this.Radius, this.BackGrad, this.StartAngle, this.StopAngle);

      if (this.Fill)
      {
        // Fill pie slice
        this.smacCanvas.drawPie (this.CenterX, this.CenterY, this.Radius, this.FillGrad, this.StartAngle, valueAngle);
      }
      else
      {
        // Draw Needle
        const radAngle = (valueAngle - 90) * deg2rad;
        const x = this.CenterX + this.Radius * Math.cos (radAngle);
        const y = this.CenterY + this.Radius * Math.sin (radAngle);

        this.smacCanvas.drawLine (this.CenterX, this.CenterY, x, y, this.NeedleColor, this.NeedleThickness);
      }

      // Draw Pivot
      this.smacCanvas.drawPie (this.CenterX, this.CenterY, this.PivotRadius, this.PivotGrad, 0, 360);

      // Check alarm ranges
      if ((this.AlarmLow  != undefined && value <= this.AlarmLow ) ||
          (this.AlarmHigh != undefined && value >= this.AlarmHigh))
        this.style.border = '0.5vw solid #FF0000';
      else
        this.style.border = this.OrgBorder;
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }
};

customElements.define ('smac-gauge', SMAC_Gauge);


//=============================================================================
//  <smac-compass> element
//=============================================================================

class SMAC_Compass extends HTMLElement
{
  //--- Constructor ---------------------------------------

  constructor ()
  {
    super ();

    // Rebuild this widget on resize
    $(document.body).on ('browserResized', () => { this.build (); });

    // Right-click context menu
    this.addEventListener ('contextmenu', function (event)
    {
      StopEvent (event);

      // ...
    });
  }

  //--- Attributes ----------------------------------------

  get device    (     ) { return this.Device;     }
  set device    (value) { this.Device = value;    }

  get diameter  (     ) { return this.Diameter;   }
  set diameter  (value) { this.Diameter = value;  }

  get faceColor (     ) { return this.FaceColor;  }
  set faceColor (value) { this.FaceColor = value; }

  //--- connectedCallback ---------------------------------

  connectedCallback ()
  {
    try
    {
      // // Create shadow root
      // this.shadow = this.attachShadow ({mode: 'open'});

      // Check for required attributes
      if (!this.hasAttribute ('device'))
        throw '(smac-compass): Missing device attribute';

      const deviceInfo = this.getAttribute ('device').replaceAll (' ', '').split (',');
      this.NodeID   = parseInt (deviceInfo[0]);
      this.DeviceID = parseInt (deviceInfo[1]);

      SetAsInlineBlock (this);

      // Set optional attributes
      this.Diameter  = this.hasAttribute ('diameter' ) ? Number (this.getAttribute ('diameter' )) : 30;
      this.FaceColor = this.hasAttribute ('faceColor') ?         this.getAttribute ('faceColor' ) : '#202030';
      this.OrgBorder = this.style.border;  // To restore after alarm conditions

      // Build this widget
      this.build ();

      //--- React to device data ---
      const self = this;
      $(document.body).on ('deviceData', function (event, nodeID, deviceID, timestamp, value)
      {
        // Match this UI widget to its Node and device
        if (nodeID == self.NodeID && deviceID == self.DeviceID)
        {
          // // Show current sampling rate on hover
          // if (!self.hasAttribute ('title'))
          //   if (Nodes[nodeID] != undefined)
          //     if (Nodes[nodeID].devices[deviceID] != undefined)
          //       self.setAttribute ('title', 'Current rate: ' + Nodes[nodeID].devices[deviceID].rate.toString() + ' s/hour');

          // Update this widget
          window.requestAnimationFrame.bind (self.updateWidget (Number(value)));
        }
      });
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- build ---------------------------------------------

  build = function ()
  {
    try
    {
      // Canvas dimensions are percentage of browser's client size
      const cSize = Math.ceil (Math.max (this.Diameter * GetBrowserHeight() / 100.0, 120));

      // Create a 2D data visualization canvas (dvCanvas2D)
      this.smacCanvas = new dvCanvas2D (cSize, cSize, 'transparent');

      // Check if a canvas already exists from previous build
      if (this.firstChild == undefined)
        this.appendChild  (this.smacCanvas.canvas);
      else
        this.replaceChild (this.smacCanvas.canvas, this.firstChild);  // Replace existing canvas

      // Save center of compass
      this.Center = Math.round (cSize / 2);

      // Set various radii
      const outerRadius = Math.round (cSize / 2) - 1;
      const innerRadius = Math.round (0.95 * outerRadius);

      this.NeedleRadius = innerRadius - 55;
      this.PivotRadius  = Math.round (this.NeedleRadius / 6);

      // Draw outer sections of compass
      const outerGrad = this.smacCanvas.cc.createLinearGradient (0, 0, cSize, cSize);
      outerGrad.addColorStop (0.0, '#F0F0F0');
      outerGrad.addColorStop (1.0, '#202020');

      const innerGrad = this.smacCanvas.cc.createLinearGradient (0, 0, cSize, cSize);
      innerGrad.addColorStop (0.0, '#404040');
      innerGrad.addColorStop (1.0, '#808080');

      this.FaceGrad = this.smacCanvas.cc.createRadialGradient (this.Center, this.Center, 0, this.Center, this.Center, this.NeedleRadius);
      this.FaceGrad.addColorStop (0.0 , this.smacCanvas.adjustBrightness (this.FaceColor,  30));  // lighter
      this.FaceGrad.addColorStop (0.95, this.smacCanvas.adjustBrightness (this.FaceColor, -30));  // darker
      this.FaceGrad.addColorStop (1.0 , '#404040');  // edge shadow

      this.NeedleRedGrad = this.smacCanvas.cc.createLinearGradient (this.Center-this.NeedleRadius, this.Center-this.NeedleRadius, this.Center+this.NeedleRadius, this.Center+this.NeedleRadius);
      this.NeedleRedGrad.addColorStop (0.0, '#FF0000');
      this.NeedleRedGrad.addColorStop (1.0, '#800000');

      this.NeedleWhiteGrad = this.smacCanvas.cc.createLinearGradient (this.Center-this.NeedleRadius, this.Center-this.NeedleRadius, this.Center+this.NeedleRadius, this.Center+this.NeedleRadius);
      this.NeedleWhiteGrad.addColorStop (0.0, '#FFFFFF');
      this.NeedleWhiteGrad.addColorStop (1.0, '#808080');

      this.OuterPivotGrad = this.smacCanvas.cc.createLinearGradient (this.Center-this.PivotRadius, this.Center-this.PivotRadius, this.Center+this.PivotRadius, this.Center+this.PivotRadius);
      this.OuterPivotGrad.addColorStop (0.0, '#F0F0F0');
      this.OuterPivotGrad.addColorStop (1.0, '#202020');

      this.InnerPivotGrad = this.smacCanvas.cc.createLinearGradient (this.Center-this.PivotRadius, this.Center-this.PivotRadius, this.Center+this.PivotRadius, this.Center+this.PivotRadius);
      this.InnerPivotGrad.addColorStop (0.0, '#202020');
      this.InnerPivotGrad.addColorStop (1.0, '#F0F0F0');

      this.smacCanvas.drawEllipse       (this.Center, this.Center, outerRadius, outerRadius, outerGrad, 0);
      this.smacCanvas.drawEllipse       (this.Center, this.Center, innerRadius, innerRadius, innerGrad, 0);
      this.smacCanvas.drawCircularScale (this.Center, this.Center, this.NeedleRadius, 0, 360, 0, 360, '°', '#F0F0F0');

      // N, E, S, W
      const fontSize   = Math.floor (cSize / 18);
      const font       = fontSize.toString() + 'px font_beamweaponCondensed';
      this.smacCanvas.drawText (this.Center-fontSize*0.3        , this.Center-innerRadius+1       , 'n', '#FFFF00', font);
      this.smacCanvas.drawText (this.Center-fontSize*0.3        , this.Center+innerRadius-fontSize, 's', '#FFFF00', font);
      this.smacCanvas.drawText (this.Center-innerRadius+1       , this.Center-fontSize*0.5        , 'w', '#FFFF00', font);
      this.smacCanvas.drawText (this.Center+innerRadius-fontSize, this.Center-fontSize*0.5        , 'e', '#FFFF00', font);

      // First draw
      this.updateWidget (0, 255);
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- updateWidget --------------------------------------

  updateWidget = function (value)
  {
    try
    {
      // Clamp to min/max values
      value = Clamp (value, 0, 359.999);

      // Clear compass
      this.smacCanvas.drawEllipse (this.Center, this.Center, this.NeedleRadius-1, this.NeedleRadius-1, this.FaceGrad, fill);

      // Draw Needle
      let needleAngle = (90 - value) * deg2rad;
      let needleX     = this.NeedleRadius     * Math.cos (needleAngle);
      let needleY     = this.NeedleRadius     * Math.sin (needleAngle);
      let pivotX      = this.PivotRadius*0.75 * Math.cos (needleAngle + piOver2);
      let pivotY      = this.PivotRadius*0.75 * Math.sin (needleAngle + piOver2);

      let vertices = [3];
      vertices[0] = { x:this.Center+pivotX , y:this.Center-pivotY  };
      vertices[1] = { x:this.Center+needleX, y:this.Center-needleY };
      vertices[2] = { x:this.Center-pivotX , y:this.Center+pivotY  };
      this.smacCanvas.drawPoly (vertices, this.NeedleRedGrad, fill);

      vertices[0] = { x:this.Center-pivotX , y:this.Center+pivotY  };
      vertices[1] = { x:this.Center-needleX * 0.6, y:this.Center+needleY * 0.6 };
      vertices[2] = { x:this.Center+pivotX , y:this.Center-pivotY  };
      this.smacCanvas.drawPoly (vertices, this.NeedleWhiteGrad, fill);

      this.smacCanvas.drawEllipse (this.Center, this.Center, this.PivotRadius    , this.PivotRadius    , this.OuterPivotGrad, fill);
      this.smacCanvas.drawEllipse (this.Center, this.Center, this.PivotRadius*0.8, this.PivotRadius*0.8, this.InnerPivotGrad, fill);


//       // Check alarm ranges
//       if ((this.AlarmLow  != undefined && value <= this.AlarmLow ) ||
//           (this.AlarmHigh != undefined && value >= this.AlarmHigh))
//         this.style.border = '0.5vw solid #FF0000';
//       else
//         this.style.border = this.OrgBorder;



    }
    catch (ex)
    {
      ShowException (ex);
    }
  }
};

customElements.define ('smac-compass', SMAC_Compass);


//=============================================================================
//  <smac-timegraph> element
//=============================================================================

class SMAC_TimeGraph extends HTMLElement
{
  //--- Constructor ---------------------------------------

  constructor ()
  {
    super ();

    // Rebuild this widget on resize
    $(document.body).on ('browserResized', () => { this.build (); });

    // Right-click context menu
    this.addEventListener ('contextmenu', function (event)
    {
      StopEvent (event);

      // ...
    });
  }

  //--- Attributes ----------------------------------------

  get devices        (     ) { return this.Devices;         }
  set devices        (value) { this.Devices = value;        }

  get minValue       (     ) { return this.MinValue;        }
  set minValue       (value) { this.MinValue = value;       }

  get maxValue       (     ) { return this.MaxValue;        }
  set maxValue       (value) { this.MaxValue = value;       }

  get alarmLow       (     ) { return this.AlarmLow;        }
  set alarmLow       (value) { this.AlarmLow = value;       }

  get alarmHigh      (     ) { return this.AlarmHigh;       }
  set alarmHigh      (value) { this.AlarmHigh = value;      }

  get timeUnit       (     ) { return this.TimeUnit;        }
  set timeUnit       (value) { this.TimeUnit = value;       }

  get timeSpan       (     ) { return this.TimeSpan;        }
  set timeSpan       (value) { this.TimeSpan = value;       }

  get width          (     ) { return this.Width;           }
  set width          (value) { this.Width = value;          }

  get height         (     ) { return this.Height;          }
  set height         (value) { this.Height = value;         }

  get backColor      (     ) { return this.BackColor;       }
  set backColor      (value) { this.BackColor = value;      }

  get gridColor      (     ) { return this.GridColor;       }
  set gridColor      (value) { this.GridColor = value;      }

  get scaleColor     (     ) { return this.ScaleColor;      }
  set scaleColor     (value) { this.ScaleColor = value;     }

  get scalePlacement (     ) { return this.ScalePlacement;  }
  set scalePlacement (value) { this.ScalePlacement = value; }

  //--- connectedCallback ---------------------------------

  connectedCallback ()
  {
    try
    {
      // // Create shadow root
      // this.shadow = this.attachShadow ({mode: 'open'});

      // Check for required attributes
      if (!this.hasAttribute ('devices'))
        throw '(smac-timegraph): Missing devices attribute';

      // Build multi-device arrays
      const deviceArray = this.getAttribute ('devices').replaceAll (' ', '').split ('|');
      // deviceArray[0] = 'nodeID,deviceID,plotColor'
      // deviceArray[1] = 'nodeID,deviceID,plotColor'
      //  :

      this.NodeID    = [];
      this.DeviceID  = [];
      this.PlotColor = [];

      let deviceInfo;
      const self = this;
      deviceArray.forEach ((item) =>
      {
        deviceInfo = item.split (',');
        self.NodeID   .push (parseInt (deviceInfo[0]));
        self.DeviceID .push (parseInt (deviceInfo[1]));
        self.PlotColor.push (          deviceInfo[2]);
      });

      SetAsInlineBlock (this);

      // Set optional attributes
      this.MinValue       = this.hasAttribute ('minValue'      ) ? Number (this.getAttribute ('minValue'     )) : 0;
      this.MaxValue       = this.hasAttribute ('maxValue'      ) ? Number (this.getAttribute ('maxValue'     )) : 4095;
      this.AlarmLow       = this.hasAttribute ('alarmLow'      ) ? Number (this.getAttribute ('alarmLow'     )) : undefined;
      this.AlarmHigh      = this.hasAttribute ('alarmHigh'     ) ? Number (this.getAttribute ('alarmHigh'    )) : undefined;
      this.TimeUnit       = this.hasAttribute ('timeUnit'      ) ?         this.getAttribute ('timeUnit'     )  : 'sec';
      this.TimeSpan       = this.hasAttribute ('timeSpan'      ) ? Number (this.getAttribute ('timeSpan'     )) : 60;
      this.Width          = this.hasAttribute ('width'         ) ? Number (this.getAttribute ('width'        )) : 32;
      this.Height         = this.hasAttribute ('height'        ) ? Number (this.getAttribute ('height'       )) : 18;
      this.BackColor      = this.hasAttribute ('backColor'     ) ?         this.getAttribute ('backColor'     ) : '#303030';
      this.GridColor      = this.hasAttribute ('gridColor'     ) ?         this.getAttribute ('gridColor'     ) : undefined;
      this.ScaleColor     = this.hasAttribute ('scaleColor'    ) ?         this.getAttribute ('scaleColor'    ) : undefined;
      this.ScalePlacement = this.hasAttribute ('scalePlacement') ?         this.getAttribute ('scalePlacement') : 'leftBottom';
      this.OrgBorder = this.style.border;  // To restore after alarm conditions

      // Build this widget
      this.build ();

      //--- React to device data ---
      $(document.body).on ('deviceData', function (event, nodeID, deviceID, timestamp, value)
      {
        // This is a multi-device display
        // Match this UI widget to any of its devices
        for (let i=0; i<self.NodeID.length; i++)
        {
          if (nodeID == self.NodeID[i] && deviceID == self.DeviceID[i])
          {
            // // Show current sampling rate on hover
            // if (!self.hasAttribute ('title'))
            //   if (Nodes[nodeID] != undefined)
            //     if (Nodes[nodeID].devices[deviceID] != undefined)
            //       self.setAttribute ('title', 'Current rate: ' + Nodes[nodeID].devices[deviceID].rate.toString() + ' s/hour');

            // Update this widget
            window.requestAnimationFrame.bind (self.updateWidget (i, Number(timestamp), Number(value)));
            break;
          }
        }
      });
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- build ---------------------------------------------

  build = function ()
  {
    try
    {
      // Canvas dimensions are percentages of Browser's size
      const cWidth  = Math.max (Math.round (this.Width  * GetBrowserWidth () / 100), 90);
      const cHeight = Math.max (Math.round (this.Height * GetBrowserHeight() / 100), 45);

      // Create a 2D data visualization canvas (dvCanvas2D)
      this.smacCanvas = new dvCanvas2D (cWidth, cHeight, 'transparent');

      // Check if a canvas already exists from a previous build
      if (this.firstChild == undefined)
        this.appendChild  (this.smacCanvas.canvas);
      else
        this.replaceChild (this.smacCanvas.canvas, this.firstChild);  // Replace existing canvas

      // Set initial size of graph area
      this.GraphWidth  = cWidth;
      this.GraphHeight = cHeight;
      this.OffsetX     = 0;
      this.OffsetY     = 0;

      // Reduce graph area and adjust offsets for scale, if specified
      if (this.ScaleColor != undefined)
      {
        this.GraphWidth  -= 80;
        this.GraphHeight -= 35;

             if (this.ScalePlacement == 'leftTop'    ) { this.OffsetX = 60; this.OffsetY = 30; }
        else if (this.ScalePlacement == 'leftBottom' ) { this.OffsetX = 60; this.OffsetY =  7; }
        else if (this.ScalePlacement == 'rightTop'   ) { this.OffsetX = 20; this.OffsetY = 30; }
        else if (this.ScalePlacement == 'rightBottom') { this.OffsetX = 20; this.OffsetY =  7; }
      }

      // Set time factor (convert timestamp from milliseconds)
      this.TimeFactor = 1;
      switch (this.TimeUnit)
      {
        case 'sec'  : this.TimeFactor =             1000; break;
        case 'min'  : this.TimeFactor =        60 * 1000; break;
        case 'hour' : this.TimeFactor =      3600 * 1000; break;
        case 'day'  : this.TimeFactor = 24 * 3600 * 1000; break;
      }

      // Set scale factors
      this.ScaleFactorX = this.GraphWidth / this.TimeSpan;
      if (this.MaxValue <= this.MinValue)
        this.ScaleFactorY = 1.0;
      else
        this.ScaleFactorY = this.GraphHeight / (this.MaxValue - this.MinValue);

      //--- Timing and plot start ---
      this.StartStamp = [];
      this.PrevX      = [];
      this.PrevY      = [];
      for (let i=0; i<this.NodeID.length; i++)
      {
        this.StartStamp[i] = -1;
        this.PrevX     [i] = 0;
        this.PrevY     [i] = 0;
      }

      // Background gradient
      this.BackGrad = this.smacCanvas.cc.createLinearGradient (this.OffsetX, this.OffsetY, this.OffsetX, this.OffsetY+this.GraphHeight);
      this.BackGrad.addColorStop (0.0, this.BackColor);
      this.BackGrad.addColorStop (1.0, this.smacCanvas.adjustBrightness (this.BackColor, -20));  // darker

      // Initial drawing
      this.updateWidget (0, 0, 0);

      // Draw X-Y Scales
      if (this.ScaleColor != undefined)
      {
        // Time scale
        if (this.ScalePlacement.endsWith ('Top'))
          this.smacCanvas.drawLinearScale (this.OffsetX, this.OffsetY                 , this.GraphWidth, this.GraphHeight, ScaleOrientation.HorizTop   , 0.0, this.TimeSpan, this.TimeUnit, this.ScaleColor, this.GridColor);  // X-Axis
        else
          this.smacCanvas.drawLinearScale (this.OffsetX, this.OffsetY+this.GraphHeight, this.GraphWidth, this.GraphHeight, ScaleOrientation.HorizBottom, 0.0, this.TimeSpan, this.TimeUnit, this.ScaleColor, this.GridColor);  // X-Axis

        // Value scale
        if (this.ScalePlacement.startsWith ('right'))
          this.smacCanvas.drawLinearScale (this.OffsetX+this.GraphWidth, this.OffsetY+this.GraphHeight, this.GraphWidth, this.GraphHeight, ScaleOrientation.VertRight, this.MinValue, this.MaxValue, '', this.ScaleColor, this.GridColor);  // Y-Axis
        else
          this.smacCanvas.drawLinearScale (this.OffsetX                , this.OffsetY+this.GraphHeight, this.GraphWidth, this.GraphHeight, ScaleOrientation.VertLeft , this.MinValue, this.MaxValue, '', this.ScaleColor, this.GridColor);  // Y-Axis
      }

      // Set clipping region
      this.smacCanvas.setClipRectangle (this.OffsetX, this.OffsetY, this.GraphWidth, this.GraphHeight);
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }

  //--- updateWidget --------------------------------------

  updateWidget = function (i, timestamp, value)
  {
    try
    {
      let refreshGrid = false;
      let cx = 0;
      let cy = 0;

      // Check flag to refresh grid
      if (this.StartStamp[i] < 0)
        refreshGrid = true;
      else
      {
        cx = Math.round ((timestamp - this.StartStamp[i]) / this.TimeFactor * this.ScaleFactorX);

        // Check if past end of graph
        if (cx > this.GraphWidth)
          refreshGrid = true;
      }

      // Calculate next graphic y value
      cy = this.GraphHeight - Math.round ((value - this.MinValue) * this.ScaleFactorY);
      // if (cy < 0) cy = 0;

      // Does grid need refreshing?
      if (refreshGrid)
      {
        this.StartStamp[i] = timestamp;
        this.smacCanvas.drawRectangle (this.OffsetX, this.OffsetY, this.GraphWidth, this.GraphHeight, this.BackGrad, fill);

        // Do not redraw scales
        // Draw only the grid by setting scaleColor param to undefined
        if (this.GridColor != undefined)
        {
          // Time scale
          if (this.ScalePlacement.endsWith ('Top'))
            this.smacCanvas.drawLinearScale (this.OffsetX, this.OffsetY                 , this.GraphWidth, this.GraphHeight, ScaleOrientation.HorizTop   , 0.0, this.TimeSpan, this.TimeUnit, undefined, this.GridColor);  // X-Axis
          else
            this.smacCanvas.drawLinearScale (this.OffsetX, this.OffsetY+this.GraphHeight, this.GraphWidth, this.GraphHeight, ScaleOrientation.HorizBottom, 0.0, this.TimeSpan, this.TimeUnit, undefined, this.GridColor);  // X-Axis

          // Value scale
          if (this.ScalePlacement.startsWith ('right'))
            this.smacCanvas.drawLinearScale (this.OffsetX+this.GraphWidth, this.OffsetY+this.GraphHeight, this.GraphWidth, this.GraphHeight, ScaleOrientation.VertRight, this.MinValue, this.MaxValue, '', undefined, this.GridColor);  // Y-Axis
          else
            this.smacCanvas.drawLinearScale (this.OffsetX                , this.OffsetY+this.GraphHeight, this.GraphWidth, this.GraphHeight, ScaleOrientation.VertLeft , this.MinValue, this.MaxValue, '', undefined, this.GridColor);  // Y-Axis
        }

        this.PrevX[i] = 0;
        this.PrevY[i] = cy;
      }
      else
      {
        this.smacCanvas.drawLine (this.PrevX[i]+this.OffsetX, this.PrevY[i]+this.OffsetY, cx+this.OffsetX, cy+this.OffsetY, this.PlotColor[i], 2);

        this.PrevX[i] = cx;
        this.PrevY[i] = cy;
      }

      // Check alarm ranges
      if ((this.AlarmLow  != undefined && value <= this.AlarmLow ) ||
          (this.AlarmHigh != undefined && value >= this.AlarmHigh))
        this.style.border = '0.5vw solid #FF0000';
      else
        this.style.border = this.OrgBorder;
    }
    catch (ex)
    {
      ShowException (ex);
    }
  }
};

customElements.define ('smac-timegraph', SMAC_TimeGraph);

