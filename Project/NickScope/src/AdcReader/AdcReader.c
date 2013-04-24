#include "lpc17xx_adc.h"
#include "lpc17xx_pinsel.h"

#include "FreeRTOS.h"
#include "task.h"

#include "AdcReader.h"
#include "ScopeDisplay/ScopeDisplay.h"

#include "Drivers/SPI/LPC/SSP_LPC.h"


#define ADC_RATE     10000
#define ADC_CHANNEL  0


static void AdcInitialize();
static portTASK_FUNCTION(AdcReaderTask, pvParameters);

static AdcReaderCommand_E State = ADC_READER_READ_CONTINUOUS;


bool AdcReaderInit()
{
   bool retVal = xTaskCreate(AdcReaderTask,
                             ADC_READER_TASK_NAME,
                             ADC_READER_TASK_STACK,
                             NULL,
                             ADC_READER_TASK_PRIORITY,
                             NULL);
#ifdef USE_INTERNAL_ADC
   PINSEL_CFG_Type PinCfg;
   PinCfg.Portnum = 0;
   PinCfg.Pinnum = 23;
   PinCfg.Pinmode = PINSEL_PINMODE_TRISTATE;
   PinCfg.Funcnum = 1;
   PinCfg.OpenDrain = PINSEL_PINMODE_NORMAL;
   PINSEL_ConfigPin(&PinCfg);

   AdcInitialize();
#else
   SSP0_Enable();
#endif

   return retVal;
}


static void AdcInitialize()
{
   ADC_Init(LPC_ADC, ADC_RATE);
   ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL, ENABLE);
}


static portTASK_FUNCTION(AdcReaderTask, pvParameters)
{
   ScopeDisplayEvent_T displayEvent;
   uint16_t buf;
   int32_t adcReading = -1;
   portTickType lastWakeTime = xTaskGetTickCount();

   while(1)
   {
      switch(State)
      {
      case ADC_READER_READ_CONTINUOUS:
#ifdef USE_INTERNAL_ADC
         if(ADC_ChannelGetStatus(LPC_ADC, ADC_CHANNEL, ADC_DATA_DONE) == SET)
         {
            adcReading = ADC_ChannelGetData(LPC_ADC, ADC_CHANNEL);
         }
         else
         {
            adcReading = -1;
            ADC_DeInit(LPC_ADC);
            AdcInitialize();
         }


#else
         SSP0_Receive(&buf);
         adcReading = (int32_t)((buf >> 6) & 0xFF);
#endif

         if(adcReading != -1)
         {
            displayEvent.type = SCOPE_DISPLAY_EVENT_UPDATE_TRACE;
            displayEvent.adcReading = adcReading;
            ScopeDisplayQueueEvent(&displayEvent);
         }

         //ADC_StartCmd(LPC_ADC, ADC_START_NOW);

         vTaskDelayUntil(&lastWakeTime, ADC_READER_TASK_DELAY_TICKS);
         break;
      }
   }
}

