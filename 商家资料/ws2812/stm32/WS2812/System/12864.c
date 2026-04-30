#include "12864.h"
#include "font.h"  // 自定义字库
#include "delay.h"

// 初始化GPIO
void LCD_Init(){
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    // 配置控制引脚
    GPIO_InitStruct.GPIO_Pin = LCD_RS_PIN | LCD_EN_PIN | LCD_CS1_PIN | LCD_CS2_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // 配置数据总线PB0~PB7
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_4 | 
                              GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8;
    GPIO_Init(LCD_DATA_PORT, &GPIO_InitStruct);
}

// 写命令/数据（时序严格遵循KS0108手册）
void LCD_Write(uint8_t data, uint8_t is_data) {
    GPIO_WriteBit(GPIOB, LCD_RS_PIN, is_data ? Bit_SET : Bit_RESET);
    GPIO_Write(LCD_DATA_PORT, data);
    GPIO_WriteBit(GPIOB, LCD_EN_PIN, Bit_SET);
    Delay_us(1);
    GPIO_WriteBit(GPIOB, LCD_EN_PIN, Bit_RESET);
    Delay_us(1);
}

/**
  * @brief  向LCD写入数据
  * @param  data: 要写入的数据（8位）
  * @retval 无
  */
void LCD_WriteData(uint8_t data) {
    /* 1. 设置RS为高电平（数据模式） */
    GPIO_SetBits(LCD_DATA_PORT, LCD_RS_PIN);
    
    /* 2. 输出数据到数据总线（PB0~PB7） */
    GPIO_Write(LCD_DATA_PORT, data);
    
    /* 3. 产生使能脉冲（下降沿触发） */
    GPIO_SetBits(LCD_DATA_PORT, LCD_EN_PIN);  // EN=1
    Delay_us(1);                           // 保持至少1μs（KS0108时序要求）
    GPIO_ResetBits(LCD_DATA_PORT, LCD_EN_PIN); // EN=0
    
    /* 4. 短暂延时满足时序 */
    Delay_us(1); 
}

// 设置显示位置（x:0-127, y:0-7）
void LCD_SetPosition(uint8_t x, uint8_t y) {
    uint8_t chip = (x < 64) ? 0 : 1;
    GPIO_WriteBit(GPIOB, LCD_CS1_PIN, chip ? Bit_RESET : Bit_SET);
    GPIO_WriteBit(GPIOB, LCD_CS2_PIN, chip ? Bit_SET : Bit_RESET);
    LCD_Write(0xB8 | (y & 0x07), 0);  // 行地址
    LCD_Write(0x40 | (x % 64), 0);    // 列地址
}

// 清屏
void LCD_Clear(void) {
    for(uint8_t y=0; y<8; y++) {
        LCD_SetPosition(0, y);
        for(uint8_t x=0; x<128; x++) {
            LCD_Write(0x00, 1);
        }
    }
}

// 画点（基于像素坐标）
void LCD_SetPixel(uint8_t x, uint8_t y, uint8_t state) {
    static uint8_t page_buffer[8][128] = {0}; // 显存缓存
    if(x >= 128 || y >= 64) return;
    
    uint8_t page = y / 8;
    uint8_t bit_mask = 1 << (y % 8);
    
    if(state) page_buffer[page][x] |= bit_mask;
    else page_buffer[page][x] &= ~bit_mask;
    
    LCD_SetPosition(x, page);
    LCD_Write(page_buffer[page][x], 1);
}


// 绘制ASCII字符（6x8像素）
void LCD_DrawChar(uint8_t x, uint8_t y, char ch) {
    if (ch < 32 || ch > 127) return; // 只支持可打印ASCII
    
    const uint8_t *font_ptr = &Font6x8[(ch - 32)][0]; // 指向字模数据
    
    for (uint8_t col = 0; col < 6; col++) {
        LCD_SetPosition(x + col, y);
        LCD_WriteData(font_ptr[col]);
    }
}

// 绘制字符串
void LCD_DrawString(uint8_t x, uint8_t y, char *str) {
    while (*str) {
        LCD_DrawChar(x, y, *str++);
        x += 6; // 字符宽度为6像素
        if (x >= 122) { // 换行检查
            x = 0;
            y++;
        }
    }
}

// 绘制直线（Bresenham算法）
void LCD_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;
    
    while (1) {
        LCD_SetPixel(x0, y0, 1);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// 绘制矩形
void LCD_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h) {
    LCD_DrawLine(x, y, x + w, y);         // 上边
    LCD_DrawLine(x, y + h, x + w, y + h); // 下边
    LCD_DrawLine(x, y, x, y + h);         // 左边
    LCD_DrawLine(x + w, y, x + w, y + h); // 右边
}
