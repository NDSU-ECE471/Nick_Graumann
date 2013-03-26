#include "LcdDisplay.h"


void LcdDrawPixel(LcdCoord x, LcdCoord y, LcdColor color)
{
   Output_Pixel(x, y, color);
}


void LcdDrawVLine(LcdCoord x, LcdCoord y, LcdCoord height, LcdColor color)
{
   for(LcdCoord h=0; h<=height; h++)
   {
      LcdDrawPixel(x, y+h, color);
   }
}

