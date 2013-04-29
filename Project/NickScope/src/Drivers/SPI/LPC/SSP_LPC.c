#include <stdbool.h>
#include <stdint.h>
#include "LPC17xx.h"
#include "../../DMA/LPC/DMA_LPC.h"
#include "Drivers/Clocks/ClocksFacade.h"
#include "SSP_LPC.h"


///////////////////////////////////////////////////////////////////////////////
//
// Local defines
//
///////////////////////////////////////////////////////////////////////////////
#define LPC_SSP_FIFO_SIZE  8

// SC PCONP register
#define SSP0_PWR_EN       (1<<21)

// SSPx CR0 register
#define SSP_XFER_SIZE(bits)   (bits-1)
#define SSP_XFER_SIZE_MIN     4
#define SSP_XFER_SIZE_MAX     16

#define SSP_CPOL              (1<<6)
#define SSP_CPHA              (1<<7)
#define SSP_SCR_BITS          (0xFF<<8)

// SSPx CR1 register
#define SSP_ENABLE         (1<<1)

// SSPx SR register
#define SSP_TX_FIFO_EMPTY  (1<<0)
#define SSP_TX_FIFO_NFULL  (1<<1)
#define SSP_RX_FIFO_NEMPTY (1<<2)
#define SSP_RX_FIFO_FULL   (1<<3)
#define SSP_BUSY           (1<<4)

// CPSR register
#define SSP0_PRESCALE_2 0x02


//DMACR register
#define SPI_DMA_RX_EN   (1<<0)
#define SPI_DMA_TX_EN   (1<<1)


// SSP0 Pin configuration
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
#define PCLK_DIV_2      (0x02<<10)
#define PCLK_DIV_4      (0x03<<10)


///////////////////////////////////////////////////////////////////////////////
//
// Local variables
//
///////////////////////////////////////////////////////////////////////////////
volatile uint32_t Trash;
volatile const uint32_t Dummy = 0;

static bool SSP0_Initialized = false;


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
// Calculates the PClock divider
//
///////////////////////////////////////////////////////////////////////////////
static SPI_Error_E SSP0_SetPClkDiv(SPI_ClkDiv_T pClkDiv)
{
   SPI_Error_E err = SPI_SUCCESS;

   LPC_SC->PCLKSEL1 &= ~PCLK_DIV_BITS;

   switch(pClkDiv)
   {
   case 1:
      LPC_SC->PCLKSEL1 |= PCLK_DIV_1;
      break;

   case 2:
      LPC_SC->PCLKSEL1 |= PCLK_DIV_2;
      break;

   case 4:
      LPC_SC->PCLKSEL1 |= PCLK_DIV_4;
      break;

   default:
      err = SPI_CLK_UNSUPPORTED;
   }

   return err;
}


///////////////////////////////////////////////////////////////////////////////
//
// Calculates the bus divider
//
///////////////////////////////////////////////////////////////////////////////
static SPI_Error_E SSP0_SetBusClkDiv(SPI_ClkDiv_T busClkDiv)
{
   SPI_Error_E err = SPI_SUCCESS;

   // Assume the SCR value is always 0 so the formula for calculating the bus clk is just
   // BCLK = PCLK / CPSDVSR. This must be an even value
   if(busClkDiv % 2 != 0)
   {
      err = SPI_CLK_UNSUPPORTED;
   }
   else
   {
      LPC_SSP0->CR0 &= ~SSP_SCR_BITS;
      LPC_SSP0->CPSR = busClkDiv;
   }

   return err;
}


///////////////////////////////////////////////////////////////////////////////
//
// Sets up pins to the default configuration
//
///////////////////////////////////////////////////////////////////////////////
static void SSP0_SetupPins()
{
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
}



///////////////////////////////////////////////////////////////////////////////
//
// See SSP_LPC.h
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E LPC_SSP0_Init(SPI_ClkDiv_T pClkDiv, SPI_ClkDiv_T busClkDiv, SPI_ClkPolarity_E clkPol, SPI_ClkPhase_E clkPhase, size_t xferSizeBits)
{
   SPI_Error_E err = SPI_SUCCESS;

   if(xferSizeBits < SSP_XFER_SIZE_MIN || xferSizeBits > SSP_XFER_SIZE_MAX)
   {
      err = SPI_INVALID_PARAMETER;
   }
   else if(!SSP0_Initialized)
   {
      // Power on the peripheral
      LPC_SC->PCONP |= SSP0_PWR_EN;

      if(SPI_SUCCESS == SSP0_SetPClkDiv(pClkDiv) && SPI_SUCCESS == SSP0_SetBusClkDiv(busClkDiv))
      {
         SSP0_SetupPins();

         // Convert transfer size to register bit representation
         LPC_SSP0->CR0 = SSP_XFER_SIZE(xferSizeBits);

         // Clock polarity
         if(SPI_CLK_POLARITY_HIGH == clkPol)
         {
            LPC_SSP0->CR0 |= SSP_CPOL;
         }

         //
         if(SPI_PHASE_SECOND_EDGE == clkPhase)
         {
            LPC_SSP0->CR0 |= SSP_CPHA;
         }

         SSP_FlushRx(LPC_SSP0);

         // Enable
         LPC_SSP0->CR1 = SSP_ENABLE;

         SSP0_Initialized = true;
      }
   }

   return err;
}


