#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "LPC17xx.h"
#include "../../DMA/DMA_Facade.h"
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
#define SSP_CPSR_BITS      (0xFF)
#define SSP_CPSR_MAX_VAL   254


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
static SPI_Callback_T SSP0_DMA_TransactionTxCallback = NULL;
static SPI_Callback_T SSP0_DMA_TransactionRxCallback = NULL;


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
// Receive DMA Callback
//
///////////////////////////////////////////////////////////////////////////////
static void SSP0_DMA_RxCallback(DMA_Error_E dmaErr)
{
   if(SSP0_DMA_TransactionRxCallback)
   {
      if(DMA_SUCCESS == dmaErr)
      {
         (*SSP0_DMA_TransactionRxCallback)(SPI_SUCCESS);
      }
      else
      {
         (*SSP0_DMA_TransactionRxCallback)(SPI_DMA_XFER_FAILED);
      }
   }
}


///////////////////////////////////////////////////////////////////////////////
//
// Transmit DMA Callback
//
///////////////////////////////////////////////////////////////////////////////
static void SSP0_DMA_TxCallback(DMA_Error_E dmaErr)
{
   if(SSP0_DMA_TransactionTxCallback)
   {
      if(DMA_SUCCESS == dmaErr)
      {
         (*SSP0_DMA_TransactionTxCallback)(SPI_SUCCESS);
      }
      else
      {
         (*SSP0_DMA_TransactionTxCallback)(SPI_DMA_XFER_FAILED);
      }
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
static SPI_Error_E SSP0_SetBusClkDivInternal(SPI_ClkDiv_T busClkDiv)
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
      uint32_t cpsrVal = busClkDiv;
      uint8_t cr0Val = 1;

      while(cpsrVal > SSP_CPSR_MAX_VAL)
      {
         cpsrVal /= 2;
         cr0Val *= 2;
      }

      LPC_SSP0->CR0 &= ~SSP_SCR_BITS;
      LPC_SSP0->CR0 |= ((cr0Val-1)<<8);
      LPC_SSP0->CPSR = (cpsrVal & SSP_CPSR_BITS);
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

      if(SPI_SUCCESS == SSP0_SetPClkDiv(pClkDiv) && SPI_SUCCESS == SSP0_SetBusClkDivInternal(busClkDiv))
      {
         SSP0_SetupPins();

         // Convert transfer size to register bit representation
         // CR0 has already been initially setup by SSP0_SetBusClkDiv()
         LPC_SSP0->CR0 |= SSP_XFER_SIZE(xferSizeBits);

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
SPI_Error_E LPC_SSP0_SetBusClkDiv(SPI_ClkDiv_T busClkDiv)
{
   SPI_Error_E err = SPI_SUCCESS;

   if(!SSP0_Initialized)
   {
      err = SPI_NOT_INITIALIZED;
   }
   else
   {
      LPC_SSP0->CR1 &= ~SSP_ENABLE;
      err = SSP0_SetBusClkDivInternal(busClkDiv);
      LPC_SSP0->CR1 = SSP_ENABLE;
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
      if(DMA_SUCCESS != DMA_Init())
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
SPI_Error_E LPC_SSP0_DMA_Transaction(const void *src, void *dest, size_t size, SPI_Callback_T txCallback, SPI_Callback_T rxCallback)
{
   SPI_Error_E err = SPI_SUCCESS;

   if(!SSP0_Initialized)
   {
      err = SPI_NOT_INITIALIZED;
   }
   if((!src && !dest) || size == 0 || (!txCallback && !rxCallback))
   {
      err = SPI_INVALID_PARAMETER;
   }
   else
   {
      DMA_Channel_T txChan, rxChan;
      DMA_Error_E dmaErr;

      do
      {
         dmaErr = DMA_FindFreeChannel(&rxChan);
         if(DMA_SUCCESS != dmaErr)
         {
            break;
         }

         // Trial and error showed that this combination of burst size with the standard
         // transfer size was the most reliable. If a different combination is needed,
         // it can be added as a parameter in the future.
         bool incDest = dest ? true : false;
         volatile void *destAddr = dest ? dest : &Trash;
         dmaErr = DMA_InitChannel(rxChan,
                                      (DMA_Address_T)&LPC_SSP0->DR, (DMA_Address_T)destAddr,
                                      DMA_PERIPH_SPI0, DMA_PERIPH_MEMORY, size,
                                      DMA_BURST_SIZE_1, DMA_BURST_SIZE_1,
                                      DMA_XFER_WIDTH_16, DMA_XFER_WIDTH_16,
                                      false, incDest);
         if(DMA_SUCCESS != dmaErr)
         {
            break;
         }

         dmaErr = DMA_FindFreeChannel(&txChan);
         if(DMA_SUCCESS != dmaErr)
         {
            break;
         }

         bool incSrc = src ? true : false;
         volatile const void *srcAddr = src ? src : &Dummy;
         dmaErr = DMA_InitChannel(txChan,
                                      (DMA_Address_T)srcAddr, (DMA_Address_T)&LPC_SSP0->DR,
                                      DMA_PERIPH_MEMORY, DMA_PERIPH_SPI0, size,
                                      DMA_BURST_SIZE_1, DMA_BURST_SIZE_1,
                                      DMA_XFER_WIDTH_16, DMA_XFER_WIDTH_16,
                                      incSrc, false);
         if(DMA_SUCCESS != dmaErr)
         {
            break;
         }

         // Setup the callbacks before the DMA transaction starts
         SSP0_DMA_TransactionTxCallback = txCallback;
         SSP0_DMA_TransactionRxCallback = rxCallback;

         dmaErr = DMA_BeginTransfer(txChan, &SSP0_DMA_TxCallback);
         if(DMA_SUCCESS != dmaErr)
         {
            break;
         }

         dmaErr = DMA_BeginTransfer(rxChan, &SSP0_DMA_RxCallback);
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

