#include <stdbool.h>
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

static inline void UpdateTimeFromTick(Time_T *time);


typedef enum
{
   STATE_STOPPED,
   STATE_RUNNING
} WatchState_E;

volatile WatchState_E WatchState = STATE_STOPPED;


#define BUTTON_DEBOUNCE_TIME  40
#define START_STOP_BUTTON     0
#define RESET_BUTTON          1
uint8_t buttonHysteresisMs[2] = { 0, 0 };
uint8_t buttonPrevState = 0;
// Enable both bits
uint8_t buttonEnable = BUTTON0_BIT | BUTTON1_BIT;


// Screen stuff
#define TEXT_COLOR ST7735_16_WHITE
#define BG_COLOR   ST7735_16_BLUE

void LCD_DrawTime(uint8_t x, uint8_t y, Time_T *time);

int main (void)
{
	// Update system clock frequency
	SystemCoreClockUpdate();

	lcd_init();
	fillScreen(BG_COLOR);
	setColor16(TEXT_COLOR);
	setBackgroundColor16(BG_COLOR);
	drawString(10, 10, "-- Nick's Stopwatch --");
	drawString(10, 100, "P0.2: Start/stop");
	drawString(10, 110, "P0.3: Reset");

   SysTick_Config((SystemCoreClock/1000)*SYSTICK_TIME_MS);

	while(1)
	{
	   LCD_DrawTime(10, 30, (Time_T *)&RunTime);

	   // This debounce code is designed specifically for a stopwatch. It works as follows:
	   // Trigger immediately upon the first falling edge detect. Once triggered, disable the button and
	   // add a hysteresis delay. The button stays disabled until a rising edge is detected after
	   // the hysteresis delay. This results in ideal behavior for a stopwatch, where the start/stop triggers
	   // immediately upon press, and holding the button doesn't affect functionality.
	   if(!(LPC_GPIO0->FIOPIN & BUTTON0_BIT) && (buttonEnable & BUTTON0_BIT) && buttonHysteresisMs[START_STOP_BUTTON] == 0)
	   {
	      buttonHysteresisMs[START_STOP_BUTTON] = BUTTON_DEBOUNCE_TIME;
	      buttonEnable &= ~BUTTON0_BIT;

	      // Actual start/stop functionality here
	      if(STATE_STOPPED == WatchState)
         {
            WatchState = STATE_RUNNING;
         }
         else
         {
            WatchState = STATE_STOPPED;
         }
	   }
	   else if(!(buttonEnable & BUTTON0_BIT) && (LPC_GPIO0->FIOPIN & BUTTON0_BIT))
	   {
	      buttonEnable |= BUTTON0_BIT;
	   }

	   // Similar debounce for reset button
	   if(!(LPC_GPIO0->FIOPIN & BUTTON1_BIT) && (buttonEnable & BUTTON1_BIT) && buttonHysteresisMs[RESET_BUTTON] == 0)
      {
         buttonHysteresisMs[RESET_BUTTON] = BUTTON_DEBOUNCE_TIME;
         buttonEnable &= ~BUTTON1_BIT;

         memset((Time_T *)&RunTime, 0, sizeof(RunTime));
         WatchState = STATE_STOPPED;
      }
      else if(!(buttonEnable & BUTTON1_BIT) && (LPC_GPIO0->FIOPIN & BUTTON1_BIT))
      {
         buttonEnable |= BUTTON1_BIT;
      }
   }

	return 0;
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

static inline void UpdateTimeFromTick(Time_T *time)
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

__attribute__ ((interrupt))
void SysTick_Handler(void)
{
   // No for loop here for speed (compiler would probably unroll it anyway)
   if(buttonHysteresisMs[START_STOP_BUTTON] > 0)
   {
      buttonHysteresisMs[START_STOP_BUTTON] -= SYSTICK_TIME_MS;
   }

   if(buttonHysteresisMs[RESET_BUTTON] > 0)
   {
      buttonHysteresisMs[RESET_BUTTON] -= SYSTICK_TIME_MS;
   }

   if(STATE_RUNNING == WatchState)
   {
      UpdateTimeFromTick((Time_T *)&RunTime);
   }
}
