#include "lpc17xx_adc.h"
#include "lpc17xx_pinsel.h"

#include "FreeRTOS.h"
#include "task.h"

#include "AdcReader.h"
#include "ScopeDisplay/ScopeDisplay.h"


#define ADC_RATE     1000
#define ADC_CHANNEL  0


static portTASK_FUNCTION(AdcReaderTask, pvParameters);


bool AdcReaderInit()
{
   bool retVal = xTaskCreate(AdcReaderTask,
                             ADC_READER_TASK_NAME,
                             ADC_READER_TASK_STACK,
                             NULL,
                             ADC_READER_TASK_PRIORITY,
                             NULL);

   PINSEL_CFG_Type PinCfg;
   PinCfg.Portnum = 0;
   PinCfg.Pinnum = 23;
   PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
   PinCfg.Funcnum = 1;
   PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
   PINSEL_ConfigPin(&PinCfg);

   ADC_Init(LPC_ADC, ADC_RATE);
   ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL, ENABLE);

   return retVal;
}

int32_t values[] = {0, 800, 1600, 2400, 3200, 4000, 3000, 2000, 1000, 0};
int32_t index=0;


static portTASK_FUNCTION(AdcReaderTask, pvParameters)
{
   ScopeDisplayEvent_T displayEvent;
   int32_t adcReading = -1;
   portTickType lastWakeTime = xTaskGetTickCount();

   while(1)
   {
      if(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL, ADC_DATA_DONE) == SET)
      {
         adcReading = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL);
      }
      else
      {
         adcReading = -1;
      }

      if(adcReading != -1)
      {
         displayEvent.type = SCOPE_DISPLAY_EVENT_DRAW_TRACE;
         displayEvent.adcReading = adcReading;
         //scopeEvent.adcReading = values[index++];
         ScopeDisplayQueueEvent(&displayEvent);
      }

      ADC_StartCmd(LPC_ADC, ADC_START_NOW);

      vTaskDelayUntil(&lastWakeTime, ADC_READER_TASK_DELAY_TICKS);
   }
}

