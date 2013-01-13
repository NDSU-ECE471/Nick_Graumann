/*
* Copyright(C) NXP Semiconductors, 2012
* All rights reserved.
*
* Software that is described herein is for illustrative purposes only
* which provides customers with programming information regarding the
* LPC products.  This software is supplied "AS IS" without any warranties of
* any kind, and NXP Semiconductors and its licensor disclaim any and 
* all warranties, express or implied, including all implied warranties of 
* merchantability, fitness for a particular purpose and non-infringement of 
* intellectual property rights.  NXP Semiconductors assumes no responsibility
* or liability for the use of the software, conveys no license or rights under any
* patent, copyright, mask work right, or any other intellectual property rights in 
* or to any products. NXP Semiconductors reserves the right to make changes
* in the software without notification. NXP Semiconductors also makes no 
* representation or warranty that such application will be suitable for the
* specified use without further testing or modification.
* 
* Permission to use, copy, modify, and distribute this software and its 
* documentation is hereby granted, under NXP Semiconductors' and its 
* licensor's relevant copyrights in the software, without fee, provided that it 
* is used in conjunction with NXP Semiconductors microcontrollers.  This 
* copyright, permission, and disclaimer notice must appear in all copies of 
* this code.
*/


/** \file
 *
 *  Main source file for the KeyboardHost demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "KeyboardHost.h"
//#include "USBController_LPC.h"


/** nxpUSBlib HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Host_t Keyboard_Host_HID_Interface =
	{
		.Config =
			{
				.DataINPipeNumber       = 1,
				.DataINPipeDoubleBank   = false,

				.DataOUTPipeNumber      = 2,
				.DataOUTPipeDoubleBank  = false,

				.HIDInterfaceProtocol   = HID_CSCP_KeyboardBootProtocol,
			},
	};


/** Main program entry point. This routine configures the hardware required by the application, then
 *  enters a loop to run the application tasks in sequence.
 */
