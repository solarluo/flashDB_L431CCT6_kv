#ifndef __LCD_TEXT_H
#define __LCD_TEXT_H

#include "st7735.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LCD_TEXT_OPAQUE       0   // 实心背景
#define LCD_TEXT_TRANSPARENT  1   // 透明背景

#define LCD_FONT_WIDTH        8
#define LCD_FONT_HEIGHT       16

void LCD_ShowChar(uint16_t x, uint16_t y, char ch, uint16_t color, uint16_t bgcolor);
void LCD_ShowCharEx(uint16_t x, uint16_t y, char ch, uint16_t color, uint16_t bgcolor, uint8_t transparent);

void LCD_ShowString(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bgcolor);
void LCD_ShowStringEx(uint16_t x, uint16_t y, const char *str, uint16_t color, uint16_t bgcolor, uint8_t transparent);

void LCD_ShowStringInArea(uint16_t x, uint16_t y,
                          uint16_t area_w, uint16_t area_h,
                          const char *str,
                          uint16_t color, uint16_t bgcolor,
                          uint8_t transparent);

void LCD_ClearLine(uint16_t y, uint16_t bgcolor);
void LCD_ClearArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t bgcolor);

void LCD_ShowInt(uint16_t x, uint16_t y, int value, uint16_t color, uint16_t bgcolor);
void LCD_ShowUInt(uint16_t x, uint16_t y, uint32_t value, uint16_t color, uint16_t bgcolor);

void LCD_ShowCenteredString(uint16_t y, const char *str, uint16_t color, uint16_t bgcolor);

uint16_t LCD_GetStringWidth(const char *str);

#ifdef __cplusplus
}
#endif

#endif