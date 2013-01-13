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


#define  __INCLUDE_FROM_USB_DRIVER
#include "..\Mouse.h"

uint32_t CALLBACK_UsbdRom_Register_DeviceDescriptor(void);
uint32_t CALLBACK_UsbdRom_Register_ConfigurationDescriptor(void);
uint32_t CALLBACK_UsbdRom_Register_StringDescriptor(void);
uint32_t CALLBACK_UsbdRom_Register_DeviceQualifierDescriptor(void);
uint8_t CALLBACK_UsbdRom_Register_ConfigureEndpoint(void);
uint32_t CALLBACK_UsbdHid_Register_InterfaceDescriptor(void);
uint32_t CALLBACK_UsbdHid_Register_ReportDescriptor(uint8_t **dest);
uint32_t CALLBACK_UsbdHid_Register_ReportInBuffer(uint8_t **dest);
void CALLBACK_UsbdHid_SetReport(uint8_t **reportoutbuffer, uint32_t reportoutsize);
void CALLBACK_UsbdHid_SetReportChange(bool newstate);
bool CALLBACK_UsbdHid_IsReportChanged(void);

