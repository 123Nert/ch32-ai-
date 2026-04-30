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

void ws281x_breathe(uint16_t num, uint32_t c, uint8_t speed)
{
    uint16_t brightness;

    /* 亮度从 0 递增到 255。 */
    for(brightness = 0; brightness <= 255; brightness++)
    {
        uint8_t red = ((c >> 8) & 0xFF) * brightness / 255;
        uint8_t green = ((c >> 16) & 0xFF) * brightness / 255;
        uint8_t blue = (c & 0xFF) * brightness / 255;

        ws281x_setPixelRGB(num, red, green, blue);
        ws281x_show();
        Delay_Ms(speed);
    }

    /* 亮度从 255 递减到 0。 */
    for(brightness = 255; brightness > 0; brightness--)
    {
        uint8_t red = ((c >> 8) & 0xFF) * brightness / 255;
        uint8_t green = ((c >> 16) & 0xFF) * brightness / 255;
        uint8_t blue = (c & 0xFF) * brightness / 255;

        ws281x_setPixelRGB(num, red, green, blue);
        ws281x_show();
        Delay_Ms(speed);
    }

    ws281x_setPixelRGB(num, 0, 0, 0);
    ws281x_show();
}

void ws281x_breatheAll(uint32_t c, uint8_t speed)
{
    uint16_t brightness;
    uint16_t i;

    /* 亮度从 0 递增到 255。 */
    for(brightness = 0; brightness <= 255; brightness++)
    {
        uint8_t red = ((c >> 8) & 0xFF) * brightness / 255;
        uint8_t green = ((c >> 16) & 0xFF) * brightness / 255;
        uint8_t blue = (c & 0xFF) * brightness / 255;

        for(i = 0; i < PIXEL_NUM; ++i)
        {
            ws281x_setPixelRGB(i, red, green, blue);
        }

        ws281x_show();
        Delay_Ms(speed);
    }

    /* 亮度从 255 递减到 0。 */
    for(brightness = 255; brightness > 0; brightness--)
    {
        uint8_t red = ((c >> 8) & 0xFF) * brightness / 255;
        uint8_t green = ((c >> 16) & 0xFF) * brightness / 255;
        uint8_t blue = (c & 0xFF) * brightness / 255;

        for(i = 0; i < PIXEL_NUM; ++i)
        {
            ws281x_setPixelRGB(i, red, green, blue);
        }

        ws281x_show();
        Delay_Ms(speed);
    }

    for(i = 0; i < PIXEL_NUM; ++i)
    {
        ws281x_setPixelRGB(i, 0, 0, 0);
    }
    ws281x_show();
}

void ws281x_blink(uint16_t num, uint32_t c, uint8_t times, uint8_t speed)
{
    uint8_t i;

    for(i = 0; i < times; ++i)
    {
        ws281x_setPixelColor(num, c);
        ws281x_show();
        Delay_Ms(speed);

        ws281x_setPixelColor(num, 0);
        ws281x_show();
        Delay_Ms(speed);
    }
}

void ws281x_blinkAll(uint32_t c, uint8_t times, uint8_t speed)
{
    uint8_t i;
    uint16_t j;

    for(i = 0; i < times; ++i)
    {
        for(j = 0; j < PIXEL_NUM; ++j)
        {
            ws281x_setPixelColor(j, c);
        }
        ws281x_show();
        Delay_Ms(speed);

        for(j = 0; j < PIXEL_NUM; ++j)
        {
            ws281x_setPixelColor(j, 0);
        }
        ws281x_show();
        Delay_Ms(speed);
    }
}

void ws281x_colorScan(uint8_t wait)
{
    uint16_t i;
    uint32_t colors[3] = {
        ws281x_color(255, 0, 0),      /* 红 */
        ws281x_color(0, 255, 0),      /* 绿 */
        ws281x_color(0, 0, 255)       /* 蓝 */
    };

    /* 每次取一个颜色，从前到后扫一遍。 */
    for(int c = 0; c < 3; ++c)
    {
        for(i = 0; i < PIXEL_NUM; ++i)
        {
            WS2812_ClearAll();
            ws281x_setPixelColor(i, colors[c]);
            ws281x_show();
            Delay_Ms(wait);
        }
    }
}

