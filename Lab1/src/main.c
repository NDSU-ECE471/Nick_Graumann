#include "lpc17xx.h"
#include "core_cm3.h"
#include "type.h"

#define SYSTICK_TIME_MS 10
#define P1_19_to_P1_26 0x07F80000
#define BUTTON0_BIT ((1<<2))

volatile uint32_t ticks = 0;
uint8_t enable = TRUE;

void SysTick_Handler(void)
{
   SysTick->CTRL &= ~(1<<16);
   ticks++;
}

void EINT3_IRQHandler(void)
{
   // Check for falling edge interrupt
   if(LPC_GPIOINT->IO0IntStatF & BUTTON0_BIT)
   {
      LPC_GPIOINT->IO0IntClr |= BUTTON0_BIT;
      enable = !enable;
   }
}

int main (void)
{
	// Update system clock frequency
	SystemCoreClockUpdate();

   // Systick use processor clock, enable
   SysTick->CTRL |= 0x07;
   // Systick has reload value of N-1 clock cycles
   SysTick->LOAD = (SystemCoreClock/1000)*SYSTICK_TIME_MS - 1;

	// P1.19-P1.26 outputs
	LPC_GPIO1->FIODIR |= P1_19_to_P1_26;
	// P1.19-P1.26 off
	LPC_GPIO1->FIOSET = P1_19_to_P1_26;

	// Enable falling edge interrupt for P0.2
	LPC_GPIOINT->IO0IntEnF |= BUTTON0_BIT;
	// Enable EINT3 interrupt (for GPIO interrupts)
	NVIC->ISER[0] |= 1<<EINT3_IRQn;

	int8_t pos = 0, inc = 1;

	while(1)
	{
	   if(ticks == 10)
	   {
         ticks = 0;

         if(enable)
         {
            LPC_GPIO1->FIOSET |= 1<<(19 + pos);

            pos += inc;
            if(pos == 0 || pos == 7)
            {
               inc = -inc;
            }

            LPC_GPIO1->FIOCLR |= 1<<(19 + pos);
         }
         else
         {
            LPC_GPIO1->FIOSET = P1_19_to_P1_26;
         }
	   }
   }
}
