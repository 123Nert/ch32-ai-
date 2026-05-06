#include "debug.h"
#include "oled_demo.h"
#include "../Driver/OLED.h"
#include <string.h>

/*
 * 文件说明：
 * 1. 这里实现对外展示用的文本页、滚动页和图案动画。
 * 2. 所有效果都基于 Driver/OLED 提供的基础接口组合完成。
 */

#define OLED_LINE_CHARS 16u

/* 记录演示已运行的总时长，供状态页和动画使用。 */
static uint32_t g_oled_demo_uptime_ms = 0u;

/* 按 16 个字符宽度补空格，避免旧字符残留在行尾。 */
static void OLED_Demo_ShowPaddedLine(uint8_t line, const char *text)
{
    char buffer[OLED_LINE_CHARS + 1u];
    uint8_t i = 0;

    while((i < OLED_LINE_CHARS) && (text[i] != '\0'))
    {
        buffer[i] = text[i];
        i++;
    }

    while(i < OLED_LINE_CHARS)
    {
        buffer[i] = ' ';
        i++;
    }

    buffer[OLED_LINE_CHARS] = '\0';
    OLED_ShowString(line, 1u, buffer);
}

/* 用循环取模做简易跑马灯效果。 */
static void OLED_Demo_ShowScrollLine(uint8_t line, const char *text, uint8_t offset)
{
    char buffer[OLED_LINE_CHARS + 1u];
    uint16_t len = (uint16_t)strlen(text);
    uint8_t i;

    if(len == 0u)
    {
        OLED_Demo_ShowPaddedLine(line, "");
        return;
    }

    for(i = 0; i < OLED_LINE_CHARS; i++)
    {
        buffer[i] = text[(offset + i) % len];
    }

    buffer[OLED_LINE_CHARS] = '\0';
    OLED_ShowString(line, 1u, buffer);
}

/* 所有演示延时统一从这里走，保证运行时间统计一致。 */
static void OLED_Demo_DelayMs(uint16_t delay_ms)
{
    Delay_Ms(delay_ms);
    g_oled_demo_uptime_ms += delay_ms;
}

/* 斜向条纹动画，通过页号、列号和相位组合出变化图案。 */
static void OLED_Demo_DrawStripeFrame(uint8_t phase)
{
    uint8_t page;
    uint8_t x;
    uint8_t pattern;

    for(page = 0; page < 8u; page++)
    {
        OLED_SetCursor(page, 0u);
        for(x = 0; x < 128u; x++)
        {
            pattern = ((((x >> 3) + page + phase) & 1u) != 0u) ? 0xAAu : 0x55u;
            OLED_WriteData(pattern);
        }
    }
}

/* 波浪动画：每列只点亮一位，随着相位变化产生流动感。 */
static void OLED_Demo_DrawWaveFrame(uint8_t phase)
{
    uint8_t page;
    uint8_t x;
    uint8_t bit;

    for(page = 0; page < 8u; page++)
    {
        OLED_SetCursor(page, 0u);
        for(x = 0; x < 128u; x++)
        {
            bit = (uint8_t)(((x >> 2) + phase + page) & 0x07u);
            OLED_WriteData((uint8_t)(1u << bit));
        }
    }
}

/* 扫描条动画：模拟一个发光窗口沿着屏幕水平移动。 */
static void OLED_Demo_DrawScannerFrame(uint8_t offset)
{
    uint8_t page;
    uint8_t x;
    uint8_t head = (uint8_t)((offset * 4u) % 128u);
    uint8_t tail = (uint8_t)((head + 18u) % 128u);
    uint8_t inside;

    for(page = 0; page < 8u; page++)
    {
        OLED_SetCursor(page, 0u);
        for(x = 0; x < 128u; x++)
        {
            if(head <= tail)
            {
                inside = (uint8_t)((x >= head) && (x < tail));
            }
            else
            {
                inside = (uint8_t)((x >= head) || (x < tail));
            }

            if(inside != 0u)
            {
                OLED_WriteData((page & 1u) != 0u ? 0xFFu : 0x3Cu);
            }
            else
            {
                OLED_WriteData((x & 0x10u) != 0u ? 0x00u : 0x80u);
            }
        }
    }
}

/* 首页用于展示主控、屏幕地址和当前运行时间。 */
static void OLED_Demo_ShowDashboard(uint32_t uptime_s)
{
    char line[OLED_LINE_CHARS + 1u];

    OLED_Clear();
    OLED_Demo_ShowPaddedLine(1u, "CH32V307 SH1106");
    snprintf(line, sizeof(line), "ADDR 0x%02X", (unsigned int)OLED_GetAddress());
    OLED_Demo_ShowPaddedLine(2u, line);
    snprintf(line, sizeof(line), "RUN %lu S", (unsigned long)uptime_s);
    OLED_Demo_ShowPaddedLine(3u, line);
    OLED_Demo_ShowPaddedLine(4u, "DEMO READY");
}

