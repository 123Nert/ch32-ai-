#include "debug.h"
#include "ws2812.h"

/* 8 颗灯同时显示同一种颜色。 */
static void fill_all(uint8_t red, uint8_t green, uint8_t blue, uint16_t delay_ms)
{
    WS2812_SetAll(red, green, blue);
    WS2812_Show();
    Delay_Ms(delay_ms);
}

/* 单颗流水灯，从第 0 颗跑到第 7 颗。 */
static void running_light(uint8_t red, uint8_t green, uint8_t blue, uint16_t delay_ms)
{
    uint16_t step;

    for(step = 0; step < LED_NUM; ++step)
    {
        WS2812_SetAll(0, 0, 0);
        WS2812_Set(step, red, green, blue);
        WS2812_Show();
        Delay_Ms(delay_ms);
    }
}

/* 演示顺序：全色闪烁 -> 红绿蓝流水 -> 呼吸灯 -> 多彩效果。 */
static void demo_loop(void)
{
    fill_all(255, 0, 0, 250);
    fill_all(0, 255, 0, 250);
    fill_all(0, 0, 255, 250);
    fill_all(255, 255, 255, 250);

    running_light(255, 0, 0, 120);
    running_light(0, 255, 0, 120);
    running_light(0, 0, 255, 120);

    /* 红色呼吸灯效果 */
    ws281x_breatheAll(ws281x_color(255, 0, 0), 3);
    Delay_Ms(100);

    /* 绿色呼吸灯效果 */
    ws281x_breatheAll(ws281x_color(0, 255, 0), 3);
    Delay_Ms(100);

    /* 蓝色呼吸灯效果 */
    ws281x_breatheAll(ws281x_color(0, 0, 255), 3);
    Delay_Ms(100);

    /* 彩虹渐变效果 */
    ws281x_rainbow(10);

    /* 彩虹循环效果 */
    ws281x_rainbowCycle(10);

    /* 闪烁效果 */
    ws281x_blinkAll(ws281x_color(255, 100, 0), 5, 150);
    Delay_Ms(100);

    /* 色彩扫描效果 */
    ws281x_colorScan(80);
    Delay_Ms(100);

    /* 彩虹色随机变换 */
    ws281x_randomColor(50, 50);
    Delay_Ms(100);

    /* 星星闪烁效果 */
    ws281x_twinkle(30, 100);

    /* 红色流星效果 */
    ws281x_meteor(ws281x_color(255, 0, 0), 30, 50);
    Delay_Ms(100);

    /* 彩虹流星效果 */
    ws281x_meteorRainbow(30, 50);
    Delay_Ms(100);

    /* 心跳效果 */
    ws281x_heartbeat(ws281x_color(255, 0, 0), 8);
    Delay_Ms(200);

    /* 火焰效果 */
    ws281x_fire(55, 120, 50);
    Delay_Ms(100);

    /* 双色追逐效果 */
    ws281x_chase(ws281x_color(255, 0, 0), ws281x_color(0, 255, 0), 80);
    Delay_Ms(100);

    /* 波浪效果 */
    ws281x_wave(ws281x_color(0, 100, 255), 30);
    Delay_Ms(100);

    /* 警警闪（蓝红交替） */
    ws281x_police(6, 200);
    Delay_Ms(100);

    /* 彩虹彩条效果 */
    ws281x_rainbow_bands(15);

    /* 淡入淡出扫描 */
    ws281x_fade_scan(ws281x_color(255, 255, 0), 100);
    Delay_Ms(100);

    /* 多色流水效果 */
    ws281x_multicolor_running(100);
    Delay_Ms(100);

    /* 脉冲效果 */
    ws281x_pulse(15, 150);

    WS2812_ClearAll();
    Delay_Ms(150);
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);

    /* 初始化 PA8 + TIM1_CH1 + DMA1_Channel2 这条 WS2812 输出链路。 */
    WS2812_Init();
    printf("WS2812 8LED demo start\r\n");

    while(1)
    {
        /* 持续循环演示灯效。 */
        demo_loop();
    }
}
