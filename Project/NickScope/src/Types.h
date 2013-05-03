#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdbool.h>
#include <stdint.h>
#include "LPC17xx.h"

extern LPC_GPIO_TypeDef *LPC_GPIO[];

typedef uint16_t AdcCounts_T;
typedef uint16_t UiValue;

extern UiValue UI_VALUES[];
extern size_t UI_VALUES_SIZE;

typedef uint16_t V_DivValue_T;

typedef enum
{
   VDIV_mV,
   VDIV_V,
} V_DivUnits_E;

typedef uint16_t TimebaseValue_T;

typedef enum
{
   TIMEBASE_uS = 0,
   TIMEBASE_mS,
   TIMEBASE_S,
   NUM_TIMEBASES
} TimebaseUnits_E;

#endif //__TYPES_H__
