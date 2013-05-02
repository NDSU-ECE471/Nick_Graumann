#ifndef __SCOPE_DISPLAY_H__
#define __SCOPE_DISPLAY_H__

#include "FreeRTOS.h"
#include "LcdDisplay.h"
#include "Types.h"

#define DISPLAY_BG_COLOR         LCD_COLOR_BLACK

#define DISPLAY_TEXT_HEIGHT      7
#define DISPLAY_TEXT_WIDTH       6

#define LOWER_STATUS_COLOR       LCD_COLOR_YELLOW
#define LOWER_STATUS_X           0
#define LOWER_STATUS_Y           (LCD_HEIGHT-LOWER_STATUS_HEIGHT)
#define LOWER_STATUS_WIDTH       LCD_WIDTH
#define LOWER_STATUS_HEIGHT      (DISPLAY_TEXT_HEIGHT+1)

#define TRACE_OUTER_X            2
#define TRACE_OUTER_Y            2
#define TRACE_OUTER_WIDTH        (LCD_WIDTH-2*TRACE_OUTER_X)
#define TRACE_OUTER_HEIGHT       (LOWER_STATUS_Y-2*TRACE_OUTER_Y)
#define TRACE_BORDER_THICKNESS   2
#define TRACE_BORDER_COLOR       LCD_COLOR_WHITE
#define TRACE_COLOR              LCD_COLOR_YELLOW

#define TRACE_AREA_X             (TRACE_OUTER_X+TRACE_BORDER_THICKNESS)
#define TRACE_AREA_Y             (TRACE_OUTER_Y+TRACE_BORDER_THICKNESS)
#define TRACE_AREA_WIDTH         (TRACE_OUTER_WIDTH-2*TRACE_BORDER_THICKNESS)
#define TRACE_AREA_HEIGHT        (TRACE_OUTER_HEIGHT-2*TRACE_BORDER_THICKNESS)

#define TRACE_NUM_TICKS          8
#define TRACE_TICK_INTEVAL_H     (TRACE_AREA_WIDTH/TRACE_NUM_TICKS)
#define TRACE_TICK_INTEVAL_V     (TRACE_AREA_HEIGHT/TRACE_NUM_TICKS)

#define TRACE_LEVEL_INVALID      (-1)


#define SCOPE_DISPLAY_TASK_NAME     (signed char *)"ScopeDisplay"
#define SCOPE_DISPLAY_TASK_STACK    128
#define SCOPE_DISPLAY_TASK_PRIORITY (tskIDLE_PRIORITY+2)


// todo: move me
#define ADC_MAX_COUNTS           255


typedef enum
{
   SCOPE_DISPLAY_EVENT_UPDATE_TRACE = 0,
   SCOPE_DISPLAY_EVENT_DRAW_TRACE,
   SCOPE_DISPLAY_EVENT_UPDATE_TIMEBASE,
   SCOPE_DISPLAY_EVENT_UPDATE_VDIV,
   SCOPE_DISPLAY_EVENT_TIMEBASE_INC,
   SCOPE_DISPLAY_EVENT_TIMEBASE_DEC
} ScopeDisplayEventType_E;


typedef struct
{
   ScopeDisplayEventType_E type;

   union
   {
      AdcCounts_T adcReading;

      struct
      {
         volatile AdcCounts_T *data;
         size_t length;
      } AdcMemory;

      struct
      {
         TimebaseValue_T value;
         TimebaseUnits_E units;
      } TimebaseData;

      struct
      {
         V_DivValue_T value;
         V_DivUnits_E units;
      } VdivData;
   };
} ScopeDisplayEvent_T;


bool ScopeDisplayInit();
void ScopeDisplayQueueEvent(ScopeDisplayEvent_T *event);


#endif //__SCOPE_DISPLAY_H__
