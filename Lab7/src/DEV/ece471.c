/*
 * ece471.c
 *
 *  Created on: Dec 26, 2012
 *      Author: Nathan Zimmerman
 */

#include "ece471.h"
#include "LPC17xx.h"

void delay_ms(unsigned int ms)
{
 	volatile unsigned int j= ms * 10000;
	volatile unsigned int i;
	for(i = 0; i < j; i++)
	{
	}
} // End delay_ms


void led_setup(void)
{
	LPC_GPIO1 -> FIODIR |= (0xFF << 19);
	LPC_GPIO1 -> FIOPIN |= (0xFF << 19);
}

void setup_gpio_interrupt()
{
	LPC_GPIOINT -> IO2IntEnF |= P2_1;
	NVIC_EnableIRQ(EINT3_IRQn);

}

