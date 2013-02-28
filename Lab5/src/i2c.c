#include "LPC17xx.h"
#include "i2c.h"

#define I2C_SDA_PORT    LPC_GPIO1
#define I2C_SDA_PIN     (1<<19)

#define I2C_SCL_PORT    LPC_GPIO1
#define I2C_SCL_PIN     (1<<24)

#define I2C_SDA_OD_REG  PINMODE_OD1
#define I2C_SCL_OD_REG  PINMODE_OD1

__attribute__((always_inline)) static void DelayUs(uint32_t us)
{
   volatile unsigned int i;
   for(i=0; i<us*10; i++);
}


__attribute__((always_inline)) static void I2cDelay()
{
   DelayUs(60);
}


__attribute__((always_inline)) static void SdaHigh()
{
   I2C_SDA_PORT->FIOSET = I2C_SDA_PIN;
}


__attribute__((always_inline)) static void SdaLow()
{
   I2C_SDA_PORT->FIOCLR = I2C_SDA_PIN;
}


__attribute__((always_inline)) static uint8_t SdaRead()
{
   return !!(I2C_SDA_PORT->FIOPIN & I2C_SDA_PIN);
}


__attribute__((always_inline)) static void SdaSetOutput()
{
   I2C_SDA_PORT->FIODIR |= I2C_SDA_PIN;
}


__attribute__((always_inline)) static void SclHigh()
{
   I2C_SCL_PORT->FIOSET = I2C_SCL_PIN;
}


__attribute__((always_inline)) static void SclLow()
{
   I2C_SCL_PORT->FIOCLR = I2C_SCL_PIN;
}


__attribute__((always_inline)) static void SclSetOutput()
{
   I2C_SCL_PORT->FIODIR |= I2C_SCL_PIN;
}


__attribute__((always_inline)) static void SclClk()
{
   SclHigh();
   I2cDelay();
   SclLow();
}


void I2cSendStart()
{
   SdaLow();
   I2cDelay();
   SclLow();
}


void I2cSendStop()
{
   SdaLow();
   I2cDelay();
   SclHigh();
   I2cDelay();
   SdaHigh();
   I2cDelay();
}


uint8_t I2cGet8(uint8_t *ack)
{
   uint8_t data = 0;
   for(int8_t i=7; i>=0; i--)
   {
      I2cDelay();

      SclClk();

      if(SdaRead())
      {
         data |= 1<<i;
      }
   }

   // Clock in ack bit
   I2cDelay();
   SclHigh();

   // Save ack bit if requested
   if(ack)
   {
      // ACK bit received if SDA is low
      *ack = !SdaRead();
   }
   I2cDelay();
   SclLow();

   return data;
}


void I2cSend8(uint8_t data, uint8_t *ack)
{
   for(int8_t i=7; i>=0; i--)
   {
      if(data & (1<<i))
      {
         SdaHigh();
      }
      else
      {
         SdaLow();
      }

      I2cDelay();

      SclClk();
   }

   SdaHigh();
   I2cDelay();

   SclHigh();
   I2cDelay();

   // Save ack bit if requested
   if(ack)
   {
      // ACK bit received if SDA is low
      *ack = !SdaRead();
   }

   SclLow();
}


void I2cInit()
{
   SdaSetOutput();
   SclSetOutput();

   // Setup both outputs as open-drain
   LPC_PINCON->I2C_SDA_OD_REG |= I2C_SDA_PIN;
   LPC_PINCON->I2C_SCL_OD_REG |= I2C_SCL_PIN;

   // Bus is initially high
   SdaHigh();
   SclHigh();
}


bool I2cRx(uint8_t address, uint8_t reg, uint8_t *buffer)
{
   if(!buffer)
   {
      return false;
   }

   I2cSendStart();

   uint8_t ack;

   // Send address and look for ACK
   I2cSend8(address<<1, &ack);
   if(!ack)
   {
      return false;
   }

   I2cDelay();
   I2cDelay();

   I2cSend8(reg, &ack);
   if(!ack)
   {
      return false;
   }

   I2cSendStop();
   I2cDelay();
   I2cDelay();
   I2cSendStart();

   I2cSend8(address<<1 | 0x01, &ack);
   if(!ack)
   {
      return false;
   }

   I2cDelay();
   I2cDelay();

   *buffer = I2cGet8(&ack);

   I2cSendStop();

   //return ack;
   return true;
}


bool I2cTx(uint8_t address, const uint8_t *data, uint8_t size)
{
   if(!data || !size)
   {
      return false;
   }

   I2cSendStart();

   uint8_t ack;

   // Send address and look for ACK
   I2cSend8(address<<1, &ack);
   if(!ack)
   {
      return false;
   }

   for(uint8_t i=0; i<size; i++)
   {
      I2cDelay();
      I2cDelay();

      I2cSend8(data[i], &ack);
      if(!ack)
      {
         return false;
      }
   }

   I2cSendStop();
   return true;
}

