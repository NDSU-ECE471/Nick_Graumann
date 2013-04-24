#ifndef __SSP_LPC_H__
#define __SSP_LPC_H__

#include "../SPI_Facade.h"

#define LPC_SSP_FIFO_SIZE  8

SPI_Error_E SSP0_Enable();
SPI_Error_E SSP0_Receive(uint16_t *buf);

#endif //__SSP_LPC_H__
