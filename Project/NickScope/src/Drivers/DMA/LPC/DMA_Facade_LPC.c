#include <string.h>
#include "LPC17xx.h"
#include "../DMA_Facade.h"


///////////////////////////////////////////////////////////////////////////////
//
// Local defines
//
///////////////////////////////////////////////////////////////////////////////
#define LPC_DMA_NUM_CHANNELS  7

// SC PCONP register
#define DMA_PWR_EN            (1<<29)

// LPC_GPDMA register
#define DMA_ENABLE            (1<<0)
#define DMA_LITTLE_ENDIAN     (0<<1)
#define DMA_BIG_ENDIAN        (1<<1)

// LPC_GPDMACH register
#define DMA_XFER_SIZE_MAX     0x0FFF


static LPC_GPDMACH_TypeDef *LPC_GPDMACH[] =
{
   LPC_GPDMACH0,
   LPC_GPDMACH1,
   LPC_GPDMACH2,
   LPC_GPDMACH3,
   LPC_GPDMACH4,
   LPC_GPDMACH5,
   LPC_GPDMACH6,
   LPC_GPDMACH7
};


#define DMA_SRC_BURST_OFFSET  12
#define DMA_DEST_BURST_OFFSET 15

static const uint8_t DMA_BurstBits[] =
{
   [DMA_BURST_SIZE_1]   = 0x00,
   [DMA_BURST_SIZE_4]   = 0x01,
   [DMA_BURST_SIZE_8]   = 0x02,
   [DMA_BURST_SIZE_16]  = 0x03,
   [DMA_BURST_SIZE_32]  = 0x04,
   [DMA_BURST_SIZE_64]  = 0x05,
   [DMA_BURST_SIZE_128] = 0x06,
   [DMA_BURST_SIZE_256] = 0x07
};

#define DMA_SRC_WIDTH_OFFSET  18
#define DMA_DEST_WIDTH_OFFSET 21

static const uint8_t DMA_XferWidthBits[] =
{
   [DMA_XFER_WIDTH_8]  = 0x00,
   [DMA_XFER_WIDTH_16] = 0x01,
   [DMA_XFER_WIDTH_32] = 0x02
};

#define DMA_SRC_INC_BIT    (1<<26)
#define DMA_DEST_INC_BIT   (1<<27)


#define DMA_SRC_PERIPH_OFFSET    1
#define DMA_DEST_PERIPH_OFFSET   6

static const uint8_t DMA_PeripheralSrcBits[] =
{
   [DMA_PERIPH_SPI0]  = 0x01,
};

static const uint8_t DMA_PeripheralDestBits[] =
{
   [DMA_PERIPH_SPI0]  = 0x00,
};

#define DMA_TC_IRQ_BIT     (1<<31)

#define DMA_XFER_M2M_BITS  (0x00<<11)
#define DMA_XFER_M2P_BITS  (0x01<<11)
#define DMA_XFER_P2M_BITS  (0x02<<11)
#define DMA_XFER_P2P_BITS  (0x03<<11)

#define DMA_ERROR_INT_BIT  (1<<14)
#define DMA_TC_INT_BIT     (1<<15)
#define DMA_CONFIG_EN_BIT  (1<<0)


///////////////////////////////////////////////////////////////////////////////
//
// Local variables
//
///////////////////////////////////////////////////////////////////////////////
static bool DMA_Initialized = false;
static bool DMA_ChannelInUse[LPC_DMA_NUM_CHANNELS];
static DMA_Callback_T DMA_Callbacks[LPC_DMA_NUM_CHANNELS];


///////////////////////////////////////////////////////////////////////////////
//
// Verifies the perhiperal is supported under this implementation
//
///////////////////////////////////////////////////////////////////////////////
static DMA_Error_E VerifyDMA_Peripheral(DMA_Peripheral_E periph)
{
   DMA_Error_E err = DMA_SUCCESS;

   if(periph != DMA_PERIPH_MEMORY && periph != DMA_PERIPH_SPI0)
   {
      err = DMA_UNSUPPORTED_PERIPHERAL;
   }

   return err;
}


