====================================
==                                ==
== Android Open Accessory example ==
==                                ==
====================================
This example implements the Android Open Accessory protocol which 
allows external USB hardware to interact with an Android-powered device
such as a Phone or Tablet. In this case the USB hardware is an LPCXpresso
Base Board with an LPC1769 LPCXpresso Board.



=====================
==                 ==
== Getting Started ==
==                 ==
=====================

This example can be built and debugged using the LPCXpresso tool chain.
Visit http://www.lpcware.com/content/project/LPCUSBlib/build-it to learn
how to setup and configure LPCXpresso to build this example.


===================================
==                               ==
== Compiler build configurations ==
==                               ==
===================================

The different libraries in the LPCUSBlib package are designed to run on many 
different development boards. Therefore the compiler must be configured to build 
an image that is targeted to the development board you are using. This section 
explains how to set these build configurations for each of the supported compilers.


LPCXpresso IDE
--------------

Configure projects:
  right click on the BSP project in the project explorer window
  click Build Configurations->Set Active->(see configuration in table below)
  repeat these steps with the CDL, LPCUSBlib, and Project_EA_AndroidAccessory

Configure MCU:
  right click on the Example_MassStorageHost project in the project explorer window
  click Properties->C/C++ Build->MCU settings->(see configuration in table below)

Configure the indexer:
   click Window->Preferences->C/C++->Indexer->Use active build configuration

                                  BSP             CDL     LPCUSBlib    Project_EA_AndroidAccessory    MCU
                        ======================  =======  ============  ===========================  =======
LPC1769  / LPCXpresso   LPC17xx_LPCXpressoRevB  LPC17xx  LPC17xx_Host         LPC17xx               LPC1769
 



==========================================
==                                      ==
== Board connections and configurations ==
==                                      ==
==========================================


LPC17xx_LPCXpressoRevB (LPCXpresso LPC17xx on the LPCXpresso Rev B base board)
------------------------------------------------------------------------------
schematic: ./BSP/schematics/LPCXpressoLPC1769revB.pdf

Plug the LPCXpresso LPC1769 board into the connector labeled J4 on a
  LPCXpresso base board
Connect the mini type-B end of a USB cable to the mini USB receptacle labeled J3 
  on the LPCXpresso LPC1769 board. Connect the type-A to the USB receptacle on 
  a PC.
Configure the base board according to the users guide from Embedded Artists
Install jumpers 1-2, 3-4, 5-6 on the jumper block marked J61 on the base board.
Connect the mini type-B end of a USB cable to the USB-to-serial receptacle on 
  the base board labeled X3 and the type-A end to the USB receptacle on a PC.

		

======================
==                  ==
== PC configuration ==
==                  ==
======================


Use a PC running Windows XP, Vista or 7 to display runtime diagnostics
that can be viewed with a terminal emulation program like Teraterm or
Hyperterm. The baud rate in these programs should be set to 115200.



=================================
==                             ==
== Build and debug the example ==
==                             ==
=================================


LPCXpresso IDE
--------------

In the project explorer window of the IDE right click on the
  Project_EA_AndroidAccessory project and select Debug As->C/C++ MCU Application
In the main window of the IDE click Run->Resume to start running the application



================================================
==                                            ==
== More information about how to run the demo ==
==                                            ==
================================================

For more details about how to run this demo download the "Getting Started 
Manual" from Embedded Artists website. The manual is available on the support
page for the LPCXpresso Base Board. Access to this support page is given when
registering the serial number for the LPCXpresso Base Board. The serial number
is printed on a flyer put in the same box as the board.
