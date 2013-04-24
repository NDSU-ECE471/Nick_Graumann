#include "LPC17xx.h"
#include "SSP_LPC.h"



SPI_Error_E SSP0_Enable()
{
   // Enable the SSP0 peripheral
   LPC_SC->PCONP |= (1<<21);

   // Setup clock divider clk/4
   LPC_SC->PCLKSEL1 &= ~(0x03<<10);
   // clk/2
   LPC_SC->PCLKSEL1 |= (2<<10);

   // Setup pins
   // P0[15]->SCK0
   LPC_PINCON->PINSEL0 &= ~(0x03<<30);
   LPC_PINCON->PINSEL0 |= (0x02<<30);

   LPC_PINCON->PINMODE0 &= ~(0x03<<30);
   LPC_PINCON->PINMODE0 |= (0x02<<30);

   // P0[17]->MISO0
   LPC_PINCON->PINSEL1 &= ~(0x03<<2);
   LPC_PINCON->PINSEL1 |= (0x02<<2);

   LPC_PINCON->PINMODE1 &= ~(0x03<<2);
   LPC_PINCON->PINMODE1 |= (0x02<<2);

   // P0[16]->SSEL0
   LPC_PINCON->PINSEL1 &= ~(0x03<<0);
   LPC_PINCON->PINSEL1 |= (0x02<<0);

   LPC_PINCON->PINMODE1 &= ~(0x03<<0);
   LPC_PINCON->PINMODE1 |= (0x02<<0);

   // 16-bit transfers
   LPC_SSP0->CR0 = 0x0F;

   // CPOL=1, CPHA=0
   LPC_SSP0->CR0 |= (1<<6);

   // Clock rate
   // SCR=0, CPSR=2 (pclk/2)
   LPC_SSP0->CPSR = 0x2;

   // Flush data
   volatile uint8_t trash=trash;
   for(uint32_t i=0; i<LPC_SSP_FIFO_SIZE; i++)
   {
      trash = LPC_SSP0->DR;
   }

   // Enable
   LPC_SSP0->CR1 = (1<<1);

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
      while ((LPC_SSP0->SR & ((1<<1)|(1<<4))) != (1<<1));

      // Clock out dummy data
      LPC_SSP0->DR = 0;

      // Wait for send complete
      while(((LPC_SSP0->SR) & (1<<4)));

      // Wait for data received
      while(!((LPC_SSP0->SR) & (1<<2)));

      *buf = (LPC_SSP0->DR & 0x0000FFFF);
   }

   return err;
}

