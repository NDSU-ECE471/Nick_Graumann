#include "lpc17xx_pinsel.h"
#include "lpc17xx_uart.h"

#include "../UART_Facade.h"

UART_Error_E UART_Open(UART_Dev_E device, UART_BaudRate baud)
{
   UART_Error_E err = UART_SUCCESS;

   do
   {
      if(device != UART_1)
      {
         err = UART_INVALID_DEVICE;
         break;
      }

      PINSEL_CFG_Type pincfg;

      // Setup Tx pin
      pincfg.Funcnum = PINSEL_FUNC_2;
      pincfg.Portnum = 2;
      pincfg.Pinnum = 0;
      pincfg.Pinmode = PINSEL_PINMODE_PULLUP;
      PINSEL_ConfigPin(&pincfg);

      // Rx pin
      pincfg.Portnum = 2;
      pincfg.Pinnum = 1;
      PINSEL_ConfigPin(&pincfg);

      UART_CFG_Type UART_ConfigData;
      UART_ConfigStructInit(&UART_ConfigData);

      UART_ConfigData.Baud_rate = baud;

      UART_Init((LPC_UART_TypeDef *)LPC_UART1, &UART_ConfigData);

      UART_FIFO_CFG_Type UART_FIFO_ConfigData;
      UART_FIFOConfigStructInit(&UART_FIFO_ConfigData);
      UART_FIFOConfig((LPC_UART_TypeDef *)LPC_UART1, &UART_FIFO_ConfigData);

      UART_TxCmd((LPC_UART_TypeDef *)LPC_UART1, ENABLE);
   }
   while(0);

   return err;
}


UART_Error_E UART_Close(UART_Dev_E device)
{
   UART_Error_E err = UART_SUCCESS;

   if(device != UART_1)
   {
      err = UART_INVALID_DEVICE;
   }
   else
   {
      UART_DeInit((LPC_UART_TypeDef *)LPC_UART1);
   }

   return err;
}


UART_Error_E UART_Rx(UART_Dev_E device, void *buf, uint32_t size, UART_BlockType_E shouldBlock)
{
   UART_Error_E err = UART_SUCCESS;

   do
   {
      if(device != UART_1)
      {
         err = UART_INVALID_DEVICE;
         break;
      }

      LPC_UART_TypeDef *devPtr = (LPC_UART_TypeDef *)LPC_UART1;
      if(UART_Receive(devPtr, (uint8_t *)buf, size, shouldBlock == UART_NON_BLOCKING ? NONE_BLOCKING : BLOCKING) != size)
      {
         err = UART_XFER_FAILED;
      }
   }
   while(0);

   return err;
}


UART_Error_E UART_Tx(UART_Dev_E device, const void *buf, uint32_t size, UART_BlockType_E shouldBlock)
{
   UART_Error_E err = UART_SUCCESS;

   do
   {
      if(device != UART_1)
      {
         err = UART_INVALID_DEVICE;
         break;
      }

      LPC_UART_TypeDef *devPtr = (LPC_UART_TypeDef *)LPC_UART1;
      if(UART_Send(devPtr, (uint8_t *)buf, size, shouldBlock) == UART_NON_BLOCKING ? NONE_BLOCKING : BLOCKING != size)
      {
         err = UART_XFER_FAILED;
      }
   }
   while(0);

   return err;
}

