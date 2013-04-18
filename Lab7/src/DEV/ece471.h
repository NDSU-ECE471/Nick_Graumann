/*
 * ece471.h
 *
 *  Created on: Dec 26, 2012
 *      Author: Nathan Zimmerman
 */

#ifndef ECE471_H_
#define ECE471_H_

#include "ports.h"

#define		LED_PORT	0x1
#define		LED0		P1_19
#define		LED1		P1_20
#define		LED2		P1_21
#define		LED3		P1_22
#define		LED4		P1_23
#define		LED5		P1_24
#define		LED6		P1_25
#define		LED7		P1_26
#define		LED_CNTRL	(LPC_GPIO1 -> FIOPIN)


#define		SW_PORT		0x0
#define		SW0			P0_2
#define		SW1			P0_3
#define		SW2			P0_4
#define		SW3			P0_5
#define		SW0_PRESSED	(!(((LPC_GPIO0->FIOPIN)&SW0)==SW0))
#define		SW1_PRESSED	(!(((LPC_GPIO0->FIOPIN)&SW1)==SW1))
#define		SW2_PRESSED	(!(((LPC_GPIO0->FIOPIN)&SW2)==SW2))
#define		SW3_PRESSED	(!(((LPC_GPIO0->FIOPIN)&SW3)==SW3))

#define		LCD_SCLK	P0_7
#define		LCD_RS		P0_0
#define		LCD_MOSI	P0_9
#define		LCD_CS		P1_18

void led_setup(void);
void delay_ms(unsigned int delay_ms);
void setup_gpio_interrupt();

#endif /* ECE471_H_ */
