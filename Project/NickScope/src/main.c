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


int main(void)
{
   // Disconnect from main PLL
   LPC_SC->PLL0CON = 0x01;
   LPC_SC->PLL0FEED = 0xAA;
   LPC_SC->PLL0FEED = 0x55;

   // Disable main PLL
   LPC_SC->PLL0CON = 0x00;
   LPC_SC->PLL0FEED = 0xAA;
   LPC_SC->PLL0FEED = 0x55;

   // Setup PLL N=1, M=15 for 360MHz
   LPC_SC->PLL0CFG = (14<<0);
   LPC_SC->PLL0FEED = 0xAA;
   LPC_SC->PLL0FEED = 0x55;

   // Re-enable PLL
   LPC_SC->PLL0CON = 0x01;
   LPC_SC->PLL0FEED = 0xAA;
   LPC_SC->PLL0FEED = 0x55;

   // CPU clock div of 3
   LPC_SC->CCLKCFG = 0x02;

   // Wait for PLL lock
   while (!(LPC_SC->PLL0STAT & (1<<26)));

   // Connect main PLL to core
   LPC_SC->PLL0CON = 0x03;
   LPC_SC->PLL0FEED = 0xAA;
   LPC_SC->PLL0FEED = 0x55;

   SystemCoreClockUpdate();

   if(AdcReaderInit() &&
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

