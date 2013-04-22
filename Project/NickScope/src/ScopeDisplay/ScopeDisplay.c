#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "LcdDisplay.h"
#include "ScopeDisplay.h"

#define SCOPE_DISPLAY_EVENT_QUEUE_SIZE 10
static xQueueHandle eventQueue = NULL;

static void DrawBorder();
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

   DrawBorder();

   return retVal;
}


void ScopeDisplayQueueEvent(ScopeDisplayEvent_T *event)
{
   xQueueSendToBack(eventQueue, event, 0);
}


static LcdCoord AdcReadingToPixels(AdcCounts_T adcReading)
{
   return ((TRACE_AREA_HEIGHT-1) - (adcReading * (TRACE_AREA_HEIGHT-1) / ADC_MAX_COUNTS));
}


static void DrawBorder()
{
#if TRACE_BORDER_THICKNESS == 2
   // Draw top and bottom borders
   LcdDrawHLine(TRACE_OUTER_X, TRACE_OUTER_Y, TRACE_OUTER_WIDTH, TRACE_BORDER_COLOR);
   LcdDrawHLine(TRACE_OUTER_X, TRACE_OUTER_Y+TRACE_OUTER_HEIGHT-1, TRACE_OUTER_WIDTH, TRACE_BORDER_COLOR);

   // Draw horizontal tick marks
   for(LcdCoord hx=TRACE_TICK_INTEVAL_H; hx<=TRACE_OUTER_WIDTH/2; hx+=TRACE_TICK_INTEVAL_H)
   {
      // Marks from left to center
      LcdDrawPixel(TRACE_OUTER_X+1+hx, TRACE_OUTER_Y+1,                          TRACE_BORDER_COLOR);
      LcdDrawPixel(TRACE_OUTER_X+1+hx, TRACE_OUTER_Y+(TRACE_OUTER_HEIGHT-1)-1,   TRACE_BORDER_COLOR);

      // Marks from right to center
      LcdDrawPixel(TRACE_OUTER_WIDTH-hx, TRACE_OUTER_Y+1,                        TRACE_BORDER_COLOR);
      LcdDrawPixel(TRACE_OUTER_WIDTH-hx, TRACE_OUTER_Y+(TRACE_OUTER_HEIGHT-1)-1, TRACE_BORDER_COLOR);
   }

   // Draw vertical tick marks
   for(LcdCoord vx=TRACE_TICK_INTEVAL_V; vx<=TRACE_OUTER_HEIGHT/2; vx+=TRACE_TICK_INTEVAL_V)
   {
      // Marks from left to center
      LcdDrawPixel(TRACE_OUTER_X+1,                         TRACE_OUTER_Y+1+vx, TRACE_BORDER_COLOR);
      LcdDrawPixel(TRACE_OUTER_X+(TRACE_OUTER_WIDTH-1)-1,   TRACE_OUTER_Y+1+vx, TRACE_BORDER_COLOR);

      // Marks from right to center
      LcdDrawPixel(TRACE_OUTER_X+1,                         TRACE_OUTER_HEIGHT-vx, TRACE_BORDER_COLOR);
      LcdDrawPixel(TRACE_OUTER_X+(TRACE_OUTER_WIDTH-1)-1,   TRACE_OUTER_HEIGHT-vx, TRACE_BORDER_COLOR);
   }

   // Draw left and right borders
   LcdDrawVLine(TRACE_OUTER_X, TRACE_OUTER_Y, TRACE_OUTER_HEIGHT, TRACE_BORDER_COLOR);
   LcdDrawVLine(TRACE_OUTER_X+TRACE_OUTER_WIDTH-1, TRACE_OUTER_Y, TRACE_OUTER_HEIGHT, TRACE_BORDER_COLOR);
#else
   #error Update DrawBorder() for border thickness != 2
#endif
}


