#ifndef __12864_NOFONT_H
#define __12864_NOFONT_H

#include "sys.h"
#include "stm32f10x.h"

// 引脚定义
#define LCD_DATA_PORT GPIOB
#define LCD_RS_PIN    GPIO_Pin_12
#define LCD_EN_PIN    GPIO_Pin_13
#define LCD_CS1_PIN   GPIO_Pin_14
#define LCD_CS2_PIN   GPIO_Pin_15

// 基本操作
//void LCD_WriteCmd(uint8_t cmd);
void LCD_WriteData(uint8_t data);
void LCD_Init(void);
void LCD_Clear(void);
void LCD_Write(uint8_t data, uint8_t is_data);
void LCD_SetPosition(uint8_t x, uint8_t y);

// 图形绘制
void LCD_SetPixel(uint8_t x, uint8_t y, uint8_t state);
void LCD_DrawChar(uint8_t x, uint8_t y, char ch);
void LCD_DrawString(uint8_t x, uint8_t y, char *str);
void LCD_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void LCD_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
	
#endif
