#ifndef __DMA_FACADE_H__
#define __DMA_FACADE_H__


#include <stdbool.h>
#include <stdint.h>


///////////////////////////////////////////////////////////////////////////////
//
// Enumerations
//
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
   DMA_SUCCESS = 0,
   DMA_NOT_INITIALIZED,
   DMA_INVALID_PARAMETER,
   DMA_NO_FREE_CHANNELS,
   DMA_UNSUPPORTED_PERIPHERAL,
   DMA_XFER_FAILED
} DMA_Error_E;


typedef enum
{
   DMA_BURST_SIZE_1 = 0,
   DMA_BURST_SIZE_4,
   DMA_BURST_SIZE_8,
   DMA_BURST_SIZE_16,
   DMA_BURST_SIZE_32,
   DMA_BURST_SIZE_64,
   DMA_BURST_SIZE_128,
   DMA_BURST_SIZE_256
} DMA_BurstSize_E;


typedef enum
{
   DMA_XFER_WIDTH_8,
   DMA_XFER_WIDTH_16,
   DMA_XFER_WIDTH_32
} DMA_TransferWidth_E;


typedef enum
{
   DMA_PERIPH_MEMORY,
   DMA_PERIPH_SPI0
} DMA_Peripheral_E;


///////////////////////////////////////////////////////////////////////////////
//
// Types
//
///////////////////////////////////////////////////////////////////////////////
typedef uint8_t DMA_Channel_T;
typedef uint32_t DMA_Address_T;
typedef uint16_t DMA_TransferSize_T;
typedef void (*DMA_Callback_T)(DMA_Error_E);


///////////////////////////////////////////////////////////////////////////////
//
// Public Functions
//
///////////////////////////////////////////////////////////////////////////////
DMA_Error_E DMA_Init();
DMA_Error_E DMA_InitChannel(DMA_Channel_T channel,
                                DMA_Address_T srcAddr, DMA_Address_T destAddr,
                                DMA_Peripheral_E srcPeriph, DMA_Peripheral_E destPeriph,
                                DMA_TransferSize_T xferSize,
                                DMA_BurstSize_E srcBurstSize, DMA_BurstSize_E destBurstSize,
                                DMA_TransferWidth_E srcWidth, DMA_TransferWidth_E destWidth,
                                bool incSrcAddr, bool incDestAddr);
DMA_Error_E DMA_BeginTransfer(DMA_Channel_T channel, DMA_Callback_T callback);
DMA_Error_E DMA_FindFreeChannel(DMA_Channel_T *channel);


#endif //__DMA_FACADE_H__
