#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "Types.h"
#include "Drivers/UART/UART_Facade.h"

#define CONSOLE_TX_TASK_NAME        (signed char *)"ConsoleTx"
#define CONSOLE_TX_TASK_STACK       128
#define CONSOLE_TX_TASK_PRIORITY    (tskIDLE_PRIORITY+1)
#define CONSOLE_TX_TASK_DELAY_TICKS (10/portTICK_RATE_MS)

#define CONSOLE_UART_PORT  UART_1
#define CONSOLE_UART_BAUD  UART_BAUD_9600

bool ConsoleInit();

#endif //__CONSOLE_H__
