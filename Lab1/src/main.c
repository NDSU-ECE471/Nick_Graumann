#include "lpc17xx.h"
#include "core_cm3.h"
#include "type.h"

#define SYSTICK_TIME_MS 10
#define NUM_LEDS 8
#define LED_PIN_NUM_BASE 19
#define LED_PIN_NUM_HIGH 26
#define LED_PINS_ALL ((0xFF<<19))
#define BUTTON0_BIT ((1<<2))

volatile uint32_t ticks = 0;
uint8_t enable = TRUE;
int8_t ledPos = 0;

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

void TIMER0_IRQHandler(void)
{
   if(LPC_TIM0->IR & 0x01)
   {
      LPC_TIM0->IR |= 0x01;

      if(enable && ledPos <= 6)
      {
         LPC_GPIO1->FIOSET |= 1<<(LED_PIN_NUM_BASE+ledPos+1);
      }

      if(enable && ledPos >= 1)
      {
         LPC_GPIO1->FIOSET |= 1<<(LED_PIN_NUM_BASE+ledPos-1);
      }
   }

   if(LPC_TIM0->IR & 0x02)
   {
      LPC_TIM0->IR |= 0x02;

      if(enable && ledPos <= 6)
      {
         LPC_GPIO1->FIOCLR |= 1<<(LED_PIN_NUM_BASE+ledPos+1);
      }

      if(enable && ledPos >= 1)
      {
         LPC_GPIO1->FIOCLR |= 1<<(LED_PIN_NUM_BASE+ledPos-1);
      }
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

	// LEDs outputs
	LPC_GPIO1->FIODIR |= LED_PINS_ALL;
	// LEDS off (active low)
	LPC_GPIO1->FIOSET = LED_PINS_ALL;

	// Enable falling edge interrupt for P0.2
	LPC_GPIOINT->IO0IntEnF |= BUTTON0_BIT;
	// Enable EINT3 interrupt (for GPIO interrupts)
	//NVIC->ISER[0] |= 1<<EINT3_IRQn;
	NVIC_EnableIRQ(EINT3_IRQn);

	// Enable Timer1, PCLKSEL0 already setup for CCLK/4
	LPC_SC->PCONP |= 1<<1;
	LPC_TIM0->TC = 0;
	LPC_TIM0->PC = 0;
	LPC_TIM0->TCR |= 1<<0;
	LPC_TIM0->TCR |= (1<<1);
	LPC_TIM0->TCR &= ~(1<<1);
	// Interrupt on MR0, MR1, reset on MR1
	LPC_TIM0->MCR |= 0x19;
	LPC_TIM0->MR0 = 2000;
	LPC_TIM0->MR1 = 25000;
	NVIC_EnableIRQ(TIMER0_IRQn);

	int8_t inc = 1;

	while(1)
	{
	   if(ticks == 10)
	   {
         ticks = 0;

         if(enable)
         {
            LPC_GPIO1->FIOSET |= 1<<(LED_PIN_NUM_BASE + ledPos);

            ledPos += inc;
            if(ledPos == 0 || ledPos == 7)
            {
               inc = -inc;
            }

            LPC_GPIO1->FIOCLR |= 1<<(LED_PIN_NUM_BASE + ledPos);
         }
         else
         {
            LPC_GPIO1->FIOSET = LED_PINS_ALL;
         }
	   }
   }
}