void ws281x_randomColor(uint8_t iterations, uint8_t wait)
{
    uint16_t iter;
    uint16_t i;

    for(iter = 0; iter < iterations; ++iter)
    {
        for(i = 0; i < PIXEL_NUM; ++i)
        {
            uint8_t random_val = (uint8_t)(i * 31 + iter * 17) & 0xFF;
            ws281x_setPixelColor(i, ws281x_wheel(random_val));
        }

        ws281x_show();
        Delay_Ms(wait);
    }

    WS2812_ClearAll();
}

void ws281x_twinkle(uint8_t iterations, uint8_t wait)
{
    uint16_t iter;
    uint16_t i;

    for(iter = 0; iter < iterations; ++iter)
    {
        WS2812_ClearAll();

        for(i = 0; i < PIXEL_NUM; ++i)
        {
            /* 简单的伪随机：奇偶位交替亮起。 */
            if((i + iter) & 1)
            {
                uint8_t wheel_val = (uint8_t)(i * 32 + iter * 13) & 0xFF;
                ws281x_setPixelColor(i, ws281x_wheel(wheel_val));
            }
        }

        ws281x_show();
        Delay_Ms(wait);
    }

    WS2812_ClearAll();
}

void ws281x_meteor(uint32_t c, uint8_t wait, uint8_t decay)
{
    uint16_t pos;
    uint16_t i;

    /* 流星扫过，留下渐暗的尾巴。 */
    for(pos = 0; pos < PIXEL_NUM * 2; ++pos)
    {
        WS2812_ClearAll();

        for(i = 0; i < PIXEL_NUM; ++i)
        {
            if((int)pos - (int)i < PIXEL_NUM && pos - i >= 0)
            {
                /* 距离流星头的距离决定亮度。 */
                uint16_t fade = (pos - i) * decay / PIXEL_NUM;
                if(fade < 256)
                {
                    uint8_t red = ((c >> 8) & 0xFF) * (255 - fade) / 255;
                    uint8_t green = ((c >> 16) & 0xFF) * (255 - fade) / 255;
                    uint8_t blue = (c & 0xFF) * (255 - fade) / 255;

                    ws281x_setPixelRGB(i, red, green, blue);
                }
            }
        }

        ws281x_show();
        Delay_Ms(wait);
    }

    WS2812_ClearAll();
}

void ws281x_meteorRainbow(uint8_t wait, uint8_t decay)
{
    uint16_t pos;
    uint16_t i;

    for(pos = 0; pos < PIXEL_NUM * 2; ++pos)
    {
        WS2812_ClearAll();

        for(i = 0; i < PIXEL_NUM; ++i)
        {
            if((int)pos - (int)i < PIXEL_NUM && pos - i >= 0)
            {
                uint16_t fade = (pos - i) * decay / PIXEL_NUM;
                if(fade < 256)
                {
                    uint32_t color = ws281x_wheel((uint8_t)(i * 32));
                    uint8_t red = ((color >> 8) & 0xFF) * (255 - fade) / 255;
                    uint8_t green = ((color >> 16) & 0xFF) * (255 - fade) / 255;
                    uint8_t blue = (color & 0xFF) * (255 - fade) / 255;

                    ws281x_setPixelRGB(i, red, green, blue);
                }
            }
        }

        ws281x_show();
        Delay_Ms(wait);
    }

    WS2812_ClearAll();
}

void ws281x_heartbeat(uint32_t c, uint8_t times)
{
    uint8_t i;

    for(i = 0; i < times; ++i)
    {
        /* 第一次闪烁 */
        ws281x_blinkAll(c, 1, 100);
        Delay_Ms(100);

        /* 第二次闪烁 */
        ws281x_blinkAll(c, 1, 100);
        Delay_Ms(300);
    }

    WS2812_ClearAll();
}

