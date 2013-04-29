#include "FreeRTOS.h"
#include "task.h"

#include "Console.h"
#include "ScopeDisplay/ScopeDisplay.h"


// todo: tempoarary
#include <stdio.h>
#include <string.h>
#include "Drivers/SPI/LPC/SSP_LPC.h"


static portTASK_FUNCTION(ConsoleTxTask, pvParameters);


bool ConsoleInit()
{
   ScopeDisplayEvent_T displayEvent;
   displayEvent.type = SCOPE_DISPLAY_EVENT_UPDATE_TIMEBASE;
   displayEvent.TimebaseData.value = 200;
   displayEvent.TimebaseData.units = TIMEBASE_mS;
   ScopeDisplayQueueEvent(&displayEvent);

   displayEvent.type = SCOPE_DISPLAY_EVENT_UPDATE_VDIV;
   displayEvent.VdivData.value = 400;
   displayEvent.VdivData.units = VDIV_mV;
   ScopeDisplayQueueEvent(&displayEvent);

   bool retVal = xTaskCreate(ConsoleTxTask,
                             CONSOLE_TX_TASK_NAME,
                             CONSOLE_TX_TASK_STACK,
                             NULL,
                             CONSOLE_TX_TASK_PRIORITY,
                             NULL);

   retVal = retVal && (UART_Open(CONSOLE_UART_PORT, CONSOLE_UART_BAUD) == UART_SUCCESS);

   //SSP0_Enable();

   return retVal;
}


static portTASK_FUNCTION(ConsoleTxTask, pvParameters)
{
   char buf[16];
   uint16_t reading = 0;;
   while(1)
   {
      //SSP0_Receive(&reading);
      snprintf(buf, sizeof(buf), "ADC: %u\r\n", reading);
      UART_Tx(CONSOLE_UART_PORT, buf, strlen(buf), UART_NON_BLOCKING);

      vTaskDelay(100);
   }
}

