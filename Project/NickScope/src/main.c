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

#define CORE_CLOCKDIV_3 0x02
#define CORE_CLOCKDIV_4 0x03

#define PLL0_FEED_SEQ1  0xAA
#define PLL0_FEED_SEQ2  0x55

#define PLL0_NONE       0
#define PLL0_ENABLE     (1<<0)
#define PLL0_CONNECT    (1<<1)

#define PLL0_PLOCK      (1<<26)

#define PLL0_M_SHIFT    0
#define PLL0_N_SHIFT    16

// M and N for 96MHz core clock
#define PLL0_N          1
#define PLL0_M          12


static void SetupMainPLL()
{
   // Disconnect from main PLL
   LPC_SC->PLL0CON = PLL0_ENABLE;
   LPC_SC->PLL0FEED = PLL0_FEED_SEQ1;
   LPC_SC->PLL0FEED = PLL0_FEED_SEQ2;

   // Disable main PLL
   LPC_SC->PLL0CON = PLL0_NONE;
   LPC_SC->PLL0FEED = PLL0_FEED_SEQ1;
   LPC_SC->PLL0FEED = PLL0_FEED_SEQ2;

   // Setup new PLL speed
   LPC_SC->PLL0CFG = ((PLL0_N-1)<<PLL0_N_SHIFT)|((PLL0_M-1)<<PLL0_M_SHIFT);

   LPC_SC->PLL0FEED = PLL0_FEED_SEQ1;
   LPC_SC->PLL0FEED = PLL0_FEED_SEQ2;

   // Re-enable PLL
   LPC_SC->PLL0CON = PLL0_ENABLE;
   LPC_SC->PLL0FEED = PLL0_FEED_SEQ1;
   LPC_SC->PLL0FEED = PLL0_FEED_SEQ2;

   // CPU clock divider
   LPC_SC->CCLKCFG = CORE_CLOCKDIV_4;

   // Wait for PLL lock
   while (!(LPC_SC->PLL0STAT & PLL0_PLOCK));

   // Connect main PLL to core
   LPC_SC->PLL0CON = PLL0_ENABLE | PLL0_CONNECT;
   LPC_SC->PLL0FEED = PLL0_FEED_SEQ1;
   LPC_SC->PLL0FEED = PLL0_FEED_SEQ2;

   SystemCoreClockUpdate();
}


int main(void)
{
   SetupMainPLL();

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

