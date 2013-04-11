/*
 * adc.c
 *
 *  Created on: Mar 25, 2013
 *      Author: Nate
 */

#include "adc.h"
#include "lpc17xx.h"
#include "../DEV/ece471.h"

/*
volatile uint32_t ADCValue[ADC_NUM];

void ADCInit( uint32_t ADC_Clk )
{
  uint32_t i, pclkdiv, pclk;

  // Enable CLOCK into ADC controller
  LPC_SC->PCONP |= (1 << 12); // Turn on ADC CLK

  for ( i = 0; i < ADC_NUM; i++ )
  {
	ADCValue[i] = 0x0;
  }

  // all the related pins are set to ADC inputs, AD0.0~7 //
  LPC_PINCON->PINSEL1 &= ~0x003FC000;	// P0.23~26, A0.0~3, function 01 //
  LPC_PINCON->PINSEL1 |= 0x00154000;
  // No pull-up no pull-down (function 10) on these ADC pins. //
  LPC_PINCON->PINMODE1 &= ~0x003FC000;
  LPC_PINCON->PINMODE1 |= 0x002A8000;

  // By default, the PCLKSELx value is zero, thus, the PCLK for
  //all the peripherals is 1/4 of the SystemFrequency. //
  // Bit 24~25 is for ADC //
  pclkdiv = (LPC_SC->PCLKSEL0 >> 24) & 0x03;
  switch ( pclkdiv )
  {
	case 0x00:
	default:
	  pclk = SystemCoreClock/4;
	break;
	case 0x01:
	  pclk = SystemCoreClock;
	break;
	case 0x02:
	  pclk = SystemCoreClock/2;
	break;
	case 0x03:
	  pclk = SystemCoreClock/8;
	break;
  }

  LPC_ADC->ADCR = ( 0x01 << 0 ) |  // SEL=1,select channel 0~7 on ADC0 //
		( ( pclk  / ADC_Clk - 1 ) << 8 ) |  // CLKDIV = Fpclk / ADC_Clk - 1 //
		( 0 << 16 ) | 		// BURST = 0, no BURST, software controlled //
		( 0 << 17 ) |  		// CLKS = 0, 11 clocks/10 bits //
		( 1 << 21 ) |  		// PDN = 1, normal operation //
		( 0 << 24 ) |  		// START = 0 A/D conversion stops //
		( 0 << 27 );		// EDGE = 0 (CAP/MAT singal falling,trigger A/D conversion) //

  // If POLLING, no need to do the following //
#if ADC_INTERRUPT_FLAG
  NVIC_EnableIRQ(ADC_IRQn);
#if BURST_MODE
  LPC_ADC->INTEN = 0xFF;		// Enable all interrupts //
#else
  LPC_ADC->INTEN = 0x1FF;		// Enable all interrupts //
#endif
#endif
  return;
}

*/

void adc_init()
{
	  LPC_SC->PCONP |= (1 << 12); // Turn on ADC CLK

	  //LPC_PINCON->PINSEL1 &= ~ (3<< 14); // CLR P0.23
	  //LPC_PINCON->PINSEL1 |= (1<< 14); // Set P0.23 to ADC Mode

	 // LPC_PINCON->PINMODE1 &= ~(3<<14); // CLR P0.23 Pin Mode
	 // LPC_PINCON->PINMODE1 |= (1<<15); // Set P0.23 as no PU/PD
	  LPC_PINCON->PINSEL1 &= ~0x003FC000;	// P0.23~26, A0.0~3, function 01 //
	  LPC_PINCON->PINSEL1 |= 0x00154000;

	  LPC_PINCON->PINMODE1 &= ~0x003FC000;
	  LPC_PINCON->PINMODE1 |= 0x002A8000;

	  LPC_ADC->ADCR = ( 0x00 << 0 ) |  // SEL=1,select channel 0~7 on ADC0 */
			( 7 << 8 ) |  // CLKDIV = Fpclk / ADC_Clk - 1
			( 0 << 16 ) | 		// BURST = 0, no BURST, software controlled
			( 0 << 17 ) |  		// CLKS = 0, 11 clocks/10 bits
			( 1 << 21 ) |  		// PDN = 1, normal operation
			( 0 << 24 ) |  		// START = 0 A/D conversion stops
			( 0 << 27 );		// EDGE = 0 (CAP/MAT singal falling,trigger A/D conversion)
	  delay_ms(1);

	  //LPC_ADC->ADCR |= (1<<24); // Start ADC Conversion
}

uint16_t ADCRead(uint8_t channelNum)
{
	uint16_t adc_result = 0;
	LPC_ADC->ADCR &= ~(0xF);
	LPC_ADC->ADCR |= (1 << 24) | (1 << channelNum);
	 delay_ms(1);

	 switch(channelNum)
	 {
	 	 case 0:
	 		 adc_result = ((LPC_ADC->ADDR0)>> 4)&(0xFFF); // Read ADC Data
	 		 break;

	 	 case 1:
	 		 adc_result = ((LPC_ADC->ADDR1)>> 4)&(0xFFF); // Read ADC Data
	 		 break;

	 	 case 2:
	 		 adc_result = ((LPC_ADC->ADDR2)>> 4)&(0xFFF); // Read ADC Data
	 		 break;

	 	 case 3:
	 		 adc_result = ((LPC_ADC->ADDR3)>> 4)&(0xFFF); // Read ADC Data
	 		 break;
	 }
	return adc_result;
}



/*
uint32_t ADCRead( uint8_t channelNum )
{
#if !ADC_INTERRUPT_FLAG
  uint32_t regVal, ADC_Data;
#endif

 //  channel number is 0 through 7
  if ( channelNum >= ADC_NUM )
  {
	channelNum = 0;		// reset channel number to 0
  }
  LPC_ADC->ADCR &= 0xFFFFFF00;
  LPC_ADC->ADCR |= (1 << 24) | (1 << channelNum);
				// switch channel,start A/D convert
#if !ADC_INTERRUPT_FLAG
  while ( 1 )			// wait until end of A/D convert
  {
	regVal = LPC_ADC->ADDR0;
	// read result of A/D conversion
	if ( regVal & ADC_DONE )
	{
	  break;
	}
  }

  LPC_ADC->ADCR &= 0xF8FFFFFF;	//stop ADC now//
  if ( regVal & ADC_OVERRUN )	// save data when it's not overrun, otherwise, return zero //
  {
	return ( 0 );
  }
  ADC_Data = ( regVal >> 4 ) & 0xFFF;
  return ( ADC_Data );	// return A/D conversion value //
#else
  return ( channelNum );	// if it's interrupt driven, the ADC reading is
							done inside the handler. so, return channel number //
#endif
}

*/




