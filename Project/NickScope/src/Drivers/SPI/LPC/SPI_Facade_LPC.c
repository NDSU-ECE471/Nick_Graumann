#include <stdio.h>
#include "../SPI_Facade.h"
#include "SSP_LPC.h"


///////////////////////////////////////////////////////////////////////////////
//
// See SPI_Facade.h
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E SPI_Init(SPI_Dev_E device, SPI_ClkDiv_T pClkDiv, SPI_ClkDiv_T busClkDiv, SPI_ClkPolarity_E clkPol, SPI_ClkPhase_E clkPhase, size_t xferSizeBits)
{
   SPI_Error_E err;

   switch(device)
   {
   case SPI_0:
      err = LPC_SSP0_Init(pClkDiv, busClkDiv, clkPol, clkPhase, xferSizeBits);
      break;

   default:
      err = SPI_INVALID_DEVICE;
   }

   return err;
}


///////////////////////////////////////////////////////////////////////////////
//
// See SPI_Facade.h
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E SPI_SetBusClkDiv(SPI_Dev_E device, SPI_ClkDiv_T busClkDiv)
{
   SPI_Error_E err;

   switch(device)
   {
   case SPI_0:
      err = LPC_SSP0_SetBusClkDiv(busClkDiv);
      break;

   default:
      err = SPI_INVALID_DEVICE;
   }

   return err;
}


///////////////////////////////////////////////////////////////////////////////
//
// See SPI_Facade.h
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E SPI_SingleTransaction(SPI_Dev_E device, const void *src, void *dest)
{
   SPI_Error_E err;

   switch(device)
   {
   case SPI_0:
      err = LPC_SSP0_SingleTransaction(src, dest);
      break;

   default:
      err = SPI_INVALID_DEVICE;
   }

   return err;
}


///////////////////////////////////////////////////////////////////////////////
//
// See SPI_Facade.h
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E SPI_DMA_Init(SPI_Dev_E device)
{
   SPI_Error_E err;

   switch(device)
   {
   case SPI_0:
      err = LPC_SSP0_DMA_Init();
      break;

   default:
      err = SPI_INVALID_DEVICE;
   }

   return err;
}


///////////////////////////////////////////////////////////////////////////////
//
// See SPI_Facade.h
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E SPI_BeginDMA_Transaction(SPI_Dev_E device, const void *src, void *dest,
                                     size_t size, SPI_Callback_T txCallback, SPI_Callback_T rxCallback)
{
   SPI_Error_E err;

   switch(device)
   {
   case SPI_0:
      err = LPC_SSP0_DMA_Transaction(src, dest, size, txCallback, rxCallback);
      break;

   default:
      err = SPI_INVALID_DEVICE;
   }

   return err;
}

