#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "LPC17xx.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "AdcReader/AdcReader.h"
#include "ScopeDisplay/ScopeDisplay.h"


LPC_GPIO_TypeDef *LPC_GPIO[] =
{
   LPC_GPIO0,
   LPC_GPIO1,
   LPC_GPIO2,
   LPC_GPIO3,
   LPC_GPIO4
};


int main(void)
{
   SystemCoreClockUpdate();

   if(AdcReaderInit() &&
      ScopeDisplayInit())
   {
      vTaskStartScheduler();
   }

   // Error trap
   while(1);
}


void vApplicationIdleHook()
{
   // Sleep while idle
   __WFI();
}


void vApplicationStackOverflowHook()
{
   while(1);
}


void vApplicationMallocFailedHook()
{
   while(1);
}