///////////////////////////////////////////////////////////////////////////////
//
// See DMA_Facade.h
//
///////////////////////////////////////////////////////////////////////////////
DMA_Error_E DMA_Init()
{
   DMA_Error_E err = DMA_SUCCESS;

   if(!DMA_Initialized)
   {
      memset(DMA_ChannelInUse, 0, sizeof(DMA_ChannelInUse));
      memset(DMA_Callbacks, 0, sizeof(DMA_Callbacks));

      LPC_SC->PCONP |= DMA_PWR_EN;
      LPC_GPDMA->DMACConfig = DMA_ENABLE | DMA_LITTLE_ENDIAN;
      while(!(LPC_GPDMA->DMACConfig & DMA_ENABLE));

      NVIC_EnableIRQ(DMA_IRQn);

      DMA_Initialized = true;
   }

   return err;
}


///////////////////////////////////////////////////////////////////////////////
//
// See DMA_Facade.h
//
///////////////////////////////////////////////////////////////////////////////
DMA_Error_E DMA_InitChannel(DMA_Channel_T channel,
                                DMA_Address_T srcAddr, DMA_Address_T destAddr,
                                DMA_Peripheral_E srcPeriph, DMA_Peripheral_E destPeriph,
                                DMA_TransferSize_T xferSize,
                                DMA_BurstSize_E srcBurstSize, DMA_BurstSize_E destBurstSize,
                                DMA_TransferWidth_E srcWidth, DMA_TransferWidth_E destWidth,
                                bool incSrcAddr, bool incDestAddr)
{
   DMA_Error_E err = DMA_SUCCESS;

   if(!DMA_Initialized)
   {
      err = DMA_NOT_INITIALIZED;
   }
   else if(channel > LPC_DMA_NUM_CHANNELS)
   {
      err = DMA_INVALID_PARAMETER;
   }
   else
   {
      do
      {
         err = VerifyDMA_Peripheral(srcPeriph);
         if(DMA_SUCCESS != err)
         {
            break;
         }

         err = VerifyDMA_Peripheral(destPeriph);
         if(DMA_SUCCESS != err)
         {
            break;
         }

         // Clear all interrupts for this channel
         LPC_GPDMA->DMACIntTCClear = (1<<channel);
         LPC_GPDMA->DMACIntErrClr = (1<<channel);

         // Set up addresses
         LPC_GPDMACH[channel]->DMACCSrcAddr = srcAddr;
         LPC_GPDMACH[channel]->DMACCDestAddr = destAddr;

         // No linked-list
         LPC_GPDMACH[channel]->DMACCLLI = (uint32_t)NULL;

         // Set up control register (burst size, width, etc..)
         LPC_GPDMACH[channel]->DMACCControl = (xferSize & DMA_XFER_SIZE_MAX) |
                                              (DMA_BurstBits[srcBurstSize]<<DMA_SRC_BURST_OFFSET) |
                                              (DMA_BurstBits[destBurstSize]<<DMA_DEST_BURST_OFFSET) |
                                              (DMA_XferWidthBits[srcWidth]<<DMA_SRC_WIDTH_OFFSET) |
                                              (DMA_XferWidthBits[destWidth]<<DMA_DEST_WIDTH_OFFSET) |
                                              (incSrcAddr ? DMA_DEST_INC_BIT : 0) |
                                              (incDestAddr ? DMA_DEST_INC_BIT : 0) |
                                              DMA_TC_IRQ_BIT;

         // Setup peripherals, they were already verified above. The DMA controller needs to know what type of peripheral
         // the source and destination addresses represent.

         // peripheral->peripheral
         if(DMA_PERIPH_MEMORY != srcPeriph && DMA_PERIPH_MEMORY != destPeriph)
         {
            LPC_GPDMACH[channel]->DMACCConfig = (DMA_PeripheralSrcBits[srcPeriph]<<DMA_SRC_PERIPH_OFFSET) |
                                                (DMA_PeripheralDestBits[destPeriph]<<DMA_DEST_PERIPH_OFFSET) |
                                                DMA_XFER_P2P_BITS;
         }
         // Peripheral->memory
         else if(DMA_PERIPH_MEMORY != srcPeriph && DMA_PERIPH_MEMORY == destPeriph)
         {
            LPC_GPDMACH[channel]->DMACCConfig = (DMA_PeripheralSrcBits[srcPeriph]<<DMA_SRC_PERIPH_OFFSET) |
                                                DMA_XFER_P2M_BITS;
         }
         // Memory->peripheral
         else if(DMA_PERIPH_MEMORY == srcPeriph && DMA_PERIPH_MEMORY != destPeriph)
         {
            LPC_GPDMACH[channel]->DMACCConfig = (DMA_PeripheralDestBits[destPeriph]<<DMA_DEST_PERIPH_OFFSET) |
                                                DMA_XFER_M2P_BITS;
         }
         else // Memory->memory
         {
            LPC_GPDMACH[channel]->DMACCConfig = DMA_XFER_M2M_BITS;
         }

         DMA_ChannelInUse[channel] = true;
      }
      while(0);
   }

   return err;
}


