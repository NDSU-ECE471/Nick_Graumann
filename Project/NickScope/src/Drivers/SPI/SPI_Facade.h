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
   SPI_NOT_INITIALIZED
} SPI_Error_E;


typedef uint32_t SPI_ClkRate_T;


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
// Initializes the specified SPI interface
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E SPI_Init(SPI_Dev_E device);


///////////////////////////////////////////////////////////////////////////////
//
// Changes an interface's configuration. The interface should be initialized.
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E SPI_Config(SPI_Dev_E device, SPI_ClkRate_T clkRate, SPI_ClkPolarity_E clkPol, SPI_ClkPhase_E clkPhase);


///////////////////////////////////////////////////////////////////////////////
//
// Multiple-byte (non-DMA) transaction, split up based on the transfer size configured
// when the peripheral was initialized. Size should be a multiple of that size.
//
///////////////////////////////////////////////////////////////////////////////
SPI_Error_E SPI_MultiByteTransaction(SPI_Dev_E device, void *src, void *dest, size_t size);


#endif //__SPI_FACADE_H__
