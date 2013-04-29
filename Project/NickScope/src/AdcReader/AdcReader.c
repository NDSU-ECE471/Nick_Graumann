#include <string.h>

#ifdef USE_INTERNAL_ADC
   #include "lpc17xx_adc.h"
   #include "lpc17xx_pinsel.h"
#endif
#include <cr_section_macros.h>

#include "FreeRTOS.h"
#include "task.h"

#include "AdcReader.h"
#include "Drivers/SPI/SPI_Facade.h"
#include "ScopeDisplay/ScopeDisplay.h"


#ifdef USE_INTERNAL_ADC
   #define ADC_RATE     10000
   #define ADC_CHANNEL  0

static void AdcInitialize()
{
   ADC_Init(LPC_ADC, ADC_RATE);
   ADC_ChannelCmd(LPC_ADC, ADC_CHANNEL, ENABLE);
}
#else
   #define SPI_ADC_DEV        SPI_0
   #define SPI_ADC_PCLK_DIV   1
   #define SPI_ADC_BUS_DIV    2
   #define SPI_ADC_BUS_POL    SPI_CLK_POLARITY_HIGH
   #define SPI_ADC_BUS_PHASE  SPI_PHASE_FIRST_EDGE
   #define SPI_ADC_XFER_SIZE  16
   #define SPI_ADC_BIT_OFFSET 6
   #define SPI_ADC_BIT_MASK   0xFF


volatile static uint16_t SampleBuffer[8192] __BSS(RamAHB32) __attribute__((aligned (256)));
#endif

static portTASK_FUNCTION(AdcReaderTask, pvParameters);

static AdcReaderCommand_E State = ADC_READER_READ_CONTINUOUS;

void AdcCallback(SPI_Error_E err, SPI_Dev_E dev, const void *src, void *dest, size_t size)
{
}


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
   SPI_Init(SPI_ADC_DEV,
            SPI_ADC_PCLK_DIV,
            SPI_ADC_BUS_DIV,
            SPI_ADC_BUS_POL,
            SPI_ADC_BUS_PHASE,
            SPI_ADC_XFER_SIZE);

   SPI_DMA_Init(SPI_ADC_DEV);

   memset((void *)SampleBuffer, 0, sizeof(SampleBuffer));
#endif

   return retVal;
}


static portTASK_FUNCTION(AdcReaderTask, pvParameters)
{
   ScopeDisplayEvent_T displayEvent;
   int32_t adcReading = -1;
   portTickType lastWakeTime = xTaskGetTickCount();

   while(1)
   {
      switch(State)
      {
      case ADC_READER_STOP:
         break;

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

         ADC_StartCmd(LPC_ADC, ADC_START_NOW);
#else
         //SPI_SingleTransaction(SPI_ADC_DEV, NULL, &SampleBuffer[0]);
         //adcReading = (int32_t)((SampleBuffer[0] >> SPI_ADC_BIT_OFFSET) & SPI_ADC_BIT_MASK);

         SPI_BeginDMA_Transaction(SPI_ADC_DEV, NULL, (void *)SampleBuffer, 1024, &AdcCallback);
         State = ADC_READER_STOP;
#endif

         if(adcReading != -1)
         {
            displayEvent.type = SCOPE_DISPLAY_EVENT_UPDATE_TRACE;
            displayEvent.adcReading = adcReading;
            ScopeDisplayQueueEvent(&displayEvent);
         }

         vTaskDelayUntil(&lastWakeTime, ADC_READER_TASK_DELAY_TICKS);
         break;

      case ADC_READER_READ_BURST:

         break;
      }
   }
}

