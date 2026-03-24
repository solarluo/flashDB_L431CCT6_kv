#ifndef __ST7735_H
#define __ST7735_H

#include "main.h"
#include <stdint.h>

/* 0.96寸 ST7735S 常见分辨率 */
#define ST7735_WIDTH    80
#define ST7735_HEIGHT   160

/* RGB565 颜色 */
#define ST7735_BLACK    0x0000
#define ST7735_BLUE     0x001F
#define ST7735_RED      0xF800
#define ST7735_GREEN    0x07E0
#define ST7735_CYAN     0x07FF
#define ST7735_MAGENTA  0xF81F
#define ST7735_YELLOW   0xFFE0
#define ST7735_WHITE    0xFFFF

extern SPI_HandleTypeDef hspi2;

void ST7735_Init(void);
void ST7735_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void ST7735_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ST7735_FillScreen(uint16_t color);
void ST7735_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

#endif