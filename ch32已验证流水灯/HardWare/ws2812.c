#include "debug.h"
#include "ws2812.h"

#define DATA_LEN        24
#define WS2812_RST_NUM  60
#define DATA_BUFF_LEN   (LED_NUM * DATA_LEN + WS2812_RST_NUM)

/* DMA 直接把这个缓冲区搬到 TIM1_CH1 比较寄存器，形成 WS2812 波形。 */
static uint32_t pixel_buffer[DATA_BUFF_LEN] = {0};

static uint32_t ws281x_wheel(uint8_t wheel_pos);

void WS2812_Init(void)
{
    GPIO_InitTypeDef gpio_init_structure = {0};
    TIM_OCInitTypeDef tim_oc_init_structure = {0};
    TIM_TimeBaseInitTypeDef tim_base_init_structure = {0};
    DMA_InitTypeDef dma_init_structure = {0};

    /* 上电后稍等一下，避免灯珠和电源还没稳定就开始发数据。 */
    Delay_Ms(500);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_TIM1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    /* WS2812 数据输出脚：PA8，对应 TIM1_CH1。 */
    gpio_init_structure.GPIO_Pin = GPIO_Pin_8;
    gpio_init_structure.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio_init_structure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_init_structure);

    /* 这里沿用工程原始周期参数，通过 PWM 占空比拼出 WS2812 的 0/1 时序。 */
    tim_base_init_structure.TIM_Period = 119;
    tim_base_init_structure.TIM_Prescaler = 0;
    tim_base_init_structure.TIM_ClockDivision = 0;
    tim_base_init_structure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &tim_base_init_structure);

    tim_oc_init_structure.TIM_OCMode = TIM_OCMode_PWM1;
    tim_oc_init_structure.TIM_OutputState = TIM_OutputState_Enable;
    tim_oc_init_structure.TIM_OutputNState = TIM_OutputNState_Disable;
    tim_oc_init_structure.TIM_OCPolarity = TIM_OCPolarity_High;
    tim_oc_init_structure.TIM_OCNPolarity = TIM_OCNPolarity_High;
    tim_oc_init_structure.TIM_Pulse = 0;
    TIM_OC1Init(TIM1, &tim_oc_init_structure);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);

    /* DMA 把 pixel_buffer 的每个元素依次写到 TIM1->CH1CVR。 */
    DMA_DeInit(DMA1_Channel2);
    dma_init_structure.DMA_PeripheralBaseAddr = (uint32_t)&(TIM1->CH1CVR);
    dma_init_structure.DMA_MemoryBaseAddr = (uint32_t)pixel_buffer;
    dma_init_structure.DMA_DIR = DMA_DIR_PeripheralDST;
    dma_init_structure.DMA_BufferSize = DATA_BUFF_LEN;
    dma_init_structure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dma_init_structure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma_init_structure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    dma_init_structure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    dma_init_structure.DMA_Mode = DMA_Mode_Circular;
    dma_init_structure.DMA_Priority = DMA_Priority_High;
    dma_init_structure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel2, &dma_init_structure);

    TIM_DMACmd(TIM1, TIM_DMA_CC1, ENABLE);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
    DMA_Cmd(DMA1_Channel2, ENABLE);

    WS2812_ClearAll();
    Delay_Ms(100);
}

void WS2812_ClearAll(void)
{
    uint16_t i;

    /* 把每颗灯的数据区清成黑色。 */
    for(i = 0; i < LED_NUM; ++i)
    {
        WS2812_Set(i, 0, 0, 0);
    }

    /* 末尾补一段低电平，满足 WS2812 的复位时序。 */
    for(i = 0; i < WS2812_RST_NUM; ++i)
    {
        pixel_buffer[LED_NUM * DATA_LEN + i] = 0;
    }

    WS2812_Show();
}

void WS2812_Show(void)
{
    /* DMA 一直在循环发送缓冲区，这里只需保证低电平复位时间足够即可。 */
    Delay_Us(80);
}

void WS2812_Set(uint16_t num, uint8_t red, uint8_t green, uint8_t blue)
{
    uint16_t index_base;
    uint8_t i;

    if(num >= LED_NUM)
    {
        return;
    }

    /* 每颗灯占 24 个时序单元，发送顺序是 GRB。 */
    index_base = num * DATA_LEN;

    for(i = 0; i < 8; ++i)
    {
        pixel_buffer[index_base + i] = ((green << i) & 0x80) ? WS_H : WS_L;
        pixel_buffer[index_base + i + 8] = ((red << i) & 0x80) ? WS_H : WS_L;
        pixel_buffer[index_base + i + 16] = ((blue << i) & 0x80) ? WS_H : WS_L;
    }
}

void WS2812_SetAll(uint8_t red, uint8_t green, uint8_t blue)
{
    uint16_t i;

    /* 批量把 8 颗灯设成同一个颜色。 */
    for(i = 0; i < LED_NUM; ++i)
    {
        WS2812_Set(i, red, green, blue);
    }
}

