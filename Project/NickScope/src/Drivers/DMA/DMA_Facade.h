#ifndef __DMA_FACADE_H__
#define __DMA_FACADE_H__


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
   DMA_UNSUPPORTED_PERIPHERAL
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


#endif //__DMA_FACADE_H__
