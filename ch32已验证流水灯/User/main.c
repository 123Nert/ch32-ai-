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

/* 演示顺序：全色闪烁 -> 红绿蓝流水。 */
static void demo_loop(void)
{
    fill_all(255, 0, 0, 250);
    fill_all(0, 255, 0, 250);
    fill_all(0, 0, 255, 250);
    fill_all(255, 255, 255, 250);

    running_light(255, 0, 0, 120);
    running_light(0, 255, 0, 120);
    running_light(0, 0, 255, 120);

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
