#ifndef __LCD_DISPLAY_H__
#define __LCD_DISPLAY_H__

#include <stdint.h>
#include "lcd/lcd.h"

#define LCD_WIDTH    JDT1800_WIDTH
#define LCD_HEIGHT   JDT1800_HEIGHT

#define LCD_COLOR_BLACK    ST7735_16_BLACK
#define LCD_COLOR_BLUE     ST7735_16_BLUE
#define LCD_COLOR_RED      ST7735_16_RED
#define LCD_COLOR_ORANGE   ST7735_16_ORANGE
#define LCD_COLOR_ORANGE2  ST7735_16_ORANGE2
#define LCD_COLOR_GREEN    ST7735_16_GREEN
#define LCD_COLOR_CYAN     ST7735_16_CYAN
#define LCD_COLOR_MAGENTA  ST7735_16_MAGENTA
#define LCD_COLOR_YELLOW   ST7735_16_YELLOW
#define LCD_COLOR_WHITE    ST7735_16_WHITE

typedef int32_t LcdCoord;
typedef uint16_t LcdColor;

void LcdDrawPixel(LcdCoord x, LcdCoord y, LcdColor color);

void LcdDrawRectangle(LcdCoord x, LcdCoord y, LcdCoord width, LcdCoord height, LcdColor color);
void LcdDrawHLine(LcdCoord x, LcdCoord y, LcdCoord width, LcdColor color);
void LcdDrawVLine(LcdCoord x, LcdCoord y, LcdCoord height, LcdColor color);

#endif //__LCD_DISPLAY_H__
