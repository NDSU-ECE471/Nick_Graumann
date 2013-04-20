#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdbool.h>
#include <stdint.h>
#include "LPC17xx.h"

extern LPC_GPIO_TypeDef *LPC_GPIO[];

typedef int32_t AdcCounts_T;

typedef uint16_t UiValue;

extern UiValue UI_VALUES[];
extern size_t UI_VALUES_SIZE;

typedef enum
{
   VDIV_mV,
   VDIV_V,
} V_PerDivUnits_E;

typedef enum
{
   TIMEBASE_nS = 0,
   TIMEBASE_uS,
   TIMEBASE_mS,
   TIMEBASE_S
} TimebaseUnits_E;

#endif //__TYPES_H__
