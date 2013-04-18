#include "ScopeDisplay/ScopeDisplay.h"
#include "UserInterface.h"


bool UserInterfaceInit()
{
   ScopeDisplayEvent_T displayEvent;
   displayEvent.type = SCOPE_DISPLAY_EVENT_DRAW_INFO;
   ScopeDisplayQueueEvent(&displayEvent);
   return true;
}
