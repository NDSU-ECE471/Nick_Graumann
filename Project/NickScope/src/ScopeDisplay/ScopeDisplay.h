#ifndef __SCOPE_DISPLAY_H__
#define __SCOPE_DISPLAY_H__

#include <stdbool.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "LcdDisplay.h"

#define DISPLAY_BG_COLOR      LCD_COLOR_BLACK
#define DISPLAY_TRACE_COLOR   LCD_COLOR_YELLOW

#define SCOPE_DISPLAY_TASK_NAME     (signed char *)"ScopeDisplay"
#define SCOPE_DISPLAY_TASK_STACK    128
#define SCOPE_DISPLAY_TASK_PRIORITY (tskIDLE_PRIORITY+2)


typedef enum
{
   SCOPE_DISPLAY_EVENT_UPDATE = 0
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
