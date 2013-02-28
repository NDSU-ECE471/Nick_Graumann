#include <stdbool.h>
#include <stdio.h>
#include "lpc17xx.h"

#include "i2c.h"
#include "lcd/lcd.h"

#define BUTTON0_PORT LPC_GPIO0
#define BUTTON0_PIN  (1<<2)
#define BUTTON1_PORT LPC_GPIO0
#define BUTTON1_PIN  (1<<3)

void Delay1s()
{
   volatile uint32_t i=0;
   for(i=0; i<10000000UL; i++);
}

const char *const BLANK_LINE = "                    ";

int main ()
{
	SystemCoreClockUpdate();

	I2cInit();

	lcd_init();
	fillScreen(ST7735_16_BLACK);
	setColor16(ST7735_16_CYAN);
	setBackgroundColor16(ST7735_16_BLACK);
	drawString(10, 10, "EEPROM initialized.");
	drawString(10, 20, "Button 0: Write data");
	drawString(10, 30, "Button 1: Read data");

	while(1)
	{
	   if(!(BUTTON0_PORT->FIOPIN & BUTTON0_PIN))
	   {
	      drawString(10, 50, BLANK_LINE);
	      drawString(10, 60, BLANK_LINE);

	      const uint8_t writeData[] = { 0x00, 0x5E };
         if(I2cTx(0x50, writeData, sizeof(writeData)))
         {
            drawString(10, 50, "Write successful.");
            drawString(10, 60, "Wrote ");
            char str[5];
            snprintf(str, 5, "0x%2X", writeData[1]);
            drawString(10+6*6, 60, str);
         }
         else
         {
            drawString(10, 50, "Write failed.");
         }

         Delay1s();
	   }

	   if(!(BUTTON1_PORT->FIOPIN & BUTTON1_PIN))
	   {
	      drawString(10, 50, BLANK_LINE);
         drawString(10, 60, BLANK_LINE);

	      uint8_t data;
         if(I2cRx(0x50, 0x00, &data))
         {
            drawString(10, 50, "Read successful:");
            char str[5];
            snprintf(str, 5, "0x%2X", data);
            drawString(10, 60, str);
         }
         else
         {
            drawString(10, 50, "Read failed.");
         }

         Delay1s();
	   }
	}

	return 0;
}