void ws281x_fire(uint8_t cooling, uint8_t sparking, uint8_t speed)
{
    uint16_t i;
    uint16_t iter;
    uint8_t heat[PIXEL_NUM] = {0};

    for(iter = 0; iter < 50; ++iter)
    {
        /* 冷却效果 */
        for(i = 0; i < PIXEL_NUM; ++i)
        {
            uint8_t cooldown = (cooling * 255) / 32;
            if(heat[i] < cooldown)
            {
                heat[i] = 0;
            }
            else
            {
                heat[i] = heat[i] - cooldown;
            }
        }

        /* 火焰传播 */
        for(i = PIXEL_NUM - 1; i > 0; --i)
        {
            heat[i] = (heat[i] + heat[i - 1]) / 2;
        }

        /* 随机点火 */
        if((iter * 7 + i * 13) % 100 < sparking)
        {
            heat[0] = 255;
        }

        /* 显示火焰颜色 */
        for(i = 0; i < PIXEL_NUM; ++i)
        {
            uint8_t h = heat[i];
            uint32_t color;

            if(h < 85)
            {
                color = ws281x_color(h * 3, 0, 0);
            }
            else if(h < 170)
            {
                color = ws281x_color(255, (h - 85) * 3, 0);
            }
            else
            {
                color = ws281x_color(255, 255, (h - 170) * 3);
            }

            ws281x_setPixelColor(i, color);
        }

        ws281x_show();
        Delay_Ms(speed);
    }

    WS2812_ClearAll();
}

void ws281x_chase(uint32_t c1, uint32_t c2, uint8_t speed)
{
    uint16_t pos1;
    uint16_t pos2;
    uint16_t i;

    /* 两个光点相互追逐 */
    for(pos1 = 0; pos1 < PIXEL_NUM * 2; ++pos1)
    {
        pos2 = (pos1 + PIXEL_NUM / 2) % (PIXEL_NUM * 2);

        WS2812_ClearAll();

        if(pos1 < PIXEL_NUM)
        {
            ws281x_setPixelColor(pos1, c1);
        }

        if(pos2 < PIXEL_NUM)
        {
            ws281x_setPixelColor(pos2, c2);
        }

        ws281x_show();
        Delay_Ms(speed);
    }

    WS2812_ClearAll();
}

void ws281x_wave(uint32_t c, uint8_t speed)
{
    uint16_t phase;
    uint16_t i;

    for(phase = 0; phase < 256; ++phase)
    {
        for(i = 0; i < PIXEL_NUM; ++i)
        {
            /* 使用正弦波产生亮度变化 */
            uint16_t brightness = (uint16_t)(128 + 127 *
                ((phase + i * 256 / PIXEL_NUM) & 0xFF)) / 256;
            if(brightness > 255) brightness = 255;

            uint8_t red = ((c >> 8) & 0xFF) * brightness / 256;
            uint8_t green = ((c >> 16) & 0xFF) * brightness / 256;
            uint8_t blue = (c & 0xFF) * brightness / 256;

            ws281x_setPixelRGB(i, red, green, blue);
        }

        ws281x_show();
        Delay_Ms(speed);
    }

    WS2812_ClearAll();
}

void ws281x_police(uint8_t times, uint8_t speed)
{
    uint8_t i;
    uint32_t blue = ws281x_color(0, 0, 255);
    uint32_t red = ws281x_color(255, 0, 0);

    for(i = 0; i < times; ++i)
    {
        /* 蓝色闪烁 */
        for(uint16_t j = 0; j < PIXEL_NUM; ++j)
        {
            ws281x_setPixelColor(j, blue);
        }
        ws281x_show();
        Delay_Ms(speed);

        WS2812_ClearAll();
        Delay_Ms(speed);

        /* 红色闪烁 */
        for(uint16_t j = 0; j < PIXEL_NUM; ++j)
        {
            ws281x_setPixelColor(j, red);
        }
        ws281x_show();
        Delay_Ms(speed);

        WS2812_ClearAll();
        Delay_Ms(speed);
    }
}

