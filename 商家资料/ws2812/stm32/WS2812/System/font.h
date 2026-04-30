#ifndef __FONT_H
#define __FONT_H

#include <stdint.h>  // 在头文件中也包含

extern const uint8_t Font6x8[8][6];
extern const uint8_t ChineseFont[16][32];

void LCD_DrawChinese(int x0,int x1,int x2);

#endif
