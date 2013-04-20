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
#include "UserInterface/UserInterface.h"


int main(void)
{
   SystemCoreClockUpdate();

   if(AdcReaderInit() &&
      ScopeDisplayInit() &&
      UserInterfaceInit())
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