void ws281x_rainbow_bands(uint8_t wait)
{
    uint16_t i;
    uint16_t j;

    for(j = 0; j < 256; ++j)
    {
        for(i = 0; i < PIXEL_NUM; ++i)
        {
            /* 每颗灯显示不同的彩虹颜色段 */
            uint8_t wheel_val = (uint8_t)(j + i * 256 / PIXEL_NUM) & 0xFF;
            ws281x_setPixelColor(i, ws281x_wheel(wheel_val));
        }

        ws281x_show();
        Delay_Ms(wait);
    }

    WS2812_ClearAll();
}

void ws281x_fade_scan(uint32_t c, uint8_t speed)
{
    uint16_t pos;
    uint16_t i;

    for(pos = 0; pos < PIXEL_NUM; ++pos)
    {
        for(i = 0; i < PIXEL_NUM; ++i)
        {
            if(i == pos)
            {
                ws281x_setPixelColor(i, c);
            }
            else if(i == (pos + PIXEL_NUM - 1) % PIXEL_NUM)
            {
                /* 前一个灯开始淡暗 */
                uint8_t red = ((c >> 8) & 0xFF) / 2;
                uint8_t green = ((c >> 16) & 0xFF) / 2;
                uint8_t blue = (c & 0xFF) / 2;
                ws281x_setPixelRGB(i, red, green, blue);
            }
            else if(i == (pos + PIXEL_NUM - 2) % PIXEL_NUM)
            {
                /* 更前一个灯继续淡暗 */
                uint8_t red = ((c >> 8) & 0xFF) / 4;
                uint8_t green = ((c >> 16) & 0xFF) / 4;
                uint8_t blue = (c & 0xFF) / 4;
                ws281x_setPixelRGB(i, red, green, blue);
            }
            else
            {
                ws281x_setPixelColor(i, 0);
            }
        }

        ws281x_show();
        Delay_Ms(speed);
    }

    WS2812_ClearAll();
}

void ws281x_multicolor_running(uint8_t wait)
{
    uint16_t pos;
    uint16_t i;
    uint32_t colors[4] = {
        ws281x_color(255, 0, 0),      /* 红 */
        ws281x_color(0, 255, 0),      /* 绿 */
        ws281x_color(0, 0, 255),      /* 蓝 */
        ws281x_color(255, 255, 0)     /* 黄 */
    };

    for(pos = 0; pos < PIXEL_NUM; ++pos)
    {
        for(i = 0; i < PIXEL_NUM; ++i)
        {
            uint8_t color_idx = (i + pos) % 4;
            ws281x_setPixelColor(i, colors[color_idx]);
        }

        ws281x_show();
        Delay_Ms(wait);
    }

    WS2812_ClearAll();
}

void ws281x_pulse(uint8_t iterations, uint8_t speed)
{
    uint8_t iter;
    uint32_t colors[3] = {
        ws281x_color(255, 0, 0),      /* 红 */
        ws281x_color(0, 255, 0),      /* 绿 */
        ws281x_color(0, 0, 255)       /* 蓝 */
    };

    for(iter = 0; iter < iterations; ++iter)
    {
        uint8_t color_idx = iter % 3;

        /* 快速脉冲 */
        for(uint16_t i = 0; i < 2; ++i)
        {
            for(uint16_t j = 0; j < PIXEL_NUM; ++j)
            {
                ws281x_setPixelColor(j, colors[color_idx]);
            }
            ws281x_show();
            Delay_Ms(speed / 2);

            WS2812_ClearAll();
            Delay_Ms(speed / 2);
        }

        /* 停顿 */
        Delay_Ms(speed);
    }
}