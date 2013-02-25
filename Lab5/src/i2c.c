#include "LPC17xx.h"
#include "i2c.h"

__attribute__((always_inline)) static void DelayMs(uint32_t ms)
{
   volatile unsigned int i;
   for(i = 0; i < ms*10000; i++);
}


__attribute__((always_inline)) static void I2cDelay()
{
   // Standard I2C delay (from lab info)
   DelayMs(2);
}


__attribute__((always_inline)) static void SdaHigh()
{
   LPC_GPIO1->FIOSET = I2C_SDA_PIN;
}


__attribute__((always_inline)) static void SdaLow()
{
   LPC_GPIO1->FIOCLR = I2C_SDA_PIN;
}


__attribute__((always_inline)) static uint8_t SdaGet()
{
   return !!(LPC_GPIO1->FIOPIN & I2C_SDA_PIN);
}


__attribute__((always_inline)) static void SclHigh()
{
   LPC_GPIO1->FIOSET = I2C_SCL_PIN;
}


__attribute__((always_inline)) static void SclLow()
{
   LPC_GPIO1->FIOCLR = I2C_SCL_PIN;
}


__attribute__((always_inline)) static void SclClk()
{
   SclHigh();
   I2cDelay();
   SclLow();
}


void I2cStart()
{
   SdaHigh();
   I2cDelay();

   SclClk();

   I2cDelay();
   SclLow();
}


void I2cStop()
{
   SdaLow();
   I2cDelay();
   SclHigh();
   I2cDelay();
   SdaHigh();
   I2cDelay();
}


void I2cSend8(uint8_t data)
{
   for(uint8_t i=8; i>0; i--)
   {
      // Transmit MSB->LSB
      if(data & (1<<i))
      {
         SdaHigh();
      }
      else
      {
         SdaLow();
      }

      SclClk();
   }

   SdaHigh();
   SclClk();
}


uint8_t I2cGet8(bool ack)
{
   uint8_t data = 0;

   SdaHigh();

   for(uint8_t i=8; i>0; i--)
   {
      SclHigh();
      I2cDelay();
      data |= (SdaGet() << i);
      SclLow();
   }

   if(ack)
   {
      SdaLow();
   }
   else
   {
      SdaHigh();
   }

   SclHigh();
   I2cDelay();
   SclLow();
   SdaHigh();

   return data;
}


void I2cInit()
{
   LPC_GPIO1->FIODIR |= I2C_SDA_PIN;
   LPC_GPIO1->FIODIR |= I2C_SCL_PIN;
}


bool I2cRead(uint8_t address, uint8_t *buffer)
{
   if(!buffer)
   {
      return false;
   }

   I2cStart();
   I2cSend8(address<<1);
   //todo: more to add here
}


void I2cWrite(uint8_t address, uint8_t *data, uint8_t size)
{
   I2cStart();
   I2cSend8(address<<1);
   for(uint8_t i=0; i<size; i++)
   {
      I2cSend8(data[i]);
   }
   I2cStop();
}

