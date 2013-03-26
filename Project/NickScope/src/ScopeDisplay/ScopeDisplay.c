#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "LcdDisplay.h"
#include "ScopeDisplay.h"

#define SCOPE_DISPLAY_EVENT_QUEUE_SIZE 10
static xQueueHandle eventQueue = NULL;

static portTASK_FUNCTION(ScopeDisplayTask, pvParameters);


bool ScopeDisplayInit()
{
   bool retVal = xTaskCreate(ScopeDisplayTask,
                             SCOPE_DISPLAY_TASK_NAME,
                             SCOPE_DISPLAY_TASK_STACK,
                             NULL,
                             SCOPE_DISPLAY_TASK_PRIORITY,
                             NULL);

   eventQueue = xQueueCreate(SCOPE_DISPLAY_EVENT_QUEUE_SIZE, sizeof(ScopeDisplayEvent_T));
   retVal = retVal && eventQueue;

   lcd_init();
   fillScreen(DISPLAY_BG_COLOR);

   return retVal;
}


void ScopeDisplayQueueEvent(ScopeDisplayEvent_T *event)
{
   xQueueSendToBack(eventQueue, event, 0);
}


static portTASK_FUNCTION(ScopeDisplayTask, pvParameters)
{
   ScopeDisplayEvent_T event;
   LcdCoord traceXPos = 0;
   LcdCoord traceLevelPixels;
   LcdCoord prevTraceLevelPixels = LCD_HEIGHT/2;

   while(1)
   {
      if(xQueueReceive(eventQueue, &event, portMAX_DELAY))
      {
         switch(event.type)
         {
         case SCOPE_DISPLAY_EVENT_UPDATE:
            traceLevelPixels = (LCD_HEIGHT-1) - (event.adcReading * (LCD_HEIGHT-1) / 4096);

            // Erase previous data
            LcdDrawVLine(traceXPos, 0, LCD_HEIGHT, DISPLAY_BG_COLOR);

            // Poor man's draw line function, essentially connecting the dots

            // If the slope is increasing (display coordinates are inverted; 0 is the highest)
            if(traceLevelPixels < prevTraceLevelPixels)
            {
               // Height is always one less that way the lowest pixel isn't right next to the previous one
               LcdDrawVLine(traceXPos, traceLevelPixels, prevTraceLevelPixels - traceLevelPixels - 1, DISPLAY_TRACE_COLOR);
            }
            // If the slope is decreasing
            else if(traceLevelPixels > prevTraceLevelPixels)
            {
               // Also don't want the lowest pixel next to the previous here, so decrease the height and lower by 1
               LcdDrawVLine(traceXPos, prevTraceLevelPixels + 1, traceLevelPixels - prevTraceLevelPixels - 1, DISPLAY_TRACE_COLOR);
            }
            // Unchanged
            else
            {
               LcdDrawPixel(traceXPos, traceLevelPixels, DISPLAY_TRACE_COLOR);
            }

            prevTraceLevelPixels = traceLevelPixels;

            if(++traceXPos >= LCD_WIDTH)
            {
               traceXPos = 0;
            }
            break;
         }
      }
   }
}

