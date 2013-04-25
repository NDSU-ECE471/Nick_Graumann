#include <stdbool.h>
#include "LPC17xx.h"
#include "SSP_LPC.h"


///////////////////////////////////////////////////////////////////////////////
//
// Local defines
//
///////////////////////////////////////////////////////////////////////////////
#define LPC_SSP_FIFO_SIZE  8

// SC PCONP register
#define SSP0_ENABLE     (1<<21)

// SSPx CR0 register
#define SSP_TX_SIZE_16     (0x0F<<0)
#define SSP_CPOL           (1<<6)
#define SSP_CPHA           (1<<7)
#define SSP_CLK_RATE_1     (0<<8)

// SSPx CR1 register
#define SSP_ENABLE         (1<<1)

// SSPx SR register
#define SSP_TX_FIFO_EMPTY  (1<<0)
#define SSP_TX_FIFO_NFULL  (1<<1)
#define SSP_RX_FIFO_NEMPTY (1<<2)
#define SSP_RX_FIFO_FULL   (1<<3)
#define SSP_BUSY           (1<<4)

// Pin configuration
#define SSP0_SCK_BITS   (0x03<<30)
#define SSP0_SCK_SEL    (0x02<<30)
#define SSP0_SCK_MODE   (0x02<<30)

#define SSP0_MISO_BITS  (0x03<<2)
#define SSP0_MISO_SEL   (0x02<<2)
#define SSP0_MISO_MODE  (0x02<<2)

#define SSP0_SSEL_BITS  (0x03<<0)
#define SSP0_SSEL_SEL   (0x02<<0)
#define SSP0_SSEL_MODE  (0x02<<0)

#define PCLK_DIV_BITS   (0x03<<10)
#define PCLK_DIV_1      (0x01<<10)
#define PCLC_DIV_2      (0x02<<10)
#define PCLK_DIV_4      (0x03<<10)

// CPSR register
#define SSP0_PRESCALE_2 0x02


///////////////////////////////////////////////////////////////////////////////
//
// Local variables
//
///////////////////////////////////////////////////////////////////////////////
volatile uint32_t Trash;

static bool SSP0_Initialized = false;
static bool SSP1_Initialized = false;


///////////////////////////////////////////////////////////////////////////////
//
// Flushes the receive FIFO
//
///////////////////////////////////////////////////////////////////////////////
static void SSP_FlushRx(LPC_SSP_TypeDef *dev)
{
   for(uint32_t i=0; i<LPC_SSP_FIFO_SIZE; i++)
   {
      Trash = dev->DR;
   }
}



///////////////////////////////////////////////////////////////////////////////
//
// See SSP_LPC.h
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E LPC_SSP0_Init()
{
   SPI_Error_E err = SPI_SUCCESS;

   if(!SSP0_Initialized)
   {
      // Enable the SSP0 peripheral
      LPC_SC->PCONP |= SSP0_ENABLE;

      // Setup clock divider
      LPC_SC->PCLKSEL1 &= ~PCLK_DIV_BITS;
      LPC_SC->PCLKSEL1 |= PCLK_DIV_1;

      // Setup pins
      // P0[15]->SCK0
      LPC_PINCON->PINSEL0 &= ~SSP0_SCK_BITS;
      LPC_PINCON->PINSEL0 |= SSP0_SCK_SEL;

      LPC_PINCON->PINMODE0 &= ~SSP0_SCK_BITS;
      LPC_PINCON->PINMODE0 |= SSP0_SCK_MODE;

      // P0[17]->MISO0
      LPC_PINCON->PINSEL1 &= ~SSP0_MISO_BITS;
      LPC_PINCON->PINSEL1 |= SSP0_MISO_SEL;

      LPC_PINCON->PINMODE1 &= ~SSP0_MISO_BITS;
      LPC_PINCON->PINMODE1 |= SSP0_MISO_MODE;

      // P0[16]->SSEL0
      LPC_PINCON->PINSEL1 &= ~SSP0_SSEL_BITS;
      LPC_PINCON->PINSEL1 |= SSP0_SSEL_SEL;

      LPC_PINCON->PINMODE1 &= ~SSP0_SSEL_BITS;
      LPC_PINCON->PINMODE1 |= SSP0_SSEL_MODE;

      // Clock rate, polarity, transfer size
      LPC_SSP0->CR0 = SSP_CLK_RATE_1 | SSP_CPOL | SSP_TX_SIZE_16;

      // Clock prescaler
      LPC_SSP0->CPSR = SSP0_PRESCALE_2;

      SSP_FlushRx(LPC_SSP0);

      // Enable
      LPC_SSP0->CR1 = SSP_ENABLE;

      SSP0_Initialized = true;
   }

   return err;
}


///////////////////////////////////////////////////////////////////////////////
//
// See SSP_LPC.h
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E LPC_SSP0_Configure(SPI_ClkRate_T clkRate, SPI_ClkPolarity_E clkPol, SPI_ClkPhase_E clkPhase)
{
   SPI_Error_E err = SPI_SUCCESS;
   // todo
#if 0
   if(!SSP0_Initialized)
   {
      err = SPI_NOT_INITIALIZED;
   }
   else
   {
      LPC_SSP0->CR1 &= ~SSP_ENABLE;

      err = SSP0_SetClockSpeed(clkRate);
   }

   if(err == SPI_SUCCESS)
   {
      LPC_SSP0->CR1 |= SSP_ENABLE;
   }
#endif
   return err;
}


///////////////////////////////////////////////////////////////////////////////
//
// See SSP_LPC.h
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E LPC_SSP0_SingleTransaction(uint16_t *txSrc, uint16_t *rxDest)
{
   SPI_Error_E err = SPI_SUCCESS;

   if(!txSrc && !rxDest)
   {
      err = SPI_INVALID_PARAMETER;
   }
   else
   {
      // Ensure module is not in use
      while ((LPC_SSP0->SR & (SSP_TX_FIFO_NFULL|SSP_BUSY)) != SSP_TX_FIFO_NFULL);

      if(txSrc)
      {
         LPC_SSP0->DR = *txSrc;
      }
      else
      {
         // Clock out dummy data
         LPC_SSP0->DR = 0;
      }

      // Wait for send complete
      while(((LPC_SSP0->SR) & SSP_BUSY));

      if(rxDest)
      {
         // Wait for data received
         while(!((LPC_SSP0->SR) & SSP_RX_FIFO_NEMPTY));

         // Data is only first 16 bits at most
         *rxDest = (LPC_SSP0->DR & 0x0000FFFF);
      }
      else
      {
         // Throw away one entry from the receive FIFO
         Trash = LPC_SSP0->DR;
      }
   }

   return err;
}

