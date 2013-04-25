#ifndef __SSP_LPC_H__
#define __SSP_LPC_H__

#include "../SPI_Facade.h"


///////////////////////////////////////////////////////////////////////////////
//
// Public Functions
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E LPC_SSP0_Init();
SPI_Error_E LPC_SSP0_Configure(SPI_ClkRate_T clkRate, SPI_ClkPolarity_E clkPol, SPI_ClkPhase_E clkPhase);
SPI_Error_E LPC_SSP0_SingleTransaction(uint16_t *txSrc, uint16_t *rxDest);


#endif //__SSP_LPC_H__
