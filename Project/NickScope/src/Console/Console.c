#include "FreeRTOS.h"
#include "task.h"

#include "AdcReader/AdcReader.h"
#include "Console.h"
#include "ScopeDisplay/ScopeDisplay.h"


static const char BREAK_STR[] = "\r";
#define BREAK_STR_LEN         sizeof(BREAK_STR)

static const char USER_PROMPT_STR[] = ">> ";
#define USER_PROMPT_STR_LEN   sizeof(USER_PROMPT_STR)


static portTASK_FUNCTION(ConsoleTxTask, pvParameters);


bool ConsoleInit()
{
   ScopeDisplayEvent_T displayEvent;
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

   uint8_t rxChr = 0;
   AdcReaderCommand_T adcCommand;
   ScopeDisplayEvent_T scopeDispCmd;

   while(1)
   {
      UART_Rx(CONSOLE_UART_PORT, &rxChr, sizeof(rxChr), UART_BLOCKING);

      switch(rxChr)
      {
      case ' ':
         adcCommand.type = ADC_READER_READ_BURST;
         AdcReaderQueueEvent(&adcCommand);
         break;

      case 'i':
      case 'I':
         scopeDispCmd.type = SCOPE_DISPLAY_EVENT_TIMEBASE_DEC;
         ScopeDisplayQueueEvent(&scopeDispCmd);

         adcCommand.type = ADC_READER_INC_SAMPLERATE;
         AdcReaderQueueEvent(&adcCommand);
         break;

      case 'o':
      case 'O':
         scopeDispCmd.type = SCOPE_DISPLAY_EVENT_TIMEBASE_INC;
         ScopeDisplayQueueEvent(&scopeDispCmd);

         adcCommand.type = ADC_READER_DEC_SAMPLERATE;
         AdcReaderQueueEvent(&adcCommand);
         break;
      }

      // Send back the character we received first
      UART_Tx(CONSOLE_UART_PORT, &rxChr, sizeof(rxChr), UART_BLOCKING);
      UART_Tx(CONSOLE_UART_PORT, BREAK_STR, BREAK_STR_LEN, UART_BLOCKING);
      UART_Tx(CONSOLE_UART_PORT, USER_PROMPT_STR, USER_PROMPT_STR_LEN, UART_BLOCKING);

      vTaskDelay(10);
   }
}

