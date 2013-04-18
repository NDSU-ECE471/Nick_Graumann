// SSP Basic SPI functions
// Date: 9/1/2012

#include "LPC17xx.h"			/* LPC17xx Peripheral Registers */
#include "../DEV/ece471.h"
#include "ssp.h"

void SSP0Init( void )
{
  uint8_t i, Dummy=Dummy;

  /* Enable AHB clock to the SSP0. */
  LPC_SC->PCONP |= (0x1<<21);

  /* Further divider is needed on SSP0 clock. Using default divided by 4 */
  LPC_SC->PCLKSEL1 &= ~(0x3<<10);

  /* P0.15~0.18 as SSP0 */
  LPC_PINCON->PINSEL0 &= ~(0x3UL<<30);
  LPC_PINCON->PINSEL0 |= (0x2UL<<30); // P0.15 to SCLK
  LPC_PINCON->PINSEL1 &= ~((0x3<<2)|(0x3<<4)); // P0.17 to MISO0
  LPC_PINCON->PINSEL1 |= ((0x2<<2)|(0x2<<4)); // P0.18 to MOSI

  /* Set DSS data to 8-bit, Frame format SPI, CPOL = 0, CPHA = 0, and SCR is 15 */
  LPC_SSP0->CR0 = (7 << 0) | (1 << 8);

  // 3:0 --> 7 for 8 bit transfer
  // 5:4 --> 0 for SPI format
  // 7:6 --> 0 for clock polairity/phase selection
  // 15:8 --> 7 for clock divider

  /* SSPCPSR clock prescale register, master mode, minimum divisor is 0x02 */

  /* SSPCPSR clock prescale register, master mode, minimum divisor is 0x02 */
  LPC_SSP0->CPSR = 0x2;

  for ( i = 0; i < FIFOSIZE; i++ )
  {
	Dummy = LPC_SSP0->DR;		/* clear the RxFIFO */
  }

  /* Enable the SSP Interrupt */
//  NVIC_EnableIRQ(SSP0_IRQn);

  /* Device select as master, SSP Enabled */

  LPC_SSP0->CR1 = SSPCR1_SSE;

  /* Set SSPINMS registers to enable interrupts */
  /* enable all error related interrupts */
 // LPC_SSP0->IMSC = SSPIMSC_RTIM; //Receive after timeout occurs

  return;
}

uint8_t SSP0Send(uint8_t buf)
{
	uint8_t local_buf = buf; //Local send var
	while ( (LPC_SSP0->SR & (SSPSR_TNF|SSPSR_BSY)) != SSPSR_TNF ); // Ensure module is not in use
	LPC_SSP0->DR = local_buf; // Send Data
	while ( (LPC_SSP0->SR & (SSPSR_TNF|SSPSR_BSY)) != SSPSR_TNF ); // Ensure module is not in use
	local_buf = (LPC_SSP0->DR);
	return local_buf;
}

void SSP1Init( void )
{
  uint8_t i, Dummy=Dummy;

  /* Enable AHB clock to the SSP1. */
  LPC_SC->PCONP |= (0x1<<10);

  /* Further divider is needed on SSP1 clock. Using default divided by 4 */
  LPC_SC->PCLKSEL0 &= ~(0x3<<20);

  /* P0.6~0.9 as SSP1 */
  LPC_PINCON->PINSEL0 &= ~((0x3<<14)|(0x3<<16)|(0x3<<18));  //  15:14 --> P0.7 CLR, 17:16 --> P0.8 CLR, 19:18 --> P0.9 CLR
  LPC_PINCON->PINSEL0 |= ((0x2<<14)|(0x2<<16)|(0x2<<18)); //  15:14 --> P0.7 to SCLK, 17:16 --> P0.8 to MISO1, 19:18 --> P0.9 to MOSI1

  /* Set DSS data to 8-bit, Frame format SPI, CPOL = 0, CPHA = 0, and SCR is 15 */
  LPC_SSP1->CR0 = (7 << 0) |
		  (1 << 8);

  // 3:0 --> 7 for 8 bit transfer
  // 5:4 --> 0 for SPI format
  // 7:6 --> 0 for clock polairity/phase selection
  // 15:8 --> 7 for clock divider


  /* SSPCPSR clock prescale register, master mode, minimum divisor is 0x02 */
  LPC_SSP1->CPSR = 0x2;

  for ( i = 0; i < FIFOSIZE; i++ )
  {
	Dummy = LPC_SSP1->DR;		/* clear the RxFIFO */
  }

  /* Enable the SSP Interrupt */
  //NVIC_EnableIRQ(SSP1_IRQn); // Don't want to mess with interrupts

  /* Device select as master, SSP Enabled */
  /* Master mode */
  LPC_SSP1->CR1 = SSPCR1_SSE;

}

void SSP1Send(uint8_t buf)
{
	uint8_t local_buf = buf; //Local send var
	LPC_GPIO1 -> FIOCLR |= LCD_CS; // SET Chip Select Low AKA select LCD device

	while ( (LPC_SSP1->SR & (SSPSR_TNF|SSPSR_BSY)) != SSPSR_TNF ); // Ensure module is not in use
	LPC_SSP1->DR = local_buf; // Send Data

	while(((LPC_SSP1->SR) & SSPSR_BSY)); // Wait until SSP1 is complete in sending data

	LPC_GPIO1 -> FIOSET |= LCD_CS; // SET Chip Select High AKA de-select LCD device.


	  /* Wait until the Busy bit is cleared. */

}







