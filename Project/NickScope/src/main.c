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
#include "Drivers/Clocks/ClocksFacade.h"
#include "UserInput/UserInput.h"


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
      UserInputInit())
   {
      vTaskStartScheduler();
   }

   // Error trap
   while(1);

   return 0;
}


void vApplicationIdleHook()
{
   // Sleep while idle
   __WFI();
}


void vApplicationStackOverflowHook()
{
   // Trap so we can attach with a debugger
   while(1);
}


void vApplicationMallocFailedHook()
{
   // Trap so we can attach with a debugger
   while(1);
}

