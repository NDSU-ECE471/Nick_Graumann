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
//#include "error.h"
//#include "usbd.h"
#include "adc_def.h"

#ifndef __ADC_USER_H__
#define __ADC_USER_H__

/* exported routines */
extern void UsbdAdc_Init(void);
extern void UsbdAdc_start_xfr(void);
extern void UsbdAdc_stop_xfr(void);
extern ErrorCode_t USB_Interface_Event (USBD_HANDLE_T hUsb);
extern ErrorCode_t UsbdAdc_ISO_OUT_Hdlr (USBD_HANDLE_T hUsb, void* data, uint32_t event);
extern ErrorCode_t UsbdAdc_ep0_hdlr(USBD_HANDLE_T hUsb, void* data, uint32_t event);

#endif  /* __ADC_USER_H__ */
