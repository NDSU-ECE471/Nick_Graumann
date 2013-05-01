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


static bool HwInit()
{
   bool retVal = (ClocksInit() == CLOCKS_SUCCESS);
   retVal = (retVal && ClocksSetCore(configCPU_CLOCK_HZ) == CLOCKS_SUCCESS);
   return retVal;
}


int main(void)
{
   if(HwInit() &&
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

