#ifndef __UART_FACADE_H__
#define __UART_FACADE_H__

typedef enum
{
   UART_0,
   UART_1,
   NUM_UARTS
} UART_Dev_E;


typedef enum
{
   UART_SUCCESS = 0,
   UART_XFER_FAILED,
   UART_INVALID_DEVICE
} UART_Error_E;


typedef enum
{
   UART_BAUD_9600 = 9600
} UART_BaudRate;


typedef enum
{
   UART_NON_BLOCKING,
   UART_BLOCKING
} UART_BlockType_E;

UART_Error_E UART_Open(UART_Dev_E device, UART_BaudRate baud);
UART_Error_E UART_Close(UART_Dev_E device);

// Non-DMA transmit and receive
UART_Error_E UART_Rx(UART_Dev_E device, void *buf, uint32_t size, UART_BlockType_E shouldBlock);
UART_Error_E UART_Tx(UART_Dev_E device, const void *buf, uint32_t size, UART_BlockType_E shouldBlock);

#endif //__UART_FACADE_H__
