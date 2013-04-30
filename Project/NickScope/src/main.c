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
#include "Console/Console.h"
#include "Drivers/Clocks/ClocksFacade.h"


int main(void)
{
   if(ClocksInit() == CLOCKS_SUCCESS &&
      ClocksSetCore(96000000) == CLOCKS_SUCCESS &&
      AdcReaderInit() &&
      ScopeDisplayInit() &&
      ConsoleInit())
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

