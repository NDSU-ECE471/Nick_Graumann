#include "LcdDisplay.h"


void LcdDrawPixel(LcdCoord x, LcdCoord y, LcdColor color)
{
   Output_Pixel(x, y, color);
}


void LcdDrawRectangle(LcdCoord x, LcdCoord y, LcdCoord width, LcdCoord height, LcdColor color)
{
   for(LcdCoord yi=y; yi<height; yi++)
   {
      for(LcdCoord xi=x; xi<width; xi++)
      {
         LcdDrawPixel(xi, yi, color);
      }
   }
}


void LcdDrawHLine(LcdCoord x, LcdCoord y, LcdCoord width, LcdColor color)
{
   for(LcdCoord w=0; w<width; w++)
   {
      LcdDrawPixel(x+w, y, color);
   }
}


void LcdDrawVLine(LcdCoord x, LcdCoord y, LcdCoord height, LcdColor color)
{
   for(LcdCoord h=0; h<height; h++)
   {
      LcdDrawPixel(x, y+h, color);
   }
}

