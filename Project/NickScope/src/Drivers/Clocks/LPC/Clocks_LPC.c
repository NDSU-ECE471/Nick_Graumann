#include "LPC17xx.h"
#include "system_LPC17xx.h"
#include "Clocks_LPC.h"


///////////////////////////////////////////////////////////////////////////////
//
// Defines
//
///////////////////////////////////////////////////////////////////////////////
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

// M and N values for various clocks
#define PLL0_M_80       40
#define PLL0_N_80       3

#define PLL0_M_96       12
#define PLL0_N_96       1

#define PLL0_M_100      25
#define PLL0_N_100      3

#define PLL0_M_120      15
#define PLL0_N_120      1


///////////////////////////////////////////////////////////////////////////////
//
// See Clocks_LPC.h
//
///////////////////////////////////////////////////////////////////////////////
ClocksErr_E LPC_InitClocks()
{
   SystemCoreClockUpdate();
   return CLOCKS_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
//
// See Clocks_LPC.h
//
///////////////////////////////////////////////////////////////////////////////
ClockRate_T LPC_GetCoreClock()
{
   return (ClockRate_T)SystemCoreClock;
}


///////////////////////////////////////////////////////////////////////////////
//
// See Clocks_LPC.h
//
///////////////////////////////////////////////////////////////////////////////
ClocksErr_E LPC_SetupPLL0(ClockRate_T rate)
{
   ClocksErr_E err = CLOCKS_SUCCESS;

   uint32_t mVal, nVal, clkDiv;
   switch(rate)
   {
   case 80000000:
      mVal = PLL0_M_80;
      nVal = PLL0_N_80;
      clkDiv = CORE_CLOCKDIV_4;
      break;

   case 96000000:
      mVal = PLL0_M_96;
      nVal = PLL0_N_96;
      clkDiv = CORE_CLOCKDIV_3;
      break;

   case 100000000:
      mVal = PLL0_M_100;
      nVal = PLL0_N_100;
      clkDiv = CORE_CLOCKDIV_3;
      break;

   case 120000000:
      mVal = PLL0_M_120;
      nVal = PLL0_N_120;
      clkDiv = CORE_CLOCKDIV_3;
      break;

   default:
      err = CLOCKS_UNSUPPORTED;
   }

   if(CLOCKS_SUCCESS == err)
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
      LPC_SC->PLL0CFG = ((nVal-1)<<PLL0_N_SHIFT)|((mVal-1)<<PLL0_M_SHIFT);

      LPC_SC->PLL0FEED = PLL0_FEED_SEQ1;
      LPC_SC->PLL0FEED = PLL0_FEED_SEQ2;

      // Re-enable PLL
      LPC_SC->PLL0CON = PLL0_ENABLE;
      LPC_SC->PLL0FEED = PLL0_FEED_SEQ1;
      LPC_SC->PLL0FEED = PLL0_FEED_SEQ2;

      // CPU clock divider
      LPC_SC->CCLKCFG = clkDiv;

      // Wait for PLL lock
      while (!(LPC_SC->PLL0STAT & PLL0_PLOCK));

      // Connect main PLL to core
      LPC_SC->PLL0CON = PLL0_ENABLE | PLL0_CONNECT;
      LPC_SC->PLL0FEED = PLL0_FEED_SEQ1;
      LPC_SC->PLL0FEED = PLL0_FEED_SEQ2;

      SystemCoreClockUpdate();
   }

   return err;
}

