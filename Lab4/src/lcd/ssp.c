// SSP Basic SPI functions
// Date: 9/1/2012

#include <stdbool.h>
#include "LPC17xx.h"
#include "ssp.h"
#include "ports.h"

// Software-based SPI implementation
// Just replaced function bodies, this doesn't actually use the SSP

// Bittime = Timer clk/spi rate; run SPI at 1MHz (timer clk is 100MHz/4)
#define SPI_BITTIME 25

#define LCD_SPI_MOSI_PIN   (1<<9)
#define LCD_SPI_SCLK_PIN   (1<<7)
#define LCD_SPI_CS_PIN     (1<<18)

#define TIMER_MATCH0_BIT ((1<<0))

void DisableTimer1();

typedef enum
{
   STATE_IDLE,
   STATE_TX_CS_LOW,
   STATE_TX_DATA,
   STATE_TX_LAST_CLK,
   STATE_TX_DATA_CLR,
   STATE_TX_CS_HIGH
} SwSPIState;

typedef struct
{
   // "Public" members
   uint8_t txData;
   SwSPIState state;

   // "Private" members
   uint8_t currentTxBit;
} SwSPI;

volatile SwSPI LcdSPI =
{
      .currentTxBit = 0,
      .state = STATE_IDLE,
      .txData = 0
};

void SetupTimer1(uint32_t value, bool repeat)
{
   DisableTimer1();

   // Timer mode
   LPC_TIM1->CTCR = 0;

   // Reset values
   LPC_TIM1->TCR |= (1<<1);
   LPC_TIM1->TCR &= ~(1<<1);

   // No prescale - increment every clock
   LPC_TIM1->PR = 0;

   // Setup match value
   LPC_TIM1->MR0 = value;

   if(repeat)
   {
      // Interrupt on match, reset on match
      LPC_TIM1->MCR = 0x03;
   }
   else
   {
      // Interrupt on match, stop on match
      LPC_TIM1->MCR = 0x05;
   }

   // Clear all interrupts
   LPC_TIM1->IR |= 0xFFFFFFFF;

   // Enable timer
   LPC_TIM1->TCR |= 0x01;

   NVIC_SetPriority(TIMER1_IRQn, 2);
   // Enable interrupt for timer 0
   NVIC_EnableIRQ(TIMER1_IRQn);
}

void DisableTimer1()
{
   NVIC_DisableIRQ(TIMER1_IRQn);
}

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
         SetupTimer1(SPI_BITTIME, true);
         LcdSPI.state = STATE_TX_DATA;
         break;

      case STATE_TX_DATA:
         // Need to check for high clk (to change data) or if it's before the first clk of transmitting
         if(LPC_GPIO0->FIOPIN & LCD_SPI_SCLK_PIN || LcdSPI.currentTxBit == 7)
         {
            // Change data on falling edge so it's always 1/2 a cycle ahead of the clk
            if(LcdSPI.txData & 1<<LcdSPI.currentTxBit)
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
         DisableTimer1();
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

   SetupTimer1(4*SPI_BITTIME, false);

   LPC_GPIO0->FIOCLR |= LCD_SPI_SCLK_PIN;
}
