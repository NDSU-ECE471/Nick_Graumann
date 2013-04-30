#ifndef __SSP_LPC_H__
#define __SSP_LPC_H__

#include "../SPI_Facade.h"


///////////////////////////////////////////////////////////////////////////////
//
// Public Functions
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E LPC_SSP0_Init(SPI_ClkDiv_T pClkDiv, SPI_ClkDiv_T busClkDiv, SPI_ClkPolarity_E clkPol, SPI_ClkPhase_E clkPhase, size_t transferSize);
SPI_Error_E LPC_SSP0_SingleTransaction(const uint16_t *txSrc, uint16_t *rxDest);

SPI_Error_E LPC_SSP0_DMA_Init();
SPI_Error_E LPC_SSP0_DMA_Transaction(const void *src, void *dest, size_t size, SPI_Callback_T txCallback, SPI_Callback_T rxCallback);


#endif //__SSP_LPC_H__
