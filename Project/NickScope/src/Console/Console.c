#include "FreeRTOS.h"
#include "task.h"

#include "AdcReader/AdcReader.h"
#include "Console.h"
#include "ScopeDisplay/ScopeDisplay.h"


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

   return retVal;
}


static portTASK_FUNCTION(ConsoleTxTask, pvParameters)
{
   (void)pvParameters;

   char buf = 0;
   AdcReaderCommand_T adcCommand;
   while(1)
   {
      UART_Rx(CONSOLE_UART_PORT, &buf, sizeof(buf), UART_BLOCKING);

      adcCommand.type = ADC_READER_READ_BURST;
      AdcReaderQueueEvent(&adcCommand);

      UART_Tx(CONSOLE_UART_PORT, &buf, sizeof(buf), UART_NON_BLOCKING);
   }
}

