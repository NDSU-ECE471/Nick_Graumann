#include <stdio.h>
#include <string.h>
#include "lpc17xx.h"
#include "core_cm3.h"
#include "type.h"

#include "lcd/lcd.h"

#define SYSTICK_TIME_MS 10
#define BUTTON0_BIT ((1<<2))
#define BUTTON1_BIT ((1<<3))

typedef struct
{
   uint8_t hours;
   uint8_t minutes;
   uint8_t seconds;
   uint8_t tenMs;
} Time_T;

volatile Time_T RunTime;

typedef enum
{
   STATE_STOPPED,
   STATE_RUNNING
} WatchState_E;

volatile WatchState_E WatchState = STATE_STOPPED;

void LCD_DrawTime(uint8_t x, uint8_t y, Time_T *time);

int main (void)
{
	// Update system clock frequency
	SystemCoreClockUpdate();

	lcd_init();
	fillScreen(ST7735_16_WHITE);
	setColor16(0);
	setBackgroundColor16(ST7735_16_WHITE);
	drawString(10, 10, "-- Stopwatch --");
	drawString(10, 100, "Button0: Start/stop");
	drawString(10, 110, "Button1: Reset");

   SysTick_Config((SystemCoreClock/1000)*SYSTICK_TIME_MS);
   // Systick needs highest priority
   SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
   NVIC_SetPriority(SysTick_IRQn, 0);
   SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

	// Enable falling edge interrupt for button 0, 1
	LPC_GPIOINT->IO0IntEnF |= (BUTTON0_BIT | BUTTON1_BIT);
	// Priority must be lower than systick for accurate timing
	NVIC_SetPriority(EINT3_IRQn, 0x01);
	NVIC_ClearPendingIRQ(EINT3_IRQn);
	NVIC_EnableIRQ(EINT3_IRQn);

	while(1)
	{
	   LCD_DrawTime(10, 30, (Time_T *)&RunTime);
   }

	return 0;
}

void UpdateTimeFromTick(Time_T *time)
{
   if(100 == ++time->tenMs)
   {
      time->tenMs = 0;

      if(60 == ++time->seconds)
      {
         time->seconds = 0;

         if(60 == ++time->minutes)
         {
            time->minutes = 0;
            time->hours++;
         }
      }
   }
}

void LCD_DrawTime(uint8_t x, uint8_t y, Time_T *time)
{
   char strBuf[12];
   snprintf(strBuf, 12, "%02u:%02u:%02u:%02u", time->hours,
                                               time->minutes,
                                               time->seconds,
                                               time->tenMs);

   drawString(x, y, strBuf);
}

void SysTick_Handler(void)
{
   if(STATE_RUNNING == WatchState)
   {
      UpdateTimeFromTick((Time_T *)&RunTime);
   }
}

void EINT3_IRQHandler(void)
{
   // Check for falling edge interrupt
   if(LPC_GPIOINT->IO0IntStatF & BUTTON0_BIT)
   {
      LPC_GPIOINT->IO0IntClr |= BUTTON0_BIT;

      if(STATE_STOPPED == WatchState)
      {
         WatchState = STATE_RUNNING;
      }
      else
      {
         WatchState = STATE_STOPPED;
      }
   }

   if(LPC_GPIOINT->IO0IntStatF & BUTTON1_BIT)
   {
      LPC_GPIOINT->IO0IntClr |= BUTTON1_BIT;

      memset((void *)&RunTime, 0, sizeof(RunTime));
   }
}
