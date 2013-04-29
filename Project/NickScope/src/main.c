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

#include <cr_section_macros.h>
#include "Drivers/DMA/LPC/DMA_LPC.h"
//static const char a[] = { 1, 2, 3, 4 };
//static const char b[] = { 5, 6, 7, 8 };


int main(void)
{
   if(ClocksInit() == CLOCKS_SUCCESS &&
      ClocksSetCore(96000000) == CLOCKS_SUCCESS &&
      AdcReaderInit() &&
      ScopeDisplayInit() &&
      ConsoleInit())
   {
      /*LPC_DMA_Init();
      LPC_DMA_InitChannel(0, (uint32_t)&a, 0x20080000,
                          DMA_PERIPH_MEMORY, DMA_PERIPH_MEMORY, 1,
                          DMA_BURST_SIZE_4, DMA_BURST_SIZE_4,
                          DMA_XFER_WIDTH_32, DMA_XFER_WIDTH_32, false, true);
      LPC_DMA_BeginTransfer(0);*/

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

