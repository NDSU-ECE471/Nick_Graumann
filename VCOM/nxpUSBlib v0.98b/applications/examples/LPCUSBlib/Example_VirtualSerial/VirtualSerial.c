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
 *  Main source file for the VirtualSerial demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "VirtualSerial.h"

/** LPCUSBlib CDC Class driver interface configuration and state information. This structure is
 *  passed to all CDC Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface =
	{
		.Config =
			{
				.ControlInterfaceNumber         = 0,

				.DataINEndpointNumber           = CDC_TX_EPNUM,
				.DataINEndpointSize             = CDC_TXRX_EPSIZE,
				.DataINEndpointDoubleBank       = false,

				.DataOUTEndpointNumber          = CDC_RX_EPNUM,
				.DataOUTEndpointSize            = CDC_TXRX_EPSIZE,
				.DataOUTEndpointDoubleBank      = false,

				.NotificationEndpointNumber     = CDC_NOTIFICATION_EPNUM,
				.NotificationEndpointSize       = CDC_NOTIFICATION_EPSIZE,
				.NotificationEndpointDoubleBank = false,
			},
	};

/** Standard file stream for the CDC interface when set up, so that the virtual CDC COM port can be
 *  used like any regular character stream in the C APIs
 */
//static FILE USBSerialStream;

const char* const DEBUG_CONSOLE_WELCOME_MSG = "USB Serial Console Demo - UART Debug Console\r\n";
const char* const DEBUG_CONSOLE_USB_CONNECT_MSG = "USB VCOM device connected\r\n";
const char* const DEBUG_CONSOLE_USB_DISCONNECT_MSG = "USB VCOM device disconnected\r\n";

typedef enum
{
	STATUS_NONE,
	STATUS_CONNECTED,
	STATUS_DISCONNECTED
} DeviceStatus_T;

DeviceStatus_T DeviceStatus = STATUS_NONE;

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
	SetupHardware();
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	sei();

	Serial_Send((uint8_t*)DEBUG_CONSOLE_WELCOME_MSG, strlen(DEBUG_CONSOLE_WELCOME_MSG), BLOCKING);

	for (;;)
	{
		HandleDeviceStatus();

		EchoCharater();

		CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	bsp_init();
	Serial_Init(9600, false);
	USB_Init();

	USB_Connect();
}

void HandleDeviceStatus(void)
{
	if(DeviceStatus == STATUS_CONNECTED)
	{
		Serial_Send((uint8_t*)DEBUG_CONSOLE_USB_CONNECT_MSG, strlen(DEBUG_CONSOLE_USB_CONNECT_MSG), BLOCKING);
		DeviceStatus = STATUS_NONE;
	}
	else if(DeviceStatus == STATUS_DISCONNECTED)
	{
		Serial_Send((uint8_t*)DEBUG_CONSOLE_USB_DISCONNECT_MSG, strlen(DEBUG_CONSOLE_USB_DISCONNECT_MSG), BLOCKING);
		DeviceStatus = STATUS_NONE;
	}
}

void EchoCharater(void)
{
	/* Echo back character */
	uint8_t recv_byte[CDC_TXRX_EPSIZE];

	if(CDC_Device_BytesReceived(&VirtualSerial_CDC_Interface))
	{
		recv_byte[0] = CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
		CDC_Device_SendData(&VirtualSerial_CDC_Interface, (char *)recv_byte, 1);
	}
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	// Todo: Not sure why this event never triggers
	DeviceStatus = STATUS_CONNECTED;
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	DeviceStatus = STATUS_DISCONNECTED;
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
}

void EVENT_CDC_Device_LineEncodingChanged(USB_ClassInfo_CDC_Device_t* const CDCInterfaceInfo)
{
	/*TODO: add LineEncoding processing here
	 * this is just a simple statement, only Baud rate is set */
	Serial_Init(CDCInterfaceInfo->State.LineEncoding.BaudRateBPS, false);
}