void ws281x_init(void)
{
    WS2812_Init();
}

void ws281x_closeAll(void)
{
    WS2812_ClearAll();
}

uint32_t ws281x_color(uint8_t red, uint8_t green, uint8_t blue)
{
    /* 打包成 GRB 顺序，便于兼容常见 ws281x 写法。 */
    return ((uint32_t)green << 16) | ((uint32_t)red << 8) | blue;
}

void ws281x_setPixelColor(uint16_t n, uint32_t grb_color)
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;

    green = (uint8_t)((grb_color >> 16) & 0xFF);
    red = (uint8_t)((grb_color >> 8) & 0xFF);
    blue = (uint8_t)(grb_color & 0xFF);

    WS2812_Set(n, red, green, blue);
}

void ws281x_setPixelRGB(uint16_t n, uint8_t red, uint8_t green, uint8_t blue)
{
    WS2812_Set(n, red, green, blue);
}

void ws281x_show(void)
{
    WS2812_Show();
}

void ws281x_colorWipe(uint32_t c, uint8_t wait)
{
    uint16_t i;

    /* 从前到后依次点亮。 */
    for(i = 0; i < PIXEL_NUM; ++i)
    {
        ws281x_setPixelColor(i, c);
        ws281x_show();
        Delay_Ms(wait);
    }
}

void ws281x_rainbow(uint8_t wait)
{
    uint16_t i;
    uint16_t j;

    /* 所有灯统一做彩虹渐变，只是相位不同。 */
    for(j = 0; j < 256; ++j)
    {
        for(i = 0; i < PIXEL_NUM; ++i)
        {
            ws281x_setPixelColor(i, ws281x_wheel((uint8_t)((i + j) & 255)));
        }

        ws281x_show();
        Delay_Ms(wait);
    }
}

void ws281x_rainbowCycle(uint8_t wait)
{
    uint16_t i;
    uint16_t j;

    /* 把一个完整彩虹分散铺到整条灯带上，再整体滚动。 */
    for(j = 0; j < 256 * 5; ++j)
    {
        for(i = 0; i < PIXEL_NUM; ++i)
        {
            ws281x_setPixelColor(i, ws281x_wheel((uint8_t)(((i * 256 / PIXEL_NUM) + j) & 255)));
        }

        ws281x_show();
        Delay_Ms(wait);
    }
}

void ws281x_theaterChase(uint32_t c, uint8_t wait)
{
    int j;
    int q;
    uint16_t i;

    /* 跑马灯效果：每次点亮间隔为 3 的灯。 */
    for(j = 0; j < 10; ++j)
    {
        for(q = 0; q < 3; ++q)
        {
            for(i = 0; i < PIXEL_NUM; i += 3)
            {
                if((i + q) < PIXEL_NUM)
                {
                    ws281x_setPixelColor((uint16_t)(i + q), c);
                }
            }

            ws281x_show();
            Delay_Ms(wait);

            for(i = 0; i < PIXEL_NUM; i += 3)
            {
                if((i + q) < PIXEL_NUM)
                {
                    ws281x_setPixelColor((uint16_t)(i + q), 0);
                }
            }
        }
    }
}

void ws281x_theaterChaseRainbow(uint8_t wait)
{
    int j;
    int q;
    uint16_t i;

    /* 彩虹版跑马灯。 */
    for(j = 0; j < 256; ++j)
    {
        for(q = 0; q < 3; ++q)
        {
            for(i = 0; i < PIXEL_NUM; i += 3)
            {
                if((i + q) < PIXEL_NUM)
                {
                    ws281x_setPixelColor((uint16_t)(i + q), ws281x_wheel((uint8_t)((i + j) % 255)));
                }
            }

            ws281x_show();
            Delay_Ms(wait);

            for(i = 0; i < PIXEL_NUM; i += 3)
            {
                if((i + q) < PIXEL_NUM)
                {
                    ws281x_setPixelColor((uint16_t)(i + q), 0);
                }
            }
        }
    }
}

static uint32_t ws281x_wheel(uint8_t wheel_pos)
{
    /* 把 0~255 映射成循环变化的 RGB 颜色。 */
    wheel_pos = 255U - wheel_pos;

    if(wheel_pos < 85U)
    {
        return ws281x_color((uint8_t)(255U - wheel_pos * 3U), 0, (uint8_t)(wheel_pos * 3U));
    }

    if(wheel_pos < 170U)
    {
        wheel_pos = (uint8_t)(wheel_pos - 85U);
        return ws281x_color(0, (uint8_t)(wheel_pos * 3U), (uint8_t)(255U - wheel_pos * 3U));
    }

    wheel_pos = (uint8_t)(wheel_pos - 170U);
    return ws281x_color((uint8_t)(wheel_pos * 3U), (uint8_t)(255U - wheel_pos * 3U), 0);
}