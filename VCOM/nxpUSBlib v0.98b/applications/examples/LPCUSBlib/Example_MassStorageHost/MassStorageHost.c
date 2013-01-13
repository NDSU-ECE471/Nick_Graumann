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
 *  Main source file for the MassStorageHost demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "MassStorageHost.h"

/** LPCUSBlib Mass Storage Class driver interface configuration and state information. This structure is
 *  passed to all Mass Storage Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_MS_Host_t FlashDisk_MS_Interface =
	{
		.Config =
			{
				.DataINPipeNumber       = 1,
				.DataINPipeDoubleBank   = false,

				.DataOUTPipeNumber      = 2,
				.DataOUTPipeDoubleBank  = false,
			},
	};


/** Main program entry point. This routine configures the hardware required by the application, then
 *  enters a loop to run the application tasks in sequence.
 */
int main(void)
{
	SetupHardware();

	puts_P(PSTR("Mass Storage Host Demo running.\r\n"));

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	sei();

	for (;;)
	{
		MassStorageHost_Task();
		
		MS_Host_USBTask(&FlashDisk_MS_Interface);
		USB_USBTask();
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
	bsp_init();

	/* Hardware Initialization */
	Serial_Init(9600, false);
	LEDs_Init();
	USB_Init();

	/* Create a stdio stream for the serial port for stdin and stdout */
	Serial_CreateStream(NULL);
}

/** Task to manage an enumerated USB Mass Storage device once connected, to print out
 *  data from the device.
 */
void MassStorageHost_Task(void)
{
	if (USB_HostState[FlashDisk_MS_Interface.Config.PortNumber] != HOST_STATE_Configured)
	  return;

	LEDs_SetAllLEDs(LEDMASK_USB_BUSY);

	puts_P(PSTR("Waiting until ready...\r\n"));

	for (;;)
	{
		uint8_t ErrorCode = MS_Host_TestUnitReady(&FlashDisk_MS_Interface, 0);

		if (!(ErrorCode))
		  break;

		/* Check if an error other than a logical command error (device busy) received */
		if (ErrorCode != MS_ERROR_LOGICAL_CMD_FAILED)
		{
			puts_P(PSTR("Error waiting for device to be ready.\r\n"));
			LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
			USB_Host_SetDeviceConfiguration(FlashDisk_MS_Interface.Config.PortNumber,0);
			return;
		}
	}

	puts_P(PSTR("Retrieving Capacity...\r\n"));

	SCSI_Capacity_t DiskCapacity;
	if (MS_Host_ReadDeviceCapacity(&FlashDisk_MS_Interface, 0, &DiskCapacity))
	{
		puts_P(PSTR("Error retrieving device capacity.\r\n"));
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		USB_Host_SetDeviceConfiguration(FlashDisk_MS_Interface.Config.PortNumber,0);
		return;
	}

	printf_P(PSTR("%lu blocks of %lu bytes.\r\n"), DiskCapacity.Blocks, DiskCapacity.BlockSize);

	//uint8_t BlockBuffer[DiskCapacity.BlockSize];
	uint8_t BlockBuffer[512];

	if (MS_Host_ReadDeviceBlocks(&FlashDisk_MS_Interface, 0, 0x00000000, 1, DiskCapacity.BlockSize, BlockBuffer))
	{
		puts_P(PSTR("Error reading device block.\r\n"));
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		USB_Host_SetDeviceConfiguration(FlashDisk_MS_Interface.Config.PortNumber,0);
		return;
	}

	puts_P(PSTR("\r\nContents of first block:\r\n"));

	for (uint16_t Chunk = 0; Chunk < (DiskCapacity.BlockSize >> 4); Chunk++)
	{
		uint8_t* ChunkPtr = &BlockBuffer[Chunk << 4];

		/* Print out the 16 bytes of the chunk in HEX format */
		for (uint8_t ByteOffset = 0; ByteOffset < (1 << 4); ByteOffset++)
		{
			char CurrByte = *(ChunkPtr + ByteOffset);
			printf_P(PSTR("%.2X "), CurrByte);
		}

		printf_P(PSTR("    "));

		/* Print out the 16 bytes of the chunk in ASCII format */
		for (uint8_t ByteOffset = 0; ByteOffset < (1 << 4); ByteOffset++)
		{
			char CurrByte = *(ChunkPtr + ByteOffset);
			putchar(isprint(CurrByte) ? CurrByte : '.');
		}

		printf_P(PSTR("\r\n"));
	}

	LEDs_SetAllLEDs(LEDMASK_USB_READY);
	USB_Host_SetDeviceConfiguration(FlashDisk_MS_Interface.Config.PortNumber,0);
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

	FlashDisk_MS_Interface.Config.PortNumber = corenum;

	if (MS_Host_ConfigurePipes(&FlashDisk_MS_Interface,
	                           ConfigDescriptorSize, ConfigDescriptorData) != MS_ENUMERROR_NoError)
	{
		puts_P(PSTR("Attached Device Not a Valid Mass Storage Device.\r\n"));
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		return;
	}

	if (USB_Host_SetDeviceConfiguration(FlashDisk_MS_Interface.Config.PortNumber,1) != HOST_SENDCONTROL_Successful)
	{
		puts_P(PSTR("Error Setting Device Configuration.\r\n"));
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		return;
	}

	uint8_t MaxLUNIndex;
	if (MS_Host_GetMaxLUN(&FlashDisk_MS_Interface, &MaxLUNIndex))
	{
		puts_P(PSTR("Error retrieving max LUN index.\r\n"));
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		USB_Host_SetDeviceConfiguration(FlashDisk_MS_Interface.Config.PortNumber,0);
		return;
	}

	printf_P(PSTR("Total LUNs: %d - Using first LUN in device.\r\n"), (MaxLUNIndex + 1));

	if (MS_Host_ResetMSInterface(&FlashDisk_MS_Interface))
	{
		puts_P(PSTR("Error resetting Mass Storage interface.\r\n"));
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		USB_Host_SetDeviceConfiguration(FlashDisk_MS_Interface.Config.PortNumber,0);
		return;
	}

	SCSI_Request_Sense_Response_t SenseData;
	if (MS_Host_RequestSense(&FlashDisk_MS_Interface, 0, &SenseData) != 0)
	{
		puts_P(PSTR("Error retrieving device sense.\r\n"));
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		USB_Host_SetDeviceConfiguration(FlashDisk_MS_Interface.Config.PortNumber,0);
		return;
	}

	if (MS_Host_PreventAllowMediumRemoval(&FlashDisk_MS_Interface, 0, true))
	{
		puts_P(PSTR("Error setting Prevent Device Removal bit.\r\n"));
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		USB_Host_SetDeviceConfiguration(FlashDisk_MS_Interface.Config.PortNumber,0);
		return;
	}

	SCSI_Inquiry_Response_t InquiryData;
	if (MS_Host_GetInquiryData(&FlashDisk_MS_Interface, 0, &InquiryData))
	{
		puts_P(PSTR("Error retrieving device Inquiry data.\r\n"));
		LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
		USB_Host_SetDeviceConfiguration(FlashDisk_MS_Interface.Config.PortNumber,0);
		return;
	}

	printf_P(PSTR("Vendor \"%.8s\", Product \"%.16s\"\r\n"), InquiryData.VendorID, InquiryData.ProductID);

	puts_P(PSTR("Mass Storage Device Enumerated.\r\n"));
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
void EVENT_USB_Host_DeviceEnumerationFailed(const uint8_t corenum,
											const uint8_t ErrorCode,
                                            const uint8_t SubErrorCode)
{
	printf_P(PSTR(ESC_FG_RED "Dev Enum Error\r\n"
							 " -- Error port %d\r\n"
	                         " -- Error Code %d\r\n"
	                         " -- Sub Error Code %d\r\n"
	                         " -- In State %d\r\n" ESC_FG_WHITE),
	                         corenum, ErrorCode, SubErrorCode, USB_HostState[corenum]);

	LEDs_SetAllLEDs(LEDMASK_USB_ERROR);
}
