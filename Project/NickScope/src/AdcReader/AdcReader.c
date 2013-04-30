#include <string.h>

#ifdef USE_INTERNAL_ADC
   #include "lpc17xx_adc.h"
   #include "lpc17xx_pinsel.h"
#endif
#include <cr_section_macros.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "AdcReader.h"
#include "Drivers/SPI/SPI_Facade.h"
#include "ScopeDisplay/ScopeDisplay.h"


#define SPI_ADC_DEV        SPI_0
#define SPI_ADC_PCLK_DIV   1
#define SPI_ADC_BUS_DIV    2
#define SPI_ADC_BUS_POL    SPI_CLK_POLARITY_HIGH
#define SPI_ADC_BUS_PHASE  SPI_PHASE_FIRST_EDGE
#define SPI_ADC_XFER_SIZE  16
#define SPI_ADC_BIT_OFFSET 6
#define SPI_ADC_BIT_MASK   0xFF

volatile uint16_t SampleBuffer[ADC_READER_BUF_LEN] __BSS(RamAHB32) __attribute__((aligned (256)));


static portTASK_FUNCTION(AdcReaderTask, pvParameters);

#define ADC_READER_EVENT_QUEUE_SIZE    10
static xQueueHandle eventQueue = NULL;
static AdcReaderCommand_E State = ADC_READER_STOP;

static void AdcCallback(SPI_Error_E err)
{
   if(SPI_SUCCESS == err)
   {
      State = ADC_READER_STOP;

      ScopeDisplayEvent_T displayEvent;
      displayEvent.type = SCOPE_DISPLAY_EVENT_DRAW_TRACE;
      displayEvent.AdcMemory.data = SampleBuffer;
      displayEvent.AdcMemory.size = sizeof(SampleBuffer)/sizeof(SampleBuffer[0]);
      ScopeDisplayQueueEvent(&displayEvent);
   }
}


bool AdcReaderInit()
{
   bool retVal = xTaskCreate(AdcReaderTask,
                             ADC_READER_TASK_NAME,
                             ADC_READER_TASK_STACK,
                             NULL,
                             ADC_READER_TASK_PRIORITY,
                             NULL);

   eventQueue = xQueueCreate(ADC_READER_EVENT_QUEUE_SIZE, sizeof(AdcReaderCommand_T));
   retVal = retVal && eventQueue;

   retVal = retVal && (SPI_Init(SPI_ADC_DEV,
                                SPI_ADC_PCLK_DIV,
                                SPI_ADC_BUS_DIV,
                                SPI_ADC_BUS_POL,
                                SPI_ADC_BUS_PHASE,
                                SPI_ADC_XFER_SIZE) == SPI_SUCCESS);

   retVal = retVal && (SPI_DMA_Init(SPI_ADC_DEV) == SPI_SUCCESS);

   memset((void *)SampleBuffer, 0, sizeof(SampleBuffer));

   return retVal;
}


void AdcReaderQueueEvent(AdcReaderCommand_T *event)
{
   xQueueSendToBack(eventQueue, event, 0);
}


uint8_t AdcTrimSampleData(AdcCounts_T data)
{
   return (uint8_t)((data >> SPI_ADC_BIT_OFFSET) & SPI_ADC_BIT_MASK);
}


static portTASK_FUNCTION(AdcReaderTask, pvParameters)
{
   (void)pvParameters;

   AdcReaderCommand_T command;
   ScopeDisplayEvent_T displayEvent;
   AdcCounts_T adcReading = -1;
   portTickType lastWakeTime = xTaskGetTickCount();

   while(1)
   {
      if(xQueueReceive(eventQueue, &command, 0))
      {
         if(ADC_READER_SAMPLING != State)
         {
            State = command.type;
         }
      }

      switch(State)
      {
      case ADC_READER_STOP:
      case ADC_READER_SAMPLING:
         break;

      case ADC_READER_READ_SINGLE:
      case ADC_READER_READ_CONTINUOUS:
         SPI_SingleTransaction(SPI_ADC_DEV, NULL, (void *)&SampleBuffer[0]);
         adcReading = AdcTrimSampleData(SampleBuffer[0]);

         if(ADC_READER_READ_CONTINUOUS == State)
         {
            displayEvent.type = SCOPE_DISPLAY_EVENT_UPDATE_TRACE;
            displayEvent.adcReading = adcReading;
            ScopeDisplayQueueEvent(&displayEvent);
         }
         else
         {
            State = ADC_READER_STOP;
         }

         vTaskDelayUntil(&lastWakeTime, ADC_READER_TASK_DELAY_TICKS);
         break;

      case ADC_READER_READ_BURST:
         SPI_BeginDMA_Transaction(SPI_ADC_DEV, NULL, (void *)SampleBuffer, ADC_READER_BUF_LEN, NULL, &AdcCallback);
         State = ADC_READER_SAMPLING;
         break;
      }
   }
}

