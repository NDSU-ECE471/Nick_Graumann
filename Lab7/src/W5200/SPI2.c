#include "lpc17xx.h"
#include "config.h"
#include "socket.h"
#include "w5200.h"
#include "util.h"
#include "../DRIVERS/ssp.h"
#include <string.h>
#include <stdlib.h>

void WIZ_SPI_Init(void)
{
	SSP0Init();
}

// Connected to Data Flash
void WIZ_CS(uint8_t val)
{
	if (val == LOW) {
   		LPC_GPIO0->FIOCLR = (1<<16);
	}else if (val == HIGH){
   		LPC_GPIO0->FIOSET = (1<<16);
	}
}


uint8_t SPI1_SendByte(uint8_t byte)
{
	uint8_t recieved_datas=0;
	recieved_datas= SSP0Send(byte);
	return recieved_datas;
}

/*
void SPI1_TXByte(uint8_t byte)
{
	  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);       

	  SPI_I2S_SendData(SPI1, byte);	
}
*/
