#ifndef __SPI_FACADE_H__
#define __SPI_FACADE_H__

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
   SPI_INVALID_PARAMETER
} SPI_Error_E;


SPI_Error_E SPI_Init(SPI_Dev_E device);

#endif //__SPI_FACADE_H__
