=============================
==                         ==
== AudioOutputHost example ==
==                         ==
=============================
This example implements an audio interface class host mode device that enumerates
an audio interface class device (USB speakers) and sends samples to the device.

The samples are created by a simple square wave generator in a timer ISR and
shipped out the streaming isochronous output pipe when a button is pressed on
the board.


=====================
==                 ==
== Getting Started ==
==                 ==
=====================

This example can be built and debugged using two compiler tool chains.
Visit http://www.lpcware.com/content/project/LPCUSBlib/build-it to learn
how to setup and configure these tool chains to build this example.


===================================
==                               ==
== Compiler build configurations ==
==                               ==
===================================

The software that makes up this example is designed to run on many different
development boards. Therefore the compiler must be configured to build an image
that is targeted to the development board you are using. This section explains
how to set these build configurations for each of the supported compilers.


LPCXpresso IDE
--------------

Configure projects:
  right click on the BSP project in the project explorer window
  click Build Configurations->Set Active->(see configuration in table below)
  repeat these steps with the CDL, LPCUSBlib, and Example_AudioOutputHost

Configure MCU:
  right click on the Example_AudioOutputHost project in the project explorer window
  click Properties->C/C++ Build->MCU settings->(see configuration in table below)

Configure the indexer:
   click Window->Preferences->C/C++->Indexer->Use active build configuration
  
                                BSP               CDL       LPCUSBlib    Example_AudioOutputHost    MCU
                        ======================  =======    ============  =======================  =======
LPC1850  / Hitex        LPC1850_HITEX_A4        LPC18xx    LPC18xx_Host        LPC18xx            LPC1850
LPC4350  / Hitex        LPC18xx_HITEX_A4        LPC43xx_M4 LPC43xx_Host        LPC43xx M4         LPC4350
LPC1768  / MCB1700      LPC1768_MCB1700         LPC17xx    LPC17xx_Host        LPC17xx            LPC1768
LPC1769  / LPCXpresso   LPC17xx_LPCXpressoRevB  LPC17xx    LPC17xx_Host        LPC17xx            LPC1769
 


Keil uVision 4 IDE
------------------

Configure a Batch Build:
  click on Project->Batch Build...
  expand the BSP project to display a list of project targets
  select the project target by checking the appropriate checkbox
    (see target list in table below)
  repeat these steps with the CDL, LPCUSBlib_Device, and Example_AudioOutputHost

                         BSP       CDL    LPCUSBlib_Host Example_AudioOutputHost 
                     ========== ========= ============== =======================
LPC1768  / MCB1700    MCB1700    LPC17xx   LPC17xx_Host         MCB1700
LPC1850  / Hitex      HITEX1800  LPC18xx   LPC18xx_Host         HITEX1800
LPC4350  / Hitex      HITEX4300  LPC43xx   LPC43xx_Host         HITEX4300
LPC1830  / Xplorer    Xplorer    LPC18xx   LPC18xx_Host         XPLORER1830
LPC4330  / Xplorer    Xplorer    LPC43xx   LPC43xx_Host         XPLORER4330
LPC4330  / Element14  Xplorer    LPC43xx   LPC43xx_Host         XPLORER4330



==========================================
==                                      ==
== Board connections and configurations ==
==                                      ==
==========================================


LPC1768_MCB1700 (Keil MCB1700 development board)
------------------------------------------------
schematic: ./BSP/schematics/mcb1700-schematics.pdf

Jumpers:
  E/U    = 1-2
  UMODE  = 1-2
  D-     = Host
  D+     = Host
  VBUS   = 1-2
  VDDIO  = 1-2
  VDDREG = 1-2
  ISP    = removed
  RST    = removed
Connect the JTAG debugger to the 20 pin JTAG or 10 pin Cortex Debug connector
Connect a 9 pin serial cable between COM0 on the board and a PC running
  a terminal program. Baud rate = 9600
Plug a USB flash drive into the type A receptacle marked HOST
To power the board:
  Connect the type-B end of a powered USB cable to the type-B receptacle
    labeled 'Device'
  or
  Supply 5v to the -VIN+ pins between the JTAG and COM0 receptacles
Once the board is powered the following LEDs should be illuminated:
    Power
    100MHz
    USC


LPC18xx_HITEX (LPC1850/LPC4350 Hitex Evaluation Board)
---------------------------------------------------------------
schematic: ./BSP/schematics/SCM_LPC1850EVA-A4-2.pdf

Connect the JTAG to the 20 pin receptacle marked X4 JTAG
Plug USB speakers into the type A receptacle marked X10
Connect a 9-pin serial cable into the receptacle marked UART X1. Connect
  the other end of this cable into a PC running a terminal program.
To power the board:
  Connect the type-B end of a powered USB cable into the receptacle marked X13
  or
  Supply 5v to the DC connector labeled X14  


LPC177x_8x_EAOEMBaseRevA (LPC177x_8x on the Embedded Artist OEM RevA basse board)
------------------------------------------------
schematic: visit http://www.embeddedartists.com

Jumpers:
  JP15 = NOT installed (Device detect)
  
Connect the JTAG debugger to the 20 pin JTAG or 10 pin Cortex Debug connector.
Plug USB speakers into the type A receptacle marked J20
Connect the type-B end of a USB mini cable to the receptacle marked J25 
  (next to the DC connector) and the type-A end of that cable into a PC.
Press the button marked SW6 to send square wave samples to the USB speakers
	

======================
==                  ==
== PC configuration ==
==                  ==
======================


Use a PC running Windows XP, Vista or 7 to display runtime diagnostics
that can be viewed with a terminal emulation program like Teraterm or
Hyperterm. The baud rate in these programs should be set to 9600.



=================================
==                             ==
== Build and debug the example ==
==                             ==
=================================


LPCXpresso IDE
--------------

In the project explorer window of the IDE right click on the
  Example_AudioOutputHost project and select Debug As->C/C++ MCU Application
In the main window of the IDE click Run->Resume to start running the application



Keil uVision 4 IDE
------------------

In the Batch Build window, select the project targets as described in Compiler
  Build Configurations and then click Build
In the main window of the IDE click Debug->Start/Stop Debug Session
click Debug->Run



================================================
==                                            ==
== How this example runs and what to look for ==
==                                            ==
================================================

When the example is first run the terminal window will display:
Audio Output Host Demo running.

When the USB speakers are plugged in the terminal window will display:
Audio Device Enumerated.

When the button is pressed on the board the USB speakers will produce
a loud tone. When the button is not pressed the USB speakers will be silent.

When the USB speakers are unplugged the terminal window will display:
Device Unattached.
