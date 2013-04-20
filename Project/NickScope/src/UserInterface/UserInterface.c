#include "ScopeDisplay/ScopeDisplay.h"
#include "UserInterface.h"


bool UserInterfaceInit()
{
   ScopeDisplayEvent_T displayEvent;
   displayEvent.type = SCOPE_DISPLAY_EVENT_UPDATE_TIMEBASE;
   displayEvent.TimebaseData.value = 200;
   displayEvent.TimebaseData.units = TIMEBASE_mS;
   ScopeDisplayQueueEvent(&displayEvent);

   displayEvent.type = SCOPE_DISPLAY_EVENT_UPDATE_VDIV;
   displayEvent.VdivData.value = 400;
   displayEvent.VdivData.units = VDIV_mV;
   ScopeDisplayQueueEvent(&displayEvent);
   return true;
}
