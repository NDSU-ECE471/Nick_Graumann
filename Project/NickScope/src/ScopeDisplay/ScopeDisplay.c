#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "LcdDisplay.h"
#include "ScopeDisplay.h"

#define SCOPE_DISPLAY_EVENT_QUEUE_SIZE 10
static xQueueHandle eventQueue = NULL;

static void ScopeDisplayDrawBorder();
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

   ScopeDisplayDrawBorder();

   return retVal;
}


void ScopeDisplayQueueEvent(ScopeDisplayEvent_T *event)
{
   xQueueSendToBack(eventQueue, event, 0);
}


static void ScopeDisplayDrawBorder()
{
   LcdDrawHLine(TRACE_AREA_X, TRACE_AREA_Y, TRACE_AREA_WIDTH, TRACE_BORDER_COLOR);
   LcdDrawHLine(TRACE_AREA_X, TRACE_AREA_Y+TRACE_AREA_HEIGHT-1, TRACE_AREA_WIDTH, TRACE_BORDER_COLOR);
   LcdDrawVLine(TRACE_AREA_X, TRACE_AREA_Y, TRACE_AREA_HEIGHT, TRACE_BORDER_COLOR);
   LcdDrawVLine(TRACE_AREA_X+TRACE_AREA_WIDTH-1, TRACE_AREA_Y, TRACE_AREA_HEIGHT, TRACE_BORDER_COLOR);
}


static void ScopeDisplayDrawInfo()
{
   setColor16(LCD_COLOR_WHITE);
   drawString(LOWER_STATUS_X, LOWER_STATUS_Y, "2V/div  10ms/div");
}


static void ScopeDisplayDrawTrace(int32_t adcReading, LcdCoord *traceXPos, LcdCoord *traceLevelPixels, LcdCoord *prevTraceLevelPixels)
{
   // Convert ADC reading to pixels
   *traceLevelPixels = TRACE_AREA_Y + ((TRACE_AREA_HEIGHT-2*TRACE_BORDER_THICKNESS-1)
         - (adcReading * (TRACE_AREA_HEIGHT-2*TRACE_BORDER_THICKNESS-1) / 4096));

   // Erase previous data
   LcdDrawVLine(*traceXPos, TRACE_AREA_Y+TRACE_BORDER_THICKNESS, TRACE_AREA_HEIGHT-2*TRACE_BORDER_THICKNESS, DISPLAY_BG_COLOR);

   // If the slope is increasing (display coordinates are inverted; 0 is the highest)
   if(*traceLevelPixels < *prevTraceLevelPixels)
   {
      // Height is always one less that way the lowest pixel isn't right next to the previous one
      LcdDrawVLine(*traceXPos, *traceLevelPixels, *prevTraceLevelPixels - *traceLevelPixels, TRACE_COLOR);
   }
   // If the slope is decreasing
   else if(*traceLevelPixels > *prevTraceLevelPixels)
   {
      // Also don't want the lowest pixel next to the previous here, so decrease the height and lower by 1
      LcdDrawVLine(*traceXPos, *prevTraceLevelPixels + 1, *traceLevelPixels - *prevTraceLevelPixels, TRACE_COLOR);
   }
   // Unchanged
   else
   {
      LcdDrawPixel(*traceXPos, *traceLevelPixels, TRACE_COLOR);
   }

   *prevTraceLevelPixels = *traceLevelPixels;

   if(++*traceXPos >= TRACE_AREA_WIDTH-TRACE_BORDER_THICKNESS)
   {
      *traceXPos = TRACE_AREA_X+TRACE_BORDER_THICKNESS;
   }
}


static portTASK_FUNCTION(ScopeDisplayTask, pvParameters)
{
   ScopeDisplayEvent_T event;
   LcdCoord traceXPos = TRACE_AREA_X+TRACE_BORDER_THICKNESS;
   LcdCoord traceLevelPixels;
   LcdCoord prevTraceLevelPixels = LCD_HEIGHT/2;

   while(1)
   {
      if(xQueueReceive(eventQueue, &event, portMAX_DELAY))
      {
         switch(event.type)
         {
         case SCOPE_DISPLAY_EVENT_DRAW_TRACE:
            ScopeDisplayDrawTrace(event.adcReading, &traceXPos, &traceLevelPixels, &prevTraceLevelPixels);
            break;

         case SCOPE_DISPLAY_EVENT_DRAW_INFO:
            ScopeDisplayDrawInfo();
            break;
         }
      }
   }
}

