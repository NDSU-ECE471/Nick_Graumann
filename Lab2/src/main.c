#include "lpc17xx.h"
#include "core_cm3.h"
#include "type.h"

#define SYSTICK_TIME_MS 1
#define LED_BITS_ALL ((0xFF<<19))
#define LED0_BIT ((1<<19))
#define LED1_BIT ((1<<20))
#define LED2_BIT ((1<<21))
#define BUTTON0_BIT ((1<<2))
#define BUTTON1_BIT ((1<<3))

volatile uint32_t ticks = 0;

inline static void DelayMs(uint32_t delayTicks)
{
   uint32_t currentTicks = ticks;
   while((ticks - currentTicks) < delayTicks);
}

int main (void)
{
	// Update system clock frequency
	SystemCoreClockUpdate();

   SysTick_Config((SystemCoreClock/1000)*SYSTICK_TIME_MS);
   // Systick needs highest priority for blinking
   SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
   NVIC_SetPriority(SysTick_IRQn, 0);
   SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

	// LEDs outputs
	LPC_GPIO1->FIODIR |= LED_BITS_ALL;
	// LEDS off (active low)
	LPC_GPIO1->FIOSET = LED_BITS_ALL;

	// Low-ish priority
	NVIC_SetPriority(EINT0_IRQn, 0x08);
	// Clear any pending interrupts as reccommended in section 3.6.3 of the
	// LPC1769 user manual
	NVIC_ClearPendingIRQ(EINT0_IRQn);
	NVIC_EnableIRQ(EINT0_IRQn);

	// Enable falling edge interrupt for button 1
	LPC_GPIOINT->IO0IntEnF |= BUTTON1_BIT;
	// Very high priority
	NVIC_SetPriority(EINT3_IRQn, 0x01);
	NVIC_ClearPendingIRQ(EINT3_IRQn);
	NVIC_EnableIRQ(EINT3_IRQn);

	while(1)
	{
	   // Only issue with this implementation: button0 must be held down until
	   // the next 500ms interval is reached. This issue would be avoided in
	   // practice by using the external GPIO interrupt which is hardware-based
	   // instead of software triggering.
	   DelayMs(500);
      LPC_GPIO1->FIOPIN ^= LED0_BIT;

      // Switches are active low
	   if(!(LPC_GPIO0->FIOPIN & BUTTON0_BIT))
	   {
	      // Software trigger interrupt
	      NVIC_SetPendingIRQ(EINT0_IRQn);
	   }
   }
}

void SysTick_Handler(void)
{
   ticks++;
}

void EINT0_IRQHandler(void)
{
   while(1)
   {
      DelayMs(300);
      LPC_GPIO1->FIOPIN ^= LED1_BIT;
   }
}

void EINT3_IRQHandler(void)
{
   // Check for falling edge interrupt
   if(LPC_GPIOINT->IO0IntStatF & BUTTON1_BIT)
   {
      LPC_GPIOINT->IO0IntClr |= BUTTON1_BIT;

      while(1)
      {
         DelayMs(100);
         LPC_GPIO1->FIOPIN ^= LED2_BIT;
      }
   }
}
