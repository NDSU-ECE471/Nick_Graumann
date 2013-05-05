#include "LPC17xx.h"
#include "../GPIO_Facade.h"


///////////////////////////////////////////////////////////////////////////////
//
// Local definitions
//
///////////////////////////////////////////////////////////////////////////////
LPC_GPIO_TypeDef *LPC_GPIO[] =
{
   LPC_GPIO0,
   LPC_GPIO1,
   LPC_GPIO2,
   LPC_GPIO3,
   LPC_GPIO4
};
#define NUM_GPIO_PORTS (sizeof(LPC_GPIO)/sizeof(LPC_GPIO[0]))


///////////////////////////////////////////////////////////////////////////////
//
// See GPIO_Facade.h
//
///////////////////////////////////////////////////////////////////////////////
GPIO_Error_E GPIO_PortGet(uint32_t port, uint32_t *res)
{
   GPIO_Error_E err = GPIO_SUCCESS;

   if(port >= NUM_GPIO_PORTS || !res)
   {
      err = GPIO_INVALID;
   }
   else
   {
      *res = LPC_GPIO[port]->FIOPIN;
   }

   return err;
}

