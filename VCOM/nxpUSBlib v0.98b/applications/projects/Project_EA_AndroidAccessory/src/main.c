/*****************************************************************************
 *
 *   Copyright(C) 2011, Embedded Artists AB
 *   All rights reserved.
 *
 ******************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * Embedded Artists AB assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. Embedded Artists AB
 * reserves the right to make changes in the software without
 * notification. Embedded Artists AB also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc_types.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_gpio.h"

#include "rgb.h"
#include "temp.h"
#include "joystick.h"

#include "USB.h"
#include "AndroidAccessoryHost.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

#define DEBUG_UART_PORT	LPC_UART3

uint32_t getMsTicks(void);

/******************************************************************************
 * Local variables
 *****************************************************************************/

static uint32_t msTicks = 0;

/******************************************************************************
 * Local functions
 *****************************************************************************/

static void serial_init(const uint32_t BaudRate, const bool DoubleSpeed)
{
	UART_CFG_Type UARTConfigStruct;
	PINSEL_CFG_Type PinCfg;

	PinCfg.Funcnum = 2;
	PinCfg.OpenDrain = 0;
	PinCfg.Pinmode = 0;
	PinCfg.Pinnum = 0;
	PinCfg.Portnum = 0;
	PINSEL_ConfigPin(&PinCfg);
	PinCfg.Pinnum = 1;
	PINSEL_ConfigPin(&PinCfg);

	/* Initialize UART Configuration parameter structure to default state:
	 * Baudrate = 9600bps
	 * 8 data bit
	 * 1 Stop bit
	 * None parity
	 */
	UART_ConfigStructInit(&UARTConfigStruct);
	// Re-configure baudrate
	UARTConfigStruct.Baud_rate = BaudRate;

	// Initialize DEBUG_UART_PORT peripheral with given to corresponding parameter
	UART_Init(DEBUG_UART_PORT, &UARTConfigStruct);

	// Enable UART Transmit
	UART_TxCmd(DEBUG_UART_PORT, ENABLE);
}

static void setup_hardware(void)
{
	// Hardware Initialization

	SysTick_Config(SystemCoreClock / 1000);

	serial_init(115200, false);
	USB_Init();

	// USB PWR
	LPC_PINCON->PINSEL3 &= ~0xC0; // PINSEL3 function 00 for pin 1.19
	GPIO_SetDir(1, 1 << 19, 1);
	GPIO_ClearValue(1, 1 << 19);

	// RGB LED
	rgb_init();
	rgb_setLeds(0);

	// Temperature control
	temp_init (&getMsTicks);

	// Joystick
	joystick_init();
}


/******************************************************************************
 * Main function
 *****************************************************************************/

int main(void)
{
	setup_hardware();

	printf("AOA Demo Running\r\n");

	for (;;)
	{
		Monitor_Task();
		AndroidHost_Task();
		USB_USBTask();
	}
}

/******************************************************************************
 * Public function
 *****************************************************************************/

void SysTick_Handler(void) {
    msTicks++;
}

uint32_t getMsTicks(void)
{
    return msTicks;
}


///////////////////////////////////////////////////////////////////////////////////////////////
// Function __write() / __sys_write
//
// Called by bottom level of printf routine within RedLib C library to write
// a character. With the default semihosting stub, this would write the character
// to the debugger console window . But this version writes
// the character to the LPC1768/RDB1768 UART.
int __sys_write (int iFileHandle, char *pcBuffer, int iLength)
{
	UART_Send(DEBUG_UART_PORT, (uint8_t*) pcBuffer, iLength, BLOCKING);
	return iLength;
}

// Function __readc() / __sys_readc
//
// Called by bottom level of scanf routine within RedLib C library to read
// a character. With the default semihosting stub, this would read the character
// from the debugger console window (which acts as stdin). But this version reads
// the character from the LPC1768/RDB1768 UART.
int __sys_readc (void)
{
	/*-- UARTGetChar --*/
	uint8_t tmp = 0;
	UART_Receive(DEBUG_UART_PORT, &tmp, 1, BLOCKING);
	return (int) tmp;
}