/* 数字页集中展示本驱动已经支持的几种文本接口。 */
static void OLED_Demo_ShowNumberScreen(uint32_t uptime_s)
{
    OLED_Clear();
    OLED_Demo_ShowPaddedLine(1u, "NUM HEX BIN");
    OLED_ShowNum(2u, 1u, uptime_s % 10000u, 4u);
    OLED_ShowHexNum(3u, 1u, uptime_s, 8u);
    OLED_ShowBinNum(4u, 1u, uptime_s, 16u);
}

/* 滚动文字页展示 SH1106 在单色屏上的动态文本效果。 */
static void OLED_Demo_ShowMarqueeScreen(uint8_t offset, uint32_t uptime_s)
{
    char line[OLED_LINE_CHARS + 1u];
    static const char text1[] = " CH32V307 SH1106 DEMO ";
    static const char text2[] = " PB8 PB9 HARDWARE I2C ";

    OLED_Clear();
    OLED_Demo_ShowPaddedLine(1u, "MARQUEE MODE");
    OLED_Demo_ShowScrollLine(2u, text1, offset);
    OLED_Demo_ShowScrollLine(3u, text2, (uint8_t)(offset * 2u));
    snprintf(line, sizeof(line), "RUN %lu S", (unsigned long)uptime_s);
    OLED_Demo_ShowPaddedLine(4u, line);
}

/* 图案动画序列。 */
static void OLED_Demo_RunPatternDemo(void)
{
    uint8_t frame;

    printf("Demo: stripes\r\n");
    for(frame = 0; frame < 18u; frame++)
    {
        OLED_Demo_DrawStripeFrame(frame);
        OLED_Demo_DelayMs(90u);
    }

    printf("Demo: wave\r\n");
    for(frame = 0; frame < 24u; frame++)
    {
        OLED_Demo_DrawWaveFrame(frame);
        OLED_Demo_DelayMs(70u);
    }

    printf("Demo: scanner\r\n");
    for(frame = 0; frame < 32u; frame++)
    {
        OLED_Demo_DrawScannerFrame(frame);
        OLED_Demo_DelayMs(60u);
    }
}

/* 文本动画序列。 */
static void OLED_Demo_RunTextDemo(void)
{
    uint8_t step;
    uint8_t contrast;

    printf("Demo: dashboard\r\n");
    OLED_Demo_ShowDashboard(g_oled_demo_uptime_ms / 1000u);
    OLED_Demo_DelayMs(1200u);

    printf("Demo: numbers\r\n");
    OLED_Demo_ShowNumberScreen(g_oled_demo_uptime_ms / 1000u);
    OLED_Demo_DelayMs(1200u);

    printf("Demo: marquee\r\n");
    for(step = 0; step < 20u; step++)
    {
        OLED_Demo_ShowMarqueeScreen(step, g_oled_demo_uptime_ms / 1000u);
        OLED_Demo_DelayMs(140u);
    }

    printf("Demo: contrast\r\n");
    OLED_Clear();
    OLED_Demo_ShowPaddedLine(1u, "CONTRAST TEST");
    OLED_Demo_ShowPaddedLine(2u, "SH1106 EFFECT");
    OLED_Demo_ShowPaddedLine(3u, "LOOK BRIGHT");
    OLED_Demo_ShowPaddedLine(4u, "AND INVERSE");
    for(contrast = 0x20u; contrast <= 0xE0u; contrast += 0x20u)
    {
        OLED_SetContrast(contrast);
        OLED_Demo_DelayMs(120u);
    }
    for(contrast = 0xE0u; contrast >= 0x40u; contrast -= 0x20u)
    {
        OLED_SetContrast(contrast);
        OLED_Demo_DelayMs(120u);
        if(contrast == 0x40u)
        {
            break;
        }
    }

    printf("Demo: inverse\r\n");
    for(step = 0; step < 6u; step++)
    {
        OLED_SetInverse((uint8_t)(step & 1u));
        OLED_Demo_DelayMs(160u);
    }
    OLED_SetInverse(0u);
    OLED_SetContrast(0x80u);
}

/* 串口打印当前 OLED 状态，方便下载后快速确认。 */
void OLED_Demo_PrintStartup(void)
{
    printf("CH32V307 SH1106 I2C start\r\n");
    printf("OLED pins: I2C1 remap, SCL PB8, SDA PB9, VCC 3.3V, GND GND\r\n");
    printf("OLED init: %s, addr=0x%02X\r\n",
           OLED_IsReady() ? "OK" : "FAIL",
           (unsigned int)OLED_GetAddress());
}

/* 开机先做一次全屏点亮，再切回状态页。 */
void OLED_Demo_BootSplash(void)
{
    if(!OLED_IsReady())
    {
        return;
    }

    OLED_Fill(0xFFu);
    Delay_Ms(600u);
    OLED_Clear();
    OLED_Demo_ShowDashboard(0u);
}

/* 按顺序循环执行文本页和图案页。 */
void OLED_Demo_RunCycle(void)
{
    if(!OLED_IsReady())
    {
        printf("OLED not ready\r\n");
        Delay_Ms(1000u);
        g_oled_demo_uptime_ms += 1000u;
        return;
    }

    OLED_Demo_RunTextDemo();
    OLED_Demo_RunPatternDemo();
}
