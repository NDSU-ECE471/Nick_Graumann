#include <stdio.h>
#include "../SPI_Facade.h"
#include "SSP_LPC.h"


///////////////////////////////////////////////////////////////////////////////
//
// See SPI_Facade.h
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E SPI_Init(SPI_Dev_E device)
{
   SPI_Error_E err;

   switch(device)
   {
   case SPI_0:
      err = LPC_SSP0_Init();
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
SPI_Error_E SPI_MultiByteTransaction(SPI_Dev_E device, void *src, void *dest, size_t size)
{
   SPI_Error_E err = SPI_SUCCESS;
   uint16_t *src16 = src;
   uint16_t *dest16 = dest;

   for(size_t i=0; i<size / sizeof(uint16_t); i++)
   {
      err = LPC_SSP0_SingleTransaction(src16 ? &src16[i] : NULL,
                                       dest16 ? &dest16[i] : NULL);

      if(err != SPI_SUCCESS)
      {
         break;
      }
   }

   return err;
}

