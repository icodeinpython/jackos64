#pragma once
#include <stdint.h>

void initScreen();
void setColorRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void setColor(uint32_t color);
void setPixel(uint32_t x, uint32_t y, uint32_t color);
void drawChar(uint32_t x, uint32_t y, char c, uint32_t color);
void clearScreen();
