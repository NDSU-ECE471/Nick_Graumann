#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "lpc17xx.h"
#include "core_cm3.h"

#include "lcd/lcd.h"

// Screen stuff
#define TEXT_COLOR      ST7735_16_GREEN
#define BG_COLOR        ST7735_16_BLACK

#define SCREEN_WIDTH    JDT1800_WIDTH
#define SCREEN_HEIGHT   JDT1800_HEIGHT
#define TEXT_WIDTH      6
#define TEXT_HEIGHT     10
#define LEFT_MARGIN     TEXT_WIDTH
#define TOP_MARGIN      TEXT_HEIGHT
// underscore character for cursor
#define CURSOR_CHAR     0x5f

#define UART_BAUD       9600
// Bittime = Timer clk/baud = 25000000/9600
#define UART_BITTIME    2604
#define UART_DATA_BITS  8
#define UART_RX_PORT    2
#define UART_RX_PIN     1
#define UART_RX_PIN_BIT ((1<<UART_RX_PIN))
#define UART_BITIME

typedef struct
{
   // "Private" members
   bool receiving;
   uint8_t currentRxBit;

   // "Public" members
   bool rxDataReady;
   uint8_t rxData;
} SwUart;

volatile SwUart FtdiUart =
{
      .receiving = false,
      .currentRxBit = 0,
      .rxDataReady = true,
      .rxData = 0
};

#define TIMER_MATCH0_BIT ((1<<0))

void ClearScreenAndPrintHeader();
void DrawCharacterOnScreen(uint8_t character, uint8_t *currXPtr, uint8_t *currYPtr);

void SetupTimer0(uint32_t value, bool repeat);
void DisableTimer0();

int main ()
{
	// Update system clock frequency
	SystemCoreClockUpdate();

	LPC_GPIO1->FIODIR |= 1<<19;

	lcd_init();
	ClearScreenAndPrintHeader();

	// Enable falling edge interrupt for UART RX
   LPC_GPIOINT->IO2IntEnF |= UART_RX_PIN_BIT;
   // Very high priority
   NVIC_SetPriority(EINT3_IRQn, 0x01);
   NVIC_ClearPendingIRQ(EINT3_IRQn);
   NVIC_EnableIRQ(EINT3_IRQn);

   SetupTimer0(SystemCoreClock / 4 * 2, true);

   uint8_t currX = LEFT_MARGIN, currY = 2*TOP_MARGIN;
   drawChar(currX, currY, CURSOR_CHAR);

	while(1)
	{
	   if(FtdiUart.rxDataReady)
	   {
	      FtdiUart.rxDataReady = false;

	      // Ignore NULL, invalid characters
	      if(FtdiUart.rxData != 0 && FtdiUart.rxData <= 0x7F)
	      {
	         DrawCharacterOnScreen(FtdiUart.rxData, &currX, &currY);
	      }

	   }
   }

	return 0;
}

void ClearScreenAndPrintHeader()
{
   fillScreen(BG_COLOR);
   setColor16(TEXT_COLOR);
   setBackgroundColor16(BG_COLOR);
   drawString(LEFT_MARGIN, TOP_MARGIN, "Received data:");
}

void DrawCharacterOnScreen(uint8_t character, uint8_t *currX, uint8_t *currY)
{
   switch(FtdiUart.rxData)
   {
   case '\t':
      // Erase current cursor
      drawChar(*currX, *currY, ' ');

      *currX += 4*TEXT_WIDTH;
      break;
   case '\r':
   case '\n':
      if(*currY < SCREEN_HEIGHT-2*TEXT_HEIGHT)
      {
         // Erase current cursor
         drawChar(*currX, *currY, ' ');

         *currX = LEFT_MARGIN;
         *currY += TEXT_HEIGHT;
      }
      else
      {
         ClearScreenAndPrintHeader();
         *currX = LEFT_MARGIN;
         *currY = 2*TOP_MARGIN;
      }
      break;
   case 0x08: // backspace
   case 0x7F: // delete
      // Only allow us to delete on the current line
      if(*currX > LEFT_MARGIN)
      {
         // Erase current cursor
         drawChar(*currX, *currY, ' ');

         *currX -= TEXT_WIDTH;
         // cursor will draw over the character
      }

      break;
   default: // normal characters
      drawChar(*currX, *currY, FtdiUart.rxData);
      *currX += TEXT_WIDTH;
      if(*currX > SCREEN_WIDTH-(2*TEXT_WIDTH))
      {
         *currX = LEFT_MARGIN;

         if(*currY < SCREEN_HEIGHT-2*TEXT_HEIGHT)
         {
            *currY += TEXT_HEIGHT;
         }
         else
         {
            ClearScreenAndPrintHeader();
            *currY = 2*TOP_MARGIN;
         }
      }
   }

   drawChar(*currX, *currY, CURSOR_CHAR);
}

void SetupTimer0(uint32_t value, bool repeat)
{
   // Timer mode
   LPC_TIM0->CTCR = 0;

   // Reset values
   LPC_TIM0->TCR |= (1<<1);
   LPC_TIM0->TCR &= ~(1<<1);

   // No prescale - increment every clock
   LPC_TIM0->PR = 0;

   // Setup match value
   LPC_TIM0->MR0 = value;

   if(repeat)
   {
      // Interrupt on match, reset on match
      LPC_TIM0->MCR = 0x03;
   }
   else
   {
      // Interrupt on match, stop on match
      LPC_TIM0->MCR = 0x05;
   }

   // Clear all interrupts
   LPC_TIM0->IR |= 0xFFFFFFFF;

   // Enable timer
   LPC_TIM0->TCR |= 0x01;

   // Lower priority than the GPIO interrupt
   NVIC_SetPriority(TIMER0_IRQn, 2);
   // Enable interrupt for timer 0
   NVIC_EnableIRQ(TIMER0_IRQn);
}

void DisableTimer0()
{
   NVIC_DisableIRQ(TIMER0_IRQn);
}

__attribute__ ((interrupt))
void TIMER0_IRQHandler()
{
   if(LPC_TIM0->IR & TIMER_MATCH0_BIT)
   {
      LPC_TIM0->IR |= TIMER_MATCH0_BIT;

      if(!FtdiUart.receiving)
      {
         FtdiUart.receiving = true;
         FtdiUart.currentRxBit = 0;
         FtdiUart.rxDataReady = false;
         FtdiUart.rxData = 0;

         SetupTimer0(UART_BITTIME, true);
      }
      else
      {
         // Normally would subtract 1, but we want the stop bit too
         if(FtdiUart.currentRxBit == UART_DATA_BITS)
         {
            DisableTimer0();

            FtdiUart.receiving = false;

            // Check for valid stop bit
            if(LPC_GPIO2->FIOPIN & UART_RX_PIN_BIT)
            {
               FtdiUart.rxDataReady = true;
            }
            else
            {
               FtdiUart.rxDataReady = false;
            }
         }

         if(LPC_GPIO2->FIOPIN & UART_RX_PIN_BIT)
         {
            FtdiUart.rxData |= 1<<FtdiUart.currentRxBit;
         }

         if(FtdiUart.currentRxBit < UART_DATA_BITS)
         {
            FtdiUart.currentRxBit++;
         }
      }
   }
}

__attribute__ ((interrupt))
void EINT3_IRQHandler()
{
   if(LPC_GPIOINT->IO2IntStatF & UART_RX_PIN_BIT)
   {
      LPC_GPIOINT->IO2IntClr |= UART_RX_PIN_BIT;

      if(!FtdiUart.receiving)
      {
         SetupTimer0(UART_BITTIME / 2, false);
      }
   }
}
