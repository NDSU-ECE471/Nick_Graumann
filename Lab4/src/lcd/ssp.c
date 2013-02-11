// SSP Basic SPI functions
// Date: 9/1/2012

#include <stdbool.h>
#include "LPC17xx.h"
#include "ssp.h"
#include "ports.h"

#include "../lab4.h"

// Software-based SPI implementation
// Just replaced function bodies, this doesn't actually use the SSP

volatile SwSPI LcdSPI =
{
      .currentTxBit = 0,
      .state = STATE_IDLE,
      .txData = 0
};

__attribute__ ((interrupt))
void TIMER1_IRQHandler()
{
   if(LPC_TIM1->IR & TIMER_MATCH0_BIT)
   {
      LPC_TIM1->IR |= TIMER_MATCH0_BIT;

      switch(LcdSPI.state)
      {
      case STATE_IDLE:
         break;

      case STATE_TX_CS_LOW:
         SetupTimer(LCD_SPI_TX_TIMER, LCD_SPI_BITTIME, true);
         EnableTimerIrq(LCD_SPI_TX_TIMER_IRQ, LCD_SPI_TX_TIMER_PRI);
         LcdSPI.state = STATE_TX_DATA;
         break;

      case STATE_TX_DATA:
         // Need to check for high clk (to change data) or if it's before the first clk of transmitting
         if(LPC_GPIO0->FIOPIN & LCD_SPI_SCLK_PIN || LcdSPI.currentTxBit == 7)
         {
            // Change data on falling edge so it's always 1/2 a cycle ahead of the clk
            if(LcdSPI.txData & (1<<LcdSPI.currentTxBit))
            {
               LPC_GPIO0->FIOSET |= LCD_SPI_MOSI_PIN;
            }
            else
            {
               LPC_GPIO0->FIOCLR |= LCD_SPI_MOSI_PIN;
            }

            LPC_GPIO0->FIOCLR |= LCD_SPI_SCLK_PIN;

            if(LcdSPI.currentTxBit > 0)
            {
               LcdSPI.currentTxBit--;
            }
            else
            {
               LcdSPI.state = STATE_TX_LAST_CLK;
            }
         }
         else // rising edge of clk
         {
            LPC_GPIO0->FIOSET |= LCD_SPI_SCLK_PIN;
         }
         break;

      case STATE_TX_LAST_CLK:
         LPC_GPIO0->FIOSET |= LCD_SPI_SCLK_PIN;
         LcdSPI.state = STATE_TX_DATA_CLR;
         break;

      case STATE_TX_DATA_CLR:
         LPC_GPIO0->FIOCLR |= LCD_SPI_SCLK_PIN;
         LPC_GPIO0->FIOCLR |= LCD_SPI_MOSI_PIN;
         LcdSPI.state = STATE_TX_CS_HIGH;
         break;

      case STATE_TX_CS_HIGH:
         DisableTimerIrq(LCD_SPI_TX_TIMER_IRQ);
         LcdSPI.state = STATE_IDLE;
         LPC_GPIO1->FIOSET |= LCD_SPI_CS_PIN;
         break;
      }
   }
}

void SSP1Init( void )
{
	LPC_GPIO0->FIODIR |= LCD_SPI_MOSI_PIN | LCD_SPI_SCLK_PIN;
	LPC_GPIO1->FIODIR |= LCD_SPI_CS_PIN;

	// CS initially high
	LPC_GPIO1->FIOSET |= LCD_SPI_CS_PIN;
}

void SSP1Send(uint8_t buf)
{
   // Wait for any previous transmit to complete
   while(LcdSPI.state != STATE_IDLE);

   LcdSPI.currentTxBit = 7;
   LcdSPI.state = STATE_TX_CS_LOW;
   LcdSPI.txData = buf;

   LPC_GPIO1->FIOCLR |= LCD_SPI_CS_PIN;

   SetupTimer(LCD_SPI_TX_TIMER, 4*LCD_SPI_BITTIME, false);
   EnableTimerIrq(LCD_SPI_TX_TIMER_IRQ, LCD_SPI_TX_TIMER_PRI);

   LPC_GPIO0->FIOCLR |= LCD_SPI_SCLK_PIN;
}