///////////////////////////////////////////////////////////////////////////////
//
// See SSP_LPC.h
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E LPC_SSP0_SingleTransaction(const uint16_t *txSrc, uint16_t *rxDest)
{
   SPI_Error_E err = SPI_SUCCESS;

   if(!SSP0_Initialized || !(LPC_SSP0->DMACR & (SPI_DMA_RX_EN|SPI_DMA_TX_EN)))
   {
      err = SPI_NOT_INITIALIZED;
   }
   else if(!txSrc && !rxDest)
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


///////////////////////////////////////////////////////////////////////////////
//
// See SSP_LPC.h
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E LPC_SSP0_DMA_Init()
{
   SPI_Error_E err = SPI_SUCCESS;

   if(!SSP0_Initialized)
   {
      err = SPI_NOT_INITIALIZED;
   }
   else
   {
      if(DMA_SUCCESS != LPC_DMA_Init())
      {
         err = SPI_DMA_SETUP_FAILED;
      }
      else
      {
         LPC_SSP0->DMACR = SPI_DMA_RX_EN|SPI_DMA_TX_EN;
      }
   }

   return err;
}


///////////////////////////////////////////////////////////////////////////////
//
// See SSP_LPC.h
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E LPC_SSP0_DMA_Transaction(const void *src, void *dest, size_t size, SPI_Callback_T callback)
{
   SPI_Error_E err = SPI_SUCCESS;

   if(!SSP0_Initialized)
   {
      err = SPI_NOT_INITIALIZED;
   }
   if((!src && !dest) || size == 0 || !callback)
   {
      err = SPI_INVALID_PARAMETER;
   }
   else
   {
      DMA_Channel_T txChan, rxChan;
      DMA_Error_E dmaErr;

      do
      {
         dmaErr = LPC_DMA_FindFreeChannel(&rxChan);
         if(DMA_SUCCESS != dmaErr)
         {
            break;
         }

         bool incDest = dest ? true : false;
         volatile void *destAddr = dest ? dest : &Trash;
         dmaErr = LPC_DMA_InitChannel(rxChan,
                                      (DMA_Address_T)&LPC_SSP0->DR, (DMA_Address_T)destAddr,
                                      DMA_PERIPH_SPI0, DMA_PERIPH_MEMORY, 1,
                                      DMA_BURST_SIZE_1, DMA_BURST_SIZE_1,
                                      DMA_XFER_WIDTH_16, DMA_XFER_WIDTH_16,
                                      false, incDest);
         if(DMA_SUCCESS != dmaErr)
         {
            break;
         }

         //todo: cleanup numbers in this area
         //dmaErr = LPC_DMA_FindFreeChannel(&txChan);
         txChan = 1;
         if(DMA_SUCCESS != dmaErr)
         {
            break;
         }

         bool incSrc = src ? true : false;
         volatile const void *srcAddr = src ? src : &Dummy;
         dmaErr = LPC_DMA_InitChannel(txChan,
                                      (DMA_Address_T)srcAddr, (DMA_Address_T)&LPC_SSP0->DR,
                                      DMA_PERIPH_MEMORY, DMA_PERIPH_SPI0, 1,
                                      DMA_BURST_SIZE_1, DMA_BURST_SIZE_1,
                                      DMA_XFER_WIDTH_16, DMA_XFER_WIDTH_16,
                                      incSrc, false);
         if(DMA_SUCCESS != dmaErr)
         {
            break;
         }

         dmaErr = LPC_DMA_BeginTransfer(rxChan);
         if(DMA_SUCCESS != dmaErr)
         {
            break;
         }

         dmaErr = LPC_DMA_BeginTransfer(txChan);
         if(DMA_SUCCESS != dmaErr)
         {
            break;
         }
      }
      while(0);

      if(DMA_SUCCESS != dmaErr)
      {
         err = SPI_DMA_SETUP_FAILED;
      }
   }

   return err;
}

