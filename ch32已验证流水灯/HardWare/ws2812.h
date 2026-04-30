#ifndef __WS2812_H
#define __WS2812_H

#include "ch32v30x.h"

/* 当前板子串了 8 颗 WS2812。 */
#define LED_NUM          8
#define PIXEL_NUM        LED_NUM

/* 在本工程的定时器配置下，1/0 码对应的比较值。 */
#define WS_H             75
#define WS_L             25
#define WS_HIGH          WS_H
#define WS_LOW           WS_L

/* 基础控制接口。 */
void WS2812_Init(void);
void WS2812_ClearAll(void);
void WS2812_Show(void);
void WS2812_Set(uint16_t num, uint8_t red, uint8_t green, uint8_t blue);
void WS2812_SetAll(uint8_t red, uint8_t green, uint8_t blue);

/* 兼容原 ws281x 风格的封装接口。 */
/* 初始化 WS2812 底层硬件，配置 PA8、TIM1_CH1 和 DMA。 */
void ws281x_init(void);

/* 关闭所有灯，全部熄灭。 */
void ws281x_closeAll(void);

/* 把 RGB 三个颜色值打包成 32 位颜色数据，内部顺序为 GRB。 */
uint32_t ws281x_color(uint8_t red, uint8_t green, uint8_t blue);

/* 按打包后的 GRB 颜色值设置第 n 颗灯的颜色。 */
void ws281x_setPixelColor(uint16_t n, uint32_t grb_color);

/* 按独立的 R、G、B 数值设置第 n 颗灯的颜色。 */
void ws281x_setPixelRGB(uint16_t n, uint8_t red, uint8_t green, uint8_t blue);

/* 刷新显示当前缓冲区中的颜色数据。 */
void ws281x_show(void);

/* 颜色依次从前往后点亮，形成铺开效果，wait 单位为 ms。 */
void ws281x_colorWipe(uint32_t c, uint8_t wait);

/* 整条灯带显示彩虹渐变效果，wait 控制变化速度。 */
void ws281x_rainbow(uint8_t wait);

/* 把完整彩虹铺到整条灯带上，并循环滚动显示。 */
void ws281x_rainbowCycle(uint8_t wait);

/* 跑马灯效果：间隔点亮部分灯珠并向前移动。 */
void ws281x_theaterChase(uint32_t c, uint8_t wait);

/* 彩虹跑马灯效果：移动时颜色也循环变化。 */
void ws281x_theaterChaseRainbow(uint8_t wait);

#endif