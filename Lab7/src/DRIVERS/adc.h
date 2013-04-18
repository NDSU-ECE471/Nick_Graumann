/*
 * adc.h
 *
 *  Created on: Mar 25, 2013
 *      Author: Nate
 */

#ifndef ADC_H_
#define ADC_H_

#include "stdint.h"

#define ADC_OFFSET      0x10
#define ADC_INDEX       4

#define ADC_DONE        0x80000000
#define ADC_OVERRUN     0x40000000
#define ADC_ADINT       0x00010000

#define ADC_NUM			8		/* for LPCxxxx */
#define ADC_CLK			1000000		/* set to 1Mhz */



//void ADCInit( uint32_t ADC_Clk );
//uint32_t ADCRead( uint8_t channelNum );

void adc_init();
uint16_t ADCRead(uint8_t channelNum);




#endif /* ADC_H_ */
