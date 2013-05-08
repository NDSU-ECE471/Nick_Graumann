#include <string.h>
#include <cr_section_macros.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "AdcReader.h"
#include "Drivers/SPI/SPI_Facade.h"
#include "ScopeDisplay/ScopeDisplay.h"


///////////////////////////////////////////////////////////////////////////////
//
// Local definitions
//
///////////////////////////////////////////////////////////////////////////////
#define SPI_ADC_DEV           SPI_0
#define SPI_ADC_PCLK_DIV      1
#define SPI_ADC_BUS_DIV       2
#define SPI_ADC_BUS_POL       SPI_CLK_POLARITY_HIGH
#define SPI_ADC_BUS_PHASE     SPI_PHASE_FIRST_EDGE
#define SPI_ADC_XFER_SIZE     14
#define SPI_ADC_BIT_OFFSET    4
#define SPI_ADC_BIT_MASK      0xFF

#define SPI_ADC_BUS_MIN_DIV   2
#define SPI_ADC_BUS_MAX_DIV   16384
#define SPI_ADC_BUS_DIV_INC   2


#define ADC_READER_TASK_TIMEOUT_TICKS  1000
#define ADC_READER_EVENT_QUEUE_SIZE    10


///////////////////////////////////////////////////////////////////////////////
//
// Local Variables
//
///////////////////////////////////////////////////////////////////////////////
volatile AdcCounts_T SampleBuffer[ADC_READER_BUF_LEN] __BSS(RamAHB32) __attribute__((aligned (256)));
#define SAMPLE_BUFFER_LEN  (sizeof(SampleBuffer)/sizeof(SampleBuffer[0]));

static xSemaphoreHandle AdcSampleMutex = NULL;
static xQueueHandle AdcEventQueue = NULL;
static AdcReaderCommand_E State = ADC_READER_STOP;
static SPI_ClkDiv_T ADC_SPI_BusClkDivVal = SPI_ADC_BUS_DIV;

static portTASK_FUNCTION(AdcReaderTask, pvParameters);


///////////////////////////////////////////////////////////////////////////////
//
// Called once the ADC has finished gathering samples in burst mode.
// Triggers a ScopeDisplay event to redraw the entire trace.
//
///////////////////////////////////////////////////////////////////////////////
static void AdcCallback(SPI_Error_E err)
{
   if(SPI_SUCCESS == err)
   {
      ScopeDisplayEvent_T displayEvent;
      displayEvent.type = SCOPE_DISPLAY_EVENT_DRAW_TRACE;
      displayEvent.AdcMemory.data = SampleBuffer;
      displayEvent.AdcMemory.length = ADC_READER_BUF_LEN;
      displayEvent.AdcMemory.mutex = AdcSampleMutex;
      ScopeDisplayQueueEvent(&displayEvent);
   }

   State = ADC_READER_STOP;

   signed portBASE_TYPE taskWoken = pdFALSE;
   xSemaphoreGiveFromISR(AdcSampleMutex, &taskWoken);

   portEND_SWITCHING_ISR(taskWoken);
}


///////////////////////////////////////////////////////////////////////////////
//
// See AdcReader.h
//
///////////////////////////////////////////////////////////////////////////////
bool AdcReaderInit()
{
   bool retVal = xTaskCreate(AdcReaderTask,
                             ADC_READER_TASK_NAME,
                             ADC_READER_TASK_STACK,
                             NULL,
                             ADC_READER_TASK_PRIORITY,
                             NULL);

   AdcEventQueue = xQueueCreate(ADC_READER_EVENT_QUEUE_SIZE, sizeof(AdcReaderCommand_T));
   retVal = retVal && AdcEventQueue;

   AdcSampleMutex = xSemaphoreCreateMutex();
   retVal = retVal && AdcSampleMutex;

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


///////////////////////////////////////////////////////////////////////////////
//
// See AdcReader.h
//
///////////////////////////////////////////////////////////////////////////////
void AdcReaderQueueEvent(AdcReaderCommand_T *event)
{
   xQueueSendToBack(AdcEventQueue, event, 0);
}


///////////////////////////////////////////////////////////////////////////////
//
// See AdcReader.h
//
///////////////////////////////////////////////////////////////////////////////
void AdcReaderGetSampleBuffer(volatile AdcCounts_T **bufPtr, size_t *length)
{
   if(bufPtr)
   {
      *bufPtr = SampleBuffer;
   }

   if(length)
   {
      *length = SAMPLE_BUFFER_LEN;
   }
}


///////////////////////////////////////////////////////////////////////////////
//
// See AdcReader.h
//
///////////////////////////////////////////////////////////////////////////////
uint8_t AdcTrimSampleData(AdcCounts_T data)
{
   return (uint8_t)((data >> SPI_ADC_BIT_OFFSET) & SPI_ADC_BIT_MASK);
}



///////////////////////////////////////////////////////////////////////////////
//
// FreeRTOS task
//
///////////////////////////////////////////////////////////////////////////////
static portTASK_FUNCTION(AdcReaderTask, pvParameters)
{
   (void)pvParameters;

   AdcReaderCommand_T command;
   ScopeDisplayEvent_T displayEvent;
   AdcCounts_T adcReading;
   portTickType lastWakeTime = xTaskGetTickCount();

   while(1)
   {
      if(xQueueReceive(AdcEventQueue, &command, ADC_READER_TASK_TIMEOUT_TICKS))
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
         SPI_SingleTransaction(SPI_ADC_DEV, NULL, (void *)&adcReading);
         adcReading = AdcTrimSampleData(adcReading);

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
         if(xSemaphoreTake(AdcSampleMutex, ADC_SAMPLE_MUTEX_TIMEOUT))
         {
            // DMA transaction setup occurs inside a critical section just in case someone else also wants to set one up too.
            taskENTER_CRITICAL();
            SPI_BeginDMA_Transaction(SPI_ADC_DEV, NULL, (void *)SampleBuffer, ADC_READER_BUF_LEN, NULL, &AdcCallback);
            taskEXIT_CRITICAL();

            State = ADC_READER_SAMPLING;

            // The DMA operation should be complete long before this timeout.
            vTaskDelay(ADC_READER_TASK_TIMEOUT_TICKS);

            // Check if it's done. If not, don't know what happened, but for some reason we're not done....
            // Just pretend we're finished so we don't get stuck in an infinte sampling state.
            if(ADC_READER_SAMPLING == State)
            {
               State = ADC_READER_STOP;
            }

            // Do not give the sample mutex; it is given when the callback signals the transaction is complete.
         }
         break;

      case ADC_READER_INC_SAMPLERATE:
         if(ADC_SPI_BusClkDivVal > SPI_ADC_BUS_MIN_DIV)
         {
            ADC_SPI_BusClkDivVal /= SPI_ADC_BUS_DIV_INC;
            SPI_SetBusClkDiv(SPI_ADC_DEV, ADC_SPI_BusClkDivVal);
         }

         State = ADC_READER_STOP;
         break;

      case ADC_READER_DEC_SAMPLERATE:
         if(ADC_SPI_BusClkDivVal < SPI_ADC_BUS_MAX_DIV)
         {
            ADC_SPI_BusClkDivVal *= SPI_ADC_BUS_DIV_INC;
            SPI_SetBusClkDiv(SPI_ADC_DEV, ADC_SPI_BusClkDivVal);
         }

         State = ADC_READER_STOP;
         break;
      }

      // Allow other tasks to run if necessary
      portYIELD();
   }
}

