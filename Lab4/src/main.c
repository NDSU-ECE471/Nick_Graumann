#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "lpc17xx.h"
#include "core_cm3.h"

#include "lcd/lcd.h"
#include "lab4.h"

volatile SwUart FtdiUart =
{
      .receiving = false,
      .currentRxBit = 0,
      .rxDataReady = true,
      .rxData = 0,

      .transmitting = false,
      .currentTxBit = 0,
      .txData = 0
};

void ClearScreenAndPrintHeader();
void DrawCharacterOnScreen(uint8_t character, uint8_t *currXPtr, uint8_t *currYPtr);
void UartSetupTransmit(SwUart *uart, uint8_t data);

int main ()
{
	// Update system clock frequency
	SystemCoreClockUpdate();

	lcd_init();
	ClearScreenAndPrintHeader();

	// Enable falling edge interrupt for UART Rx
   LPC_GPIOINT->IO2IntEnF |= UART_RX_PIN_BIT;
   // Very high priority
   NVIC_SetPriority(EINT3_IRQn, 1);
   NVIC_ClearPendingIRQ(EINT3_IRQn);
   NVIC_EnableIRQ(EINT3_IRQn);

   // Uart Tx output, high when idle
   LPC_GPIO2->FIODIR |= UART_TX_PIN_BIT;
   LPC_GPIO2->FIOSET |= UART_TX_PIN_BIT;
   // Uart uses timer2 which is disabled, enable it
   LPC_SC->PCONP |= (1<<22);

   uint8_t currX = LEFT_MARGIN, currY = 2*TOP_MARGIN;
   drawChar(currX, currY, CURSOR_CHAR);

	while(1)
	{
	   if(FtdiUart.rxDataReady)
	   {
	      FtdiUart.rxDataReady = false;

	      // Don't draw NULL, invalid characters
	      if(FtdiUart.rxData != 0 && FtdiUart.rxData <= 0x7F)
	      {
	         DrawCharacterOnScreen(FtdiUart.rxData, &currX, &currY);
	      }

	      UartSetupTransmit((SwUart *)&FtdiUart, FtdiUart.rxData);
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

void UartSetupTransmit(SwUart *uart, uint8_t data)
{
   if(!uart)
   {
      return;
   }

   while(uart->transmitting);

   uart->transmitting = true;
   uart->currentTxBit = 0;
   uart->txData = data;

   // start bit
   LPC_GPIO2->FIOCLR |= UART_TX_PIN_BIT;

   SetupTimer(UART_TX_TIMER, UART_BITTIME, true);
   EnableTimerIrq(UART_TX_TIMER_IRQ, UART_TX_TIMER_PRI);
}

void SetupTimer(LPC_TIM_TypeDef *timer, uint32_t value, bool repeat)
{
   if(!timer)
   {
      return;
   }

   timer->TCR &= ~0x01; // Disable timer
   timer->CTCR = 0; // Timer mode

   // Reset counter values
   timer->TCR |= (1<<1);
   timer->TCR &= ~(1<<1);

   timer->PR = 0; // No prescale - increment every clock
   timer->MR0 = value; // Setup match value

   if(repeat)
   {
      // Interrupt on match, reset on match
      timer->MCR = 0x03;
   }
   else
   {
      // Interrupt on match, stop on match
      timer->MCR = 0x05;
   }

   timer->IR |= 0xFFFFFFFF; // Clear all pending interrupts
   timer->TCR |= 0x01; // Enable timer
}

void EnableTimerIrq(IRQn_Type timerIrq, uint32_t priority)
{
   NVIC_SetPriority(timerIrq, priority);
   NVIC_EnableIRQ(timerIrq);
}

void DisableTimerIrq(IRQn_Type timerIrq)
{
   NVIC_DisableIRQ(timerIrq);
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

         SetupTimer(UART_RX_TIMER, UART_BITTIME, true);
         EnableTimerIrq(UART_RX_TIMER_IRQ, UART_RX_TIMER_PRI);
      }
      else
      {
         // Normally would subtract 1, but we want the stop bit too
         if(FtdiUart.currentRxBit == UART_DATA_BITS)
         {
            DisableTimerIrq(UART_RX_TIMER_IRQ);

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
void TIMER2_IRQHandler()
{
   if(LPC_TIM2->IR & TIMER_MATCH0_BIT)
   {
      LPC_TIM2->IR |= TIMER_MATCH0_BIT;

      if(FtdiUart.transmitting)
      {
         if(FtdiUart.currentTxBit <= 7)
         {
            if(FtdiUart.txData & (1<<FtdiUart.currentTxBit))
            {
               LPC_GPIO2->FIOSET |= UART_TX_PIN_BIT;
            }
            else
            {
               LPC_GPIO2->FIOCLR |= UART_TX_PIN_BIT;
            }

            FtdiUart.currentTxBit++;
         }
         // Stop bit
         else if(FtdiUart.currentTxBit == 8)
         {
            LPC_GPIO2->FIOSET |= UART_TX_PIN_BIT;
            FtdiUart.currentTxBit++;
         }
         else
         {
            FtdiUart.transmitting = false;
            DisableTimerIrq(UART_TX_TIMER_IRQ);
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
         SetupTimer(UART_RX_TIMER, UART_BITTIME / 2, false);
         EnableTimerIrq(UART_RX_TIMER_IRQ, UART_RX_TIMER_PRI);
      }
   }
}
