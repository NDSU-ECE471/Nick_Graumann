#include "../ClocksFacade.h"
#include "Clocks_LPC.h"


///////////////////////////////////////////////////////////////////////////////
//
// See ClocksFacade.h
//
///////////////////////////////////////////////////////////////////////////////
ClocksErr_E ClocksInit()
{
   return LPC_InitClocks();
}


ClocksErr_E ClocksGetCore(ClockRate_T *rate)
{
   ClocksErr_E err = CLOCKS_SUCCESS;

   if(!rate)
   {
      err = CLOCKS_INVALID_PARAM;
   }
   else
   {
      *rate = LPC_GetCoreClock();
   }

   return err;
}


///////////////////////////////////////////////////////////////////////////////
//
// See ClocksFacade.h
//
///////////////////////////////////////////////////////////////////////////////
ClocksErr_E ClocksSetCore(ClockRate_T rate)
{
   return LPC_SetupPLL0(rate);
}

