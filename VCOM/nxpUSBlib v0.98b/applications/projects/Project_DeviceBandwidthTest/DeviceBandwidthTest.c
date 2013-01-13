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
 *  Main source file for the DeviceBandwidthTest demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */
#ifndef USB_BANDWIDTH_DIAGNOSTICS
#error USB_BANDWIDTH_DIAGNOSTICS must be defined in this application and in the nxpUSBlib library
#endif


#include "DeviceBandwidthTest.h"
uint32_t SystickRates_idx = 0, SystickRates_us[] = {100, 1000, 10000};
uint32_t SysTickCounter = -1;
uint32_t USBForegroundTaskTime, USBForegroundTaskCount;
uint32_t USBForegroundRate_us = 1000;
uint32_t SystickMSCounter = 0;
extern volatile uint32_t usb_data_buffer_IN_index;

void USBForegroundTask(void)
{
#if !defined(USB_DEVICE_ROM_DRIVER)
	uint32_t USBForegroundTaskStartTime = GetTimerCounter();

	// Handle OUT packers
	Endpoint_SelectEndpoint(VENDOR_SPECIFIC_OUT_EPNUM);
	if (Endpoint_BytesInEndpoint() == USB_DATA_BUFFER_TEM_LENGTH)
		Endpoint_ClearOUT();

	// Handle IN packets
	Endpoint_SelectEndpoint(VENDOR_SPECIFIC_IN_EPNUM);
	if (Endpoint_IsINReady())
	{
		usb_data_buffer_IN_index = 512;
		Endpoint_ClearIN();
	}

	USB_USBTask();
	USBForegroundTaskTime += (GetTimerCounter() - USBForegroundTaskStartTime);
	USBForegroundTaskCount++;
#endif
}
void SysTick_Handler()
{
	SysTickCounter++;
	USBForegroundTask();
	//USB_USBTask();
}

/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
extern uint32_t USBIrqTime, USBIrqCount;
volatile uint32_t cntr;
int main(void)
{

	// wait here to give the debugger a chance to burn the flash
	{
		volatile int i = 0x400000;
		while(i--);
	}

	SetupHardware();

	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
	sei();
	#if defined(USB_DEVICE_ROM_DRIVER)
#if (defined (__LPC18XX__)||defined(__LPC43XX__))
	NVIC_SetPriority(USB0_IRQn, 0x04);
	NVIC_SetPriorityGrouping(5);
#endif

	#endif

	// Process USB foreground activity periodically
	SysTick_Config(SystemCoreClock / (1000000 / USBForegroundRate_us));

	for (;;)
	{
		static uint32_t ForegroundCounter, ForegroundPreviousCounter, PreviousCounter;
		static uint32_t ForegroundTaskTime;

		// Time taken since the last time through this loop at this spot
		ForegroundTaskTime += (ForegroundCounter - ForegroundPreviousCounter);

		// Maintain a rolling timer loop counter for the main loop
		ForegroundPreviousCounter = ForegroundCounter;
		ForegroundCounter = GetTimerCounter();


		// Only display load values every second
		if ((SysTickCounter - PreviousCounter) > (1000000 / SystickRates_us[SystickRates_idx]))
		{
			int USBForegroundLoadx100, USBIrqLoadx100, Remainingx100, USBForegroundCycleTimeMSx100;
			static int SecondCounter = 0;

			if (SecondCounter++ == 4)
			{
				SecondCounter = 0;
				SystickRates_idx++;
				if (SystickRates_idx == (sizeof(SystickRates_us)/sizeof(SystickRates_us[0])))
					SystickRates_idx = 0;
			}
			PreviousCounter = SysTickCounter;


			// Calculate load percentages

			ForegroundTaskTime -= (USBForegroundTaskTime + USBIrqTime); // Subtract the interrupt times from the main loop time
			USBForegroundLoadx100 = USBForegroundTaskTime * 100 / (ForegroundTaskTime/100);
			USBIrqLoadx100 =  USBIrqTime * 100 / (ForegroundTaskTime/100);
			Remainingx100 = 10000 - (USBForegroundLoadx100 + USBIrqLoadx100);
			USBForegroundCycleTimeMSx100 = SystickRates_us[SystickRates_idx] / 10; // Rate at which the USB foreground task is called

			printf("Foreground rate %2d.%02d ms   Foreground load %2d.%02d   Irq load %2d.%02d   Remaining %3d.%02d \n",
					USBForegroundCycleTimeMSx100 / 100,
					USBForegroundCycleTimeMSx100 - USBForegroundCycleTimeMSx100 / 100 * 100,
					USBForegroundLoadx100 / 100,
					USBForegroundLoadx100 - USBForegroundLoadx100 / 100 * 100,
					USBIrqLoadx100 / 100,
					USBIrqLoadx100 - USBIrqLoadx100 / 100 * 100,
					Remainingx100 / 100,
					Remainingx100 - Remainingx100 / 100 * 100);

			// Clear the accumulated task times for the next time through
			USBForegroundTaskTime = USBIrqTime = ForegroundTaskTime = USBForegroundTaskCount = USBIrqCount = 0;

			// Change the rate at which the foreground task is called
			SysTick_Config(SystemCoreClock / (1000000 / SystickRates_us[SystickRates_idx]));
		}
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
#if defined (__CC_ARM)  || defined(__ICCARM__) // FIXME KEIL related
    SystemInit();
#endif
	bsp_init();
#if defined(__LPC11UXX__) || defined(__LPC13UXX__) // TODO PDL lib for LPC1100 is not fully supported
	InitTimer(1);
#endif
	LEDs_Init();
	USB_Init();
#if defined(USB_DEVICE_ROM_DRIVER)
	UsbdMsc_Init();
	//USBD_API->hw->ForceFullSpeed(UsbHandle,1);
#endif
	USB_Connect();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	// Configure the IN endpoint
	if (!(Endpoint_ConfigureEndpoint(VENDOR_SPECIFIC_IN_EPNUM,
									 EP_TYPE_BULK,
									 ENDPOINT_DIR_IN,
									 VENDOR_SPECIFIC_IO_EPSIZE,
									 ENDPOINT_BANK_SINGLE)))
	{
		ConfigSuccess = false;
		return;
	}

	// Configure the OUT endpoint
	if (!(Endpoint_ConfigureEndpoint(VENDOR_SPECIFIC_OUT_EPNUM,
									 EP_TYPE_BULK,
									 ENDPOINT_DIR_OUT,
									 VENDOR_SPECIFIC_IO_EPSIZE,
									 ENDPOINT_BANK_SINGLE)))
	{
		ConfigSuccess = false;
		return;
	}
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
}

