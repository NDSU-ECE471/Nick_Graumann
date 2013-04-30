#ifndef __SPI_FACADE_H__
#define __SPI_FACADE_H__


#include <stdint.h>


///////////////////////////////////////////////////////////////////////////////
//
// Enumerations
//
///////////////////////////////////////////////////////////////////////////////
typedef enum
{
   SPI_0,
   SPI_1,
   NUM_SPI
} SPI_Dev_E;


typedef enum
{
   SPI_SUCCESS = 0,
   SPI_XFER_FAILED,
   SPI_INVALID_DEVICE,
   SPI_INVALID_PARAMETER,
   SPI_NOT_INITIALIZED,
   SPI_CLK_FAILED,
   SPI_CLK_UNSUPPORTED,
   SPI_DMA_SETUP_FAILED,
   SPI_DMA_XFER_FAILED
} SPI_Error_E;


// Clock idle priority
typedef enum
{
   SPI_CLK_POLARITY_LOW,
   SPI_CLK_POLARITY_HIGH
} SPI_ClkPolarity_E;


// Clock phase of the bus
typedef enum
{
   SPI_PHASE_FIRST_EDGE,
   SPI_PHASE_SECOND_EDGE
} SPI_ClkPhase_E;


///////////////////////////////////////////////////////////////////////////////
//
// Types
//
///////////////////////////////////////////////////////////////////////////////
typedef uint32_t SPI_ClkDiv_T;
typedef void (*SPI_Callback_T)(SPI_Error_E);

///////////////////////////////////////////////////////////////////////////////
//
// Initializes the specified SPI interface
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E SPI_Init(SPI_Dev_E device, SPI_ClkDiv_T pClkDiv, SPI_ClkDiv_T busClkDiv,
                     SPI_ClkPolarity_E clkPol, SPI_ClkPhase_E clkPhase, size_t xferSizeBits);


///////////////////////////////////////////////////////////////////////////////
//
// Initializes the DMA interface for the SPI device
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E SPI_DMA_Init(SPI_Dev_E device);


///////////////////////////////////////////////////////////////////////////////
//
// Single (non-DMA) transaction. The source and destination should be able to hold
// at least the number of bits of each transaction.
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E SPI_SingleTransaction(SPI_Dev_E device, const void *src, void *dest);


///////////////////////////////////////////////////////////////////////////////
//
//
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E SPI_BeginDMA_Transaction(SPI_Dev_E device, const void *src, void *dest,
                                     size_t size, SPI_Callback_T txCallback, SPI_Callback_T rxCallback);


#endif //__SPI_FACADE_H__
