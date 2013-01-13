=============================
==                         ==
== MassStorageHost example ==
==                         ==
=============================
This example implements a mass storage class host mode device that enumerates
a mass storage class device (USB flash drive). It reads the first sector
of the device and displays a hexdump style listing of that data on a terminal.


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
  repeat these steps with the CDL, LPCUSBlib, and Example_MassStorageHost

Configure MCU:
  right click on the Example_MassStorageHost project in the project explorer window
  click Properties->C/C++ Build->MCU settings->(see configuration in table below)

Configure the indexer:
   click Window->Preferences->C/C++->Indexer->Use active build configuration

                                  BSP             CDL       LPCUSBlib      Example_MassStorageHost    MCU
                        ======================  =======    ==============  =======================  =======
LPC1850  / Hitex        LPC1850_HITEX_A4        LPC18xx    LPC18xx_Host          LPC18xx            LPC1850
LPC4350  / Hitex        LPC18xx_HITEX_A4        LPC43xx_M4 LPC43xx_Host          LPC43xx M4         LPC4350
LPC1768  / MCB1700      LPC1768_MCB1700         LPC17xx    LPC17xx_Host          LPC17xx            LPC1768
LPC1769  / LPCXpresso   LPC17xx_LPCXpressoRevB  LPC17xx    LPC17xx_Host          LPC17xx            LPC1769
 


Keil uVision 4 IDE
------------------

Configure a Batch Build:
  click on Project->Batch Build...
  expand the BSP project to display a list of project targets
  select the project target by checking the appropriate checkbox
    (see target list in table below)
  repeat these steps with the CDL, LPCUSBlib_Device, and Example_MassStorageHost

                        BSP       CDL     LPCUSBlib_Host  Example_MassStorageHost 
                     ==========  =======  ==============  =======================
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
Plug a USB flash drive into the type A receptacle marked X10
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
Plug a USB flash drive into the type A receptacle marked J20
Connect the type-B end of a USB mini cable to the receptacle marked J25 
  (next to the DC connector) and the type-A end of that cable into a PC.
	

LPC17xx_LPCXpressoRevB (LPCXpresso LPC17xx on the LPCXpresso Rev B base board)
------------------------------------------------------------------------------
schematic: ./BSP/schematics/LPCXpressoLPC1769revB.pdf

Plug the LPCXpresso LPC1769 board into the connector labeled J4 on a
  LPCXpresso base board
Connect the JTAG to the mini USB receptacle labeled J3 on the LPCXpresso LPC1769
  board
Configure the base board according to the users guide from Embedded Artists
Install jumpers 1-2, 3-4, 5-6 on the jumper block marked J61 on the base board
Plug a USB flash drive into the type A receptacle marked J60 on the base board
Connect the mini type-B end of a USB cable to the USB-to-serial receptacle on 
  the base board labeled X3 and the type-A end to the USB receptacle on a PC.
Once the USB cable is connected to the board the following LEDs should be
  illuminated:
    LED25  (vbus)
    LED26  (3.3v rail)
		


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
  Example_MassStorageHost project and select Debug As->C/C++ MCU Application
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
Mass Storage Host Demo running.


When a flash drive is plugged in the terminal window will display
  something similar to this:

Device Attached.

Total LUNs: 1 - Using first LUN in device.
Vendor "USB     ", Product "Flash Disk      "
Mass Storage Device Enumerated.

Waiting until ready...

Retrieving Capacity...

1957887 blocks of 512 bytes.

Contents of first block:

33 C0 8E D0 BC 00 7C FB 50 07 50 1F FC BE 1B 7C     3.....|.P.P....|
BF 1B 06 50 57 B9 E5 01 F3 A4 CB BD BE 07 B1 04     ...PW...........
38 6E 00 7C 09 75 13 83 C5 10 E2 F4 CD 18 8B F5     8n.|.u..........
83 C6 10 49 74 19 38 2C 74 F6 A0 B5 07 B4 07 8B     ...It.8,t.......
F0 AC 3C 00 74 FC BB 07 00 B4 0E CD 10 EB F2 88     ..<.t...........
4E 10 E8 46 00 73 2A FE 46 10 80 7E 04 0B 74 0B     N..F.s*.F..~..t.
80 7E 04 0C 74 05 A0 B6 07 75 D2 80 46 02 06 83     .~..t....u..F...
46 08 06 83 56 0A 00 E8 21 00 73 05 A0 B6 07 EB     F...V...!.s.....
BC 81 3E FE 7D 55 AA 74 0B 80 7E 10 00 74 C8 A0     ..>.}U.t..~..t..
B7 07 EB A9 8B FC 1E 57 8B F5 CB BF 05 00 8A 56     .......W.......V
00 B4 08 CD 13 72 23 8A C1 24 3F 98 8A DE 8A FC     .....r#..$?.....
43 F7 E3 8B D1 86 D6 B1 06 D2 EE 42 F7 E2 39 56     C..........B..9V
0A 77 23 72 05 39 46 08 73 1C B8 01 02 BB 00 7C     .w#r.9F.s......|
8B 4E 02 8B 56 00 CD 13 73 51 4F 74 4E 32 E4 8A     .N..V...sQOtN2..
56 00 CD 13 EB E4 8A 56 00 60 BB AA 55 B4 41 CD     V......V.`..U.A.
13 72 36 81 FB 55 AA 75 30 F6 C1 01 74 2B 61 60     .r6..U.u0...t+a`
6A 00 6A 00 FF 76 0A FF 76 08 6A 00 68 00 7C 6A     j.j..v..v.j.h.|j
01 6A 10 B4 42 8B F4 CD 13 61 61 73 0E 4F 74 0B     .j..B....aas.Ot.
32 E4 8A 56 00 CD 13 EB D6 61 F9 C3 49 6E 76 61     2..V.....a..Inva
6C 69 64 20 70 61 72 74 69 74 69 6F 6E 20 74 61     lid partition ta
62 6C 65 00 45 72 72 6F 72 20 6C 6F 61 64 69 6E     ble.Error loadin
67 20 6F 70 65 72 61 74 69 6E 67 20 73 79 73 74     g operating syst
65 6D 00 4D 69 73 73 69 6E 67 20 6F 70 65 72 61     em.Missing opera
74 69 6E 67 20 73 79 73 74 65 6D 00 00 00 00 00     ting system.....
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     ................
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     ................
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     ................
00 00 00 00 00 2C 44 63 18 2E 07 C3 00 00 80 01     .....,Dc........
01 00 06 20 00 77 00 02 00 00 00 DE 1D 00 00 00     ... .w..........
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     ................
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00     ................
00 00 00 00 00 00 00 00 00 00 00 00 00 00 55 AA     ..............U.


When the flash drive is unplugged the ternimal window will display:
Device Unattached.
