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
 *  Header file for KeyboardHost.c.
 */

#ifndef _KEYBOARD_HOST_H_
#define _KEYBOARD_HOST_H_

	/* Includes: */
		#include "bsp.h"
		#include "USB.h"


	/* Macros: */
		/** LED mask for the library LED driver, to indicate that the USB interface is not ready. */
		#define LEDMASK_USB_NOTREADY      LEDS_LED1

		/** LED mask for the library LED driver, to indicate that the USB interface is enumerating. */
		#define LEDMASK_USB_ENUMERATING  (LEDS_LED2 | LEDS_LED3)

		/** LED mask for the library LED driver, to indicate that the USB interface is ready. */
		#define LEDMASK_USB_READY        (LEDS_LED2 | LEDS_LED4)

		/** LED mask for the library LED driver, to indicate that an error has occurred in the USB interface. */
		#define LEDMASK_USB_ERROR        (LEDS_LED1 | LEDS_LED3)

	/* Function Prototypes: */

		char keyboard(void);

		void SetupHardwareKeyboard(void);
		char KeyboardHost_Task(void);

		void EVENT_USB_Host_HostError(const uint8_t corenum, const uint8_t ErrorCode);
		void EVENT_USB_Host_DeviceAttached(const uint8_t corenum);
		void EVENT_USB_Host_DeviceUnattached(const uint8_t corenum);
		void EVENT_USB_Host_DeviceEnumerationFailed(const uint8_t corenum,
													const uint8_t ErrorCode,
		                                            const uint8_t SubErrorCode);
		void EVENT_USB_Host_DeviceEnumerationComplete(const uint8_t corenum);

#endif

