#ifndef __SCOPE_DISPLAY_H__
#define __SCOPE_DISPLAY_H__

#include "FreeRTOS.h"
#include "LcdDisplay.h"
#include "Types.h"

#define DISPLAY_BG_COLOR         LCD_COLOR_BLACK

#define DISPALY_TEXT_HEIGHT      7

#define LOWER_STATUS_COLOR       LCD_COLOR_YELLOW
#define LOWER_STATUS_X           0
#define LOWER_STATUS_Y           (LCD_HEIGHT-LOWER_STATUS_HEIGHT)
#define LOWER_STATUS_WIDTH       LCD_WIDTH
#define LOWER_STATUS_HEIGHT      (DISPALY_TEXT_HEIGHT+1)

#define TRACE_BORDER_COLOR       LCD_COLOR_WHITE
#define TRACE_BORDER_THICKNESS   1
#define TRACE_COLOR              LCD_COLOR_YELLOW
#define TRACE_AREA_X             0
#define TRACE_AREA_Y             0
#define TRACE_AREA_WIDTH         (LCD_WIDTH-2*TRACE_AREA_X)
#define TRACE_AREA_HEIGHT        (LOWER_STATUS_Y-1)

#define SCOPE_DISPLAY_TASK_NAME     (signed char *)"ScopeDisplay"
#define SCOPE_DISPLAY_TASK_STACK    128
#define SCOPE_DISPLAY_TASK_PRIORITY (tskIDLE_PRIORITY+2)


typedef enum
{
   SCOPE_DISPLAY_EVENT_DRAW_TRACE = 0,
   SCOPE_DISPLAY_EVENT_DRAW_INFO
} ScopeDisplayEventType_E;


typedef struct
{
   ScopeDisplayEventType_E type;

   union
   {
      int32_t adcReading;
   };
} ScopeDisplayEvent_T;


bool ScopeDisplayInit();
void ScopeDisplayQueueEvent(ScopeDisplayEvent_T *event);


#endif //__SCOPE_DISPLAY_H__
