#include "Types.h"

LPC_GPIO_TypeDef *LPC_GPIO[] =
{
   LPC_GPIO0,
   LPC_GPIO1,
   LPC_GPIO2,
   LPC_GPIO3,
   LPC_GPIO4
};


UiValue UI_VALUES[] = { 1, 2, 5, 10, 20, 50, 100, 200, 500 };
size_t UI_VALUES_SIZE = sizeof(UI_VALUES) / sizeof(UI_VALUES[0]);
