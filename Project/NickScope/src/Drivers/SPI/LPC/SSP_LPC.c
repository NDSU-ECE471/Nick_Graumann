#include "LPC17xx.h"
#include "SSP_LPC.h"

#define LPC_SSP_FIFO_SIZE  8

// SC PCONP register
#define SSP0_ENABLE     (1<<21)

// SSPx CR0 register
#define SSP_TX_SIZE_16     (0x0F<<0)
#define SSP_CPOL           (1<<6)
#define SSP_CPHA           (1<<7)
#define SSP_CLK_RATE_1     (0<<8)

// SSPx CR1 register
#define SSP_ENABLE         (1<<1)

// SSPx SR register
#define SSP_TX_FIFO_EMPTY  (1<<0)
#define SSP_TX_FIFO_NFULL  (1<<1)
#define SSP_RX_FIFO_NEMPTY (1<<2)
#define SSP_RX_FIFO_FULL   (1<<3)
#define SSP_BUSY           (1<<4)

// Pin configuration
#define SSP0_SCK_BITS   (0x03<<30)
#define SSP0_SCK_SEL    (0x02<<30)
#define SSP0_SCK_MODE   (0x02<<30)

#define SSP0_MISO_BITS  (0x03<<2)
#define SSP0_MISO_SEL   (0x02<<2)
#define SSP0_MISO_MODE  (0x02<<2)

#define SSP0_SSEL_BITS  (0x03<<0)
#define SSP0_SSEL_SEL   (0x02<<0)
#define SSP0_SSEL_MODE  (0x02<<0)

#define PCLK_DIV_BITS   (0x03<<10)
#define PCLK_DIV_1      (0x01<<10)
#define PCLC_DIV_2      (0x02<<10)
#define PCLK_DIV_4      (0x03<<10)

// CPSR register
#define SSP0_PRESCALE_2 0x02

// CR1 register


SPI_Error_E SSP0_Enable()
{
   // Enable the SSP0 peripheral
   LPC_SC->PCONP |= SSP0_ENABLE;

   // Setup clock divider
   LPC_SC->PCLKSEL1 &= ~PCLK_DIV_BITS;
   LPC_SC->PCLKSEL1 |= PCLK_DIV_1;

   // Setup pins
   // P0[15]->SCK0
   LPC_PINCON->PINSEL0 &= ~SSP0_SCK_BITS;
   LPC_PINCON->PINSEL0 |= SSP0_SCK_SEL;

   LPC_PINCON->PINMODE0 &= ~SSP0_SCK_BITS;
   LPC_PINCON->PINMODE0 |= SSP0_SCK_MODE;

   // P0[17]->MISO0
   LPC_PINCON->PINSEL1 &= ~SSP0_MISO_BITS;
   LPC_PINCON->PINSEL1 |= SSP0_MISO_SEL;

   LPC_PINCON->PINMODE1 &= ~SSP0_MISO_BITS;
   LPC_PINCON->PINMODE1 |= SSP0_MISO_MODE;

   // P0[16]->SSEL0
   LPC_PINCON->PINSEL1 &= ~SSP0_SSEL_BITS;
   LPC_PINCON->PINSEL1 |= SSP0_SSEL_SEL;

   LPC_PINCON->PINMODE1 &= ~SSP0_SSEL_BITS;
   LPC_PINCON->PINMODE1 |= SSP0_SSEL_MODE;

   // Clock rate, polarity, transfer size
   LPC_SSP0->CR0 = SSP_CLK_RATE_1 | SSP_CPOL | SSP_TX_SIZE_16;

   // Clock prescaler
   LPC_SSP0->CPSR = SSP0_PRESCALE_2;

   // Flush data
   volatile uint8_t trash=trash;
   for(uint32_t i=0; i<LPC_SSP_FIFO_SIZE; i++)
   {
      trash = LPC_SSP0->DR;
   }

   // Enable
   LPC_SSP0->CR1 = SSP_ENABLE;

   return SPI_SUCCESS;
}


SPI_Error_E SSP0_Receive(uint16_t *buf)
{
   SPI_Error_E err = SPI_SUCCESS;

   if(!buf)
   {
      err = SPI_INVALID_PARAMETER;
   }
   else
   {
      // Ensure module is not in use
      while ((LPC_SSP0->SR & (SSP_TX_FIFO_NFULL|SSP_BUSY)) != SSP_TX_FIFO_NFULL);

      // Clock out dummy data
      LPC_SSP0->DR = 0;

      // Wait for send complete
      while(((LPC_SSP0->SR) & SSP_BUSY));

      // Wait for data received
      while(!((LPC_SSP0->SR) & SSP_RX_FIFO_NEMPTY));

      // Mask off data size
      *buf = (LPC_SSP0->DR & 0x0000FFFF);
   }

   return err;
}