///////////////////////////////////////////////////////////////////////////////
//
// See DMA_Facade.h
//
///////////////////////////////////////////////////////////////////////////////
DMA_Error_E DMA_BeginTransfer(DMA_Channel_T channel, DMA_Callback_T callback)
{
   DMA_Error_E err = DMA_SUCCESS;

   if(!DMA_Initialized)
   {
      err = DMA_NOT_INITIALIZED;
   }
   else if(channel > LPC_DMA_NUM_CHANNELS || !DMA_ChannelInUse[channel])
   {
      err = DMA_INVALID_PARAMETER;
   }
   else
   {
      DMA_Callbacks[channel] = callback;

      // Enable error and terminal count interrupts, enable channel
      LPC_GPDMACH[channel]->DMACCConfig |= DMA_ERROR_INT_BIT|DMA_TC_INT_BIT|DMA_CONFIG_EN_BIT;
   }

   return err;
}


///////////////////////////////////////////////////////////////////////////////
//
// See DMA_Facade.h
//
///////////////////////////////////////////////////////////////////////////////
DMA_Error_E DMA_FindFreeChannel(DMA_Channel_T *channel)
{
   DMA_Error_E err = DMA_NO_FREE_CHANNELS;

   if(!channel)
   {
      err = DMA_INVALID_PARAMETER;
   }
   else
   {
      for(DMA_Channel_T ch=0; ch<LPC_DMA_NUM_CHANNELS; ch++)
      {
         if(!DMA_ChannelInUse[ch])
         {
            err = DMA_SUCCESS;
            *channel = ch;
            break;
         }
      }
   }

   return err;
}


///////////////////////////////////////////////////////////////////////////////
//
// Interrupt handlers
//
///////////////////////////////////////////////////////////////////////////////
void DMA_IRQHandler(void)
{
   uint32_t regTCStat = LPC_GPDMA->DMACIntTCStat;
   uint32_t regErrStat = LPC_GPDMA->DMACIntErrStat;

   for(DMA_Channel_T ch=0; ch<LPC_DMA_NUM_CHANNELS; ch++)
   {
      // Check if the transfer is complete on each channel
      if(regTCStat & (1<<ch) && DMA_Callbacks[ch])
      {
         DMA_ChannelInUse[ch] = false;

         if(regErrStat & (1<<ch))
         {
            (*DMA_Callbacks[ch])(DMA_XFER_FAILED);
         }
         else
         {
            (*DMA_Callbacks[ch])(DMA_SUCCESS);
         }
      }
   }

   // Clear DMA interrupts
   LPC_GPDMA->DMACIntTCClear = regTCStat;
   LPC_GPDMA->DMACIntErrClr = regErrStat;
}