char keyboard(void)
{
	char tecla;
	SetupHardwareKeyboard();

	do
	{
		tecla = KeyboardHost_Task();

		HID_Host_USBTask(&Keyboard_Host_HID_Interface);
		USB_USBTask();
	} while (tecla != 'D');
	return tecla;
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardwareKeyboard(void)
{
	//bsp_init();

	Serial_Init(9600, false);
	//LEDs_Init();
	USB_Disable();
	USB_CurrentMode = USB_MODE_Host;
	USB_Init();

	/* Create a stdio stream for the serial port for stdin and stdout */
	Serial_CreateStream(NULL);
}

/** Task to manage an enumerated USB keyboard once connected, to display key state
 *  data as it is received.
 */
char KeyboardHost_Task(void)
{
	if (USB_HostState[Keyboard_Host_HID_Interface.Config.PortNumber] != HOST_STATE_Configured)
	  return 0;

	if (HID_Host_IsReportReceived(&Keyboard_Host_HID_Interface))
	{
		USB_KeyboardReport_Data_t KeyboardReport;
		HID_Host_ReceiveReport(&Keyboard_Host_HID_Interface, &KeyboardReport);

		LEDs_ChangeLEDs(LEDS_LED1, (KeyboardReport.Modifier) ? LEDS_LED1 : 0);

		uint8_t KeyCode = KeyboardReport.KeyCode[0];

		if (KeyCode)
		{
			char PressedKey = 0;

			LEDs_ToggleLEDs(LEDS_LED2);

			/* Retrieve pressed key character if alphanumeric */
			if ((KeyCode >= HID_KEYBOARD_SC_A) && (KeyCode <= HID_KEYBOARD_SC_Z))
			{
				PressedKey = (KeyCode - HID_KEYBOARD_SC_A) + 'A';
			}
			else if ((KeyCode >= HID_KEYBOARD_SC_1_AND_EXCLAMATION) &
			         (KeyCode  < HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS))
			{
				PressedKey = (KeyCode - HID_KEYBOARD_SC_1_AND_EXCLAMATION) + '1';
			}
			else if (KeyCode == HID_KEYBOARD_SC_0_AND_CLOSING_PARENTHESIS)
			{
				PressedKey = '0';
			}
			else if (KeyCode == HID_KEYBOARD_SC_SPACE)
			{
				PressedKey = ' ';
			}
			else if (KeyCode == HID_KEYBOARD_SC_ENTER)
			{
				PressedKey = '\n';
			}
			
			if (PressedKey)
			  putchar(PressedKey);
			return PressedKey;
		}
		return 0;
	}
}

/** Event handler for the USB_DeviceAttached event. This indicates that a device has been attached to the host, and
 *  starts the library USB task to begin the enumeration and USB management process.
 */
void EVENT_USB_Host_DeviceAttached(const uint8_t corenum)
{
	printf_P(PSTR("Device Attached on port %d\r\n"),corenum);
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the USB_DeviceUnattached event. This indicates that a device has been removed from the host, and
 *  stops the library USB task management process.
 */
void EVENT_USB_Host_DeviceUnattached(const uint8_t corenum)
{
	printf_P(PSTR("\r\nDevice Unattached on port %d\r\n"),corenum);
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the USB_DeviceEnumerationComplete event. This indicates that a device has been successfully
 *  enumerated by the host and is now ready to be used by the application.
 */
void EVENT_USB_Host_DeviceEnumerationComplete(const uint8_t corenum)
{
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);

	uint16_t ConfigDescriptorSize;
	uint8_t  ConfigDescriptorData[512];

	if (USB_Host_GetDeviceConfigDescriptor(corenum, 1, &ConfigDescriptorSize, ConfigDescriptorData,
	                                       sizeof(ConfigDescriptorData)) != HOST_GETCONFIG_Successful)
	{
		puts_P(PSTR("Error Retrieving Configuration Descriptor.\r\n"));
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		return;
	}
	
	Keyboard_Host_HID_Interface.Config.PortNumber = corenum;

	if (HID_Host_ConfigurePipes(&Keyboard_Host_HID_Interface,
	                            ConfigDescriptorSize, ConfigDescriptorData) != HID_ENUMERROR_NoError)
	{
		puts_P(PSTR("Attached Device Not a Valid Keyboard.\r\n"));
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		return;
	}

	if (USB_Host_SetDeviceConfiguration(corenum, 1) != HOST_SENDCONTROL_Successful)
	{
		puts_P(PSTR("Error Setting Device Configuration.\r\n"));
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		return;
	}

	if (HID_Host_SetBootProtocol(&Keyboard_Host_HID_Interface) != 0)
	{
		puts_P(PSTR("Could not Set Boot Protocol Mode.\r\n"));
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		USB_Host_SetDeviceConfiguration(corenum, 0);
		return;
	}

	puts_P(PSTR("Keyboard Enumerated.\r\n"));
	LEDs_SetAllLEDs(LEDMASK_USB_READY);
}

/** Event handler for the USB_HostError event. This indicates that a hardware error occurred while in host mode. */
void EVENT_USB_Host_HostError(const uint8_t corenum, const uint8_t ErrorCode)
{
	USB_Disable();

	printf_P(PSTR(ESC_FG_RED "Host Mode Error\r\n"
							 " -- Error port %d\r\n"
	                         " -- Error Code %d\r\n" ESC_FG_WHITE), corenum, ErrorCode);

	LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
	for(;;);
}

/** Event handler for the USB_DeviceEnumerationFailed event. This indicates that a problem occurred while
 *  enumerating an attached USB device.
 */
void EVENT_USB_Host_DeviceEnumerationFailed(const uint8_t corenum, const uint8_t ErrorCode,
                                            const uint8_t SubErrorCode)
{
	/*
	printf_P(PSTR(ESC_FG_RED "Dev Enum Error\r\n"
	                         " -- Error Code %d\r\n"
	                         " -- Sub Error Code %d\r\n"
	                         " -- In State %d\r\n" ESC_FG_WHITE), ErrorCode, SubErrorCode, USB_HostState);
	*/
	LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
}

/** Dummy callback function for HID Parser
 */
bool CALLBACK_HIDParser_FilterHIDReportItem(HID_ReportItem_t* const CurrentItem){
}