static void DrawTimebase(TimebaseValue_T time, TimebaseUnits_E units)
{
   char buffer[16];
   switch(units)
   {
   case TIMEBASE_nS:
      snprintf(buffer, sizeof(buffer), "%uns/div", time);
      break;

   case TIMEBASE_uS:
      snprintf(buffer, sizeof(buffer), "%uus/div", time);
      break;

   case TIMEBASE_mS:
      snprintf(buffer, sizeof(buffer), "%ums/div", time);
      break;

   case TIMEBASE_S:
      snprintf(buffer, sizeof(buffer), "%us/div", time);
      break;

   default:
      buffer[0] = '\0';
   }

   setColor16(LCD_COLOR_WHITE);
   drawString(LOWER_STATUS_X+LOWER_STATUS_WIDTH-DISPLAY_TEXT_WIDTH*strlen(buffer), LOWER_STATUS_Y, buffer);
}


static void DrawVdiv(V_DivValue_T vdiv, V_DivUnits_E units)
{
   char buffer[16];
   switch(units)
   {
   case VDIV_mV:
      snprintf(buffer, sizeof(buffer), "%umV/div", vdiv);
      break;

   case VDIV_V:
      snprintf(buffer, sizeof(buffer), "%uV/div", vdiv);
      break;

   default:
      buffer[0] = '\0';
   }

   setColor16(LCD_COLOR_WHITE);
   drawString(LOWER_STATUS_X, LOWER_STATUS_Y, buffer);
}


static void DrawTraceLine(LcdCoord traceXPos, LcdCoord traceLevelPixels, LcdCoord prevTraceLevelPixels)
{
   // Erase previous data
   LcdDrawVLine(traceXPos, TRACE_AREA_Y, TRACE_AREA_HEIGHT, DISPLAY_BG_COLOR);

   // If the slope is increasing (display coordinates are inverted; 0 is the highest)
   if(traceLevelPixels < prevTraceLevelPixels)
   {
      // Height is always one less that way the lowest pixel isn't right next to the previous one
      LcdDrawVLine(traceXPos, TRACE_AREA_Y + traceLevelPixels, prevTraceLevelPixels - traceLevelPixels, TRACE_COLOR);
   }
   // If the slope is decreasing
   else if(traceLevelPixels > prevTraceLevelPixels)
   {
      // Also don't want the lowest pixel next to the previous here, so decrease the height and lower by 1
      LcdDrawVLine(traceXPos, TRACE_AREA_Y + prevTraceLevelPixels + 1, traceLevelPixels - prevTraceLevelPixels, TRACE_COLOR);
   }
   // Unchanged
   else
   {
      LcdDrawPixel(traceXPos, TRACE_AREA_Y + traceLevelPixels, TRACE_COLOR);
   }
}


static portTASK_FUNCTION(ScopeDisplayTask, pvParameters)
{
   ScopeDisplayEvent_T event;
   LcdCoord traceXPos = TRACE_AREA_X;
   LcdCoord traceLevelPixels;
   LcdCoord prevTraceLevelPixels = TRACE_LEVEL_INVALID;

   while(1)
   {
      if(xQueueReceive(eventQueue, &event, portMAX_DELAY))
      {
         switch(event.type)
         {
         case SCOPE_DISPLAY_EVENT_UPDATE_TRACE:
            traceLevelPixels = AdcReadingToPixels(event.adcReading);

            if(prevTraceLevelPixels == TRACE_LEVEL_INVALID)
            {
               prevTraceLevelPixels = traceLevelPixels;
            }

            DrawTraceLine(traceXPos, traceLevelPixels, prevTraceLevelPixels);

            if(++traceXPos >= TRACE_AREA_X+TRACE_AREA_WIDTH)
            {
               traceXPos = TRACE_AREA_X;
            }
            prevTraceLevelPixels = traceLevelPixels;
            break;

         case SCOPE_DISPLAY_EVENT_DRAW_TRACE:
            // todo: draw whole trace at once from shared memory
            break;

         case SCOPE_DISPLAY_EVENT_UPDATE_TIMEBASE:
            DrawTimebase(event.TimebaseData.value, event.TimebaseData.units);
            break;

         case SCOPE_DISPLAY_EVENT_UPDATE_VDIV:
            DrawVdiv(event.VdivData.value, event.VdivData.units);
            break;
         }
      }
   }
}

