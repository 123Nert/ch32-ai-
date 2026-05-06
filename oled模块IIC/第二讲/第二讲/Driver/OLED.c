#include "OLED.h"
#include "OLED_Font.h"

/*
 * 文件说明：
 * 1. 这里实现 SH1106 模块的硬件 I2C 驱动。
 * 2. 使用 CH32V307 的 I2C1，并把引脚重映射到 PB8/PB9。
 * 3. 为了兼容常见模块，初始化时会依次尝试 0x3C 和 0x3D 两个地址。
 */

#ifndef OLED_I2C_PERIPH
#define OLED_I2C_PERIPH            I2C1
#define OLED_I2C_CLK               RCC_APB1Periph_I2C1
#define OLED_I2C_REMAP             GPIO_Remap_I2C1
#endif

#ifndef OLED_I2C_GPIO_PORT
#define OLED_I2C_GPIO_PORT         GPIOB
#define OLED_I2C_GPIO_CLK          RCC_APB2Periph_GPIOB
#define OLED_I2C_SCL_PIN           GPIO_Pin_8
#define OLED_I2C_SDA_PIN           GPIO_Pin_9
#endif

#ifndef OLED_I2C_CLOCK_SPEED
#define OLED_I2C_CLOCK_SPEED       100000u
#endif

#ifndef OLED_I2C_TIMEOUT
#define OLED_I2C_TIMEOUT           20000u
#endif

#ifndef OLED_I2C_ADDRESS
#define OLED_I2C_ADDRESS           0x78u
#endif

#ifndef OLED_I2C_ADDRESS_ALT
#define OLED_I2C_ADDRESS_ALT       0x7Au
#endif

#ifndef OLED_SH1106_COLUMN_OFFSET
#define OLED_SH1106_COLUMN_OFFSET  2u
#endif

static uint8_t OLED_ActiveAddress = 0u;

/* 等待 I2C 总线空闲。 */
static uint8_t OLED_I2C_WaitBusIdle(void)
{
    uint32_t timeout = OLED_I2C_TIMEOUT;

    while(I2C_GetFlagStatus(OLED_I2C_PERIPH, I2C_FLAG_BUSY) != RESET)
    {
        if(timeout-- == 0u)
        {
            return 0u;
        }
    }

    return 1u;
}

/* 等待指定 I2C 事件出现。 */
static uint8_t OLED_I2C_WaitEvent(uint32_t event)
{
    uint32_t timeout = OLED_I2C_TIMEOUT;

    while(I2C_CheckEvent(OLED_I2C_PERIPH, event) != READY)
    {
        if(I2C_GetFlagStatus(OLED_I2C_PERIPH, I2C_FLAG_AF) != RESET)
        {
            return 0u;
        }

        if(timeout-- == 0u)
        {
            return 0u;
        }
    }

    return 1u;
}

/* 结束当前传输，并清掉可能出现的应答失败标志。 */
static void OLED_I2C_StopAndClear(void)
{
    I2C_GenerateSTOP(OLED_I2C_PERIPH, ENABLE);

    if(I2C_GetFlagStatus(OLED_I2C_PERIPH, I2C_FLAG_AF) != RESET)
    {
        I2C_ClearFlag(OLED_I2C_PERIPH, I2C_FLAG_AF);
    }
}

/* 配置 PB8/PB9，并初始化 I2C1。 */
static void OLED_I2C_Init(void)
{
    GPIO_InitTypeDef gpio;
    I2C_InitTypeDef i2c;

    RCC_APB2PeriphClockCmd(OLED_I2C_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(OLED_I2C_CLK, ENABLE);

    GPIO_PinRemapConfig(OLED_I2C_REMAP, ENABLE);

    gpio.GPIO_Pin = OLED_I2C_SCL_PIN | OLED_I2C_SDA_PIN;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    gpio.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_Init(OLED_I2C_GPIO_PORT, &gpio);

    I2C_DeInit(OLED_I2C_PERIPH);
    I2C_StructInit(&i2c);
    i2c.I2C_ClockSpeed = OLED_I2C_CLOCK_SPEED;
    i2c.I2C_Mode = I2C_Mode_I2C;
    i2c.I2C_DutyCycle = I2C_DutyCycle_2;
    i2c.I2C_OwnAddress1 = 0x00u;
    i2c.I2C_Ack = I2C_Ack_Enable;
    i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(OLED_I2C_PERIPH, &i2c);
    I2C_Cmd(OLED_I2C_PERIPH, ENABLE);
}

/* 一个完整的 I2C 写事务：地址 + 控制字节 + 载荷字节。 */
static uint8_t OLED_I2C_WriteByte(uint8_t Address, uint8_t Control, uint8_t Data)
{
    if(!OLED_I2C_WaitBusIdle())
    {
        OLED_I2C_StopAndClear();
        return 0u;
    }

    I2C_GenerateSTART(OLED_I2C_PERIPH, ENABLE);
    if(!OLED_I2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
    {
        OLED_I2C_StopAndClear();
        return 0u;
    }

    I2C_Send7bitAddress(OLED_I2C_PERIPH, Address, I2C_Direction_Transmitter);
    if(!OLED_I2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        OLED_I2C_StopAndClear();
        return 0u;
    }

    I2C_SendData(OLED_I2C_PERIPH, Control);
    if(!OLED_I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        OLED_I2C_StopAndClear();
        return 0u;
    }

    I2C_SendData(OLED_I2C_PERIPH, Data);
    if(!OLED_I2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        OLED_I2C_StopAndClear();
        return 0u;
    }

    I2C_GenerateSTOP(OLED_I2C_PERIPH, ENABLE);
    return 1u;
}

/* 使用当前已经探测成功的 OLED 地址继续写数据。 */
static uint8_t OLED_WriteByte(uint8_t Control, uint8_t Byte)
{
    if(OLED_ActiveAddress == 0u)
    {
        return 0u;
    }

    return OLED_I2C_WriteByte(OLED_ActiveAddress, Control, Byte);
}

/* 发送 OLED 命令。 */
void OLED_WriteCommand(uint8_t Command)
{
    (void)OLED_WriteByte(0x00u, Command);
}

/* 发送 OLED 显存数据。 */
void OLED_WriteData(uint8_t Data)
{
    (void)OLED_WriteByte(0x40u, Data);
}

/* 设置当前写入位置。 */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
    /* SH1106 显存列地址通常比可视区域多出 2 列偏移。 */
    X += OLED_SH1106_COLUMN_OFFSET;
    OLED_WriteCommand(0xB0u | Y);
    OLED_WriteCommand(0x10u | ((X & 0xF0u) >> 4));
    OLED_WriteCommand(0x00u | (X & 0x0Fu));
}

/* 清空整屏。 */
void OLED_Clear(void)
{
    OLED_Fill(0x00u);
}

/* 设置 OLED 对比度。 */
void OLED_SetContrast(uint8_t Contrast)
{
    if(!OLED_IsReady())
    {
        return;
    }

    OLED_WriteCommand(0x81u);
    OLED_WriteCommand(Contrast);
}

/* 开启或关闭整屏反显。 */
void OLED_SetInverse(uint8_t Enable)
{
    if(!OLED_IsReady())
    {
        return;
    }

    OLED_WriteCommand(Enable ? 0xA7u : 0xA6u);
}

/* 用同一个字节填充整屏。 */
void OLED_Fill(uint8_t Data)
{
    uint8_t i;
    uint8_t j;

    if(!OLED_IsReady())
    {
        return;
    }

    for(j = 0; j < 8u; j++)
    {
        OLED_SetCursor(j, 0u);
        for(i = 0; i < 128u; i++)
        {
            OLED_WriteData(Data);
        }
    }
}

/* 在指定文本坐标显示单个 ASCII 字符。 */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
    uint8_t i;
    uint16_t font_index;

    if((Char < ' ') || (Char > '~'))
    {
        Char = ' ';
    }

    font_index = (uint16_t)(Char - ' ') * 16u;

    OLED_SetCursor((Line - 1u) * 2u, (Column - 1u) * 8u);
    for(i = 0; i < 8u; i++)
    {
        OLED_WriteData(OLED_F8x16[font_index + i]);
    }

    OLED_SetCursor((Line - 1u) * 2u + 1u, (Column - 1u) * 8u);
    for(i = 0; i < 8u; i++)
    {
        OLED_WriteData(OLED_F8x16[font_index + i + 8u]);
    }
}

/* 在指定位置连续显示字符串。 */
void OLED_ShowString(uint8_t Line, uint8_t Column, const char *String)
{
    uint8_t i;

    for(i = 0; String[i] != '\0'; i++)
    {
        OLED_ShowChar(Line, Column + i, String[i]);
    }
}

/* 计算整数次幂，供数字拆位显示使用。 */
static uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1u;

    while(Y--)
    {
        Result *= X;
    }

    return Result;
}

/* 以十进制显示无符号数。 */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;

    for(i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line,
                      Column + i,
                      (char)(Number / OLED_Pow(10u, Length - i - 1u) % 10u + '0'));
    }
}

/* 以带符号十进制显示整数。 */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
    uint8_t i;
    uint32_t Number1;

    if(Number >= 0)
    {
        OLED_ShowChar(Line, Column, '+');
        Number1 = (uint32_t)Number;
    }
    else
    {
        OLED_ShowChar(Line, Column, '-');
        Number1 = (uint32_t)(-Number);
    }

    for(i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line,
                      Column + i + 1u,
                      (char)(Number1 / OLED_Pow(10u, Length - i - 1u) % 10u + '0'));
    }
}

/* 以十六进制显示无符号数。 */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    uint8_t SingleNumber;

    for(i = 0; i < Length; i++)
    {
        SingleNumber = (uint8_t)(Number / OLED_Pow(16u, Length - i - 1u) % 16u);
        if(SingleNumber < 10u)
        {
            OLED_ShowChar(Line, Column + i, (char)(SingleNumber + '0'));
        }
        else
        {
            OLED_ShowChar(Line, Column + i, (char)(SingleNumber - 10u + 'A'));
        }
    }
}

/* 以二进制显示无符号数。 */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;

    for(i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line,
                      Column + i,
                      (char)(Number / OLED_Pow(2u, Length - i - 1u) % 2u + '0'));
    }
}

/* 初始化 I2C1、探测地址并发送 SH1106 配置序列。 */
void OLED_Init(void)
{
    OLED_ActiveAddress = 0u;

    OLED_I2C_Init();

    /* 先用关屏命令探测模块地址，探测成功后再发送完整初始化序列。 */
    if(OLED_I2C_WriteByte(OLED_I2C_ADDRESS, 0x00u, 0xAEu))
    {
        OLED_ActiveAddress = OLED_I2C_ADDRESS;
    }
    else if(OLED_I2C_WriteByte(OLED_I2C_ADDRESS_ALT, 0x00u, 0xAEu))
    {
        OLED_ActiveAddress = OLED_I2C_ADDRESS_ALT;
    }
    else
    {
        return;
    }

    /* SH1106 initialization sequence for 128x64 panels. */
    OLED_WriteCommand(0xAEu);
    OLED_WriteCommand(0xD5u);
    OLED_WriteCommand(0x80u);
    OLED_WriteCommand(0xA8u);
    OLED_WriteCommand(0x3Fu);
    OLED_WriteCommand(0xD3u);
    OLED_WriteCommand(0x00u);
    OLED_WriteCommand(0x40u);
    OLED_WriteCommand(0xADu);
    OLED_WriteCommand(0x8Bu);
    OLED_WriteCommand(0xA1u);
    OLED_WriteCommand(0xC8u);
    OLED_WriteCommand(0xDAu);
    OLED_WriteCommand(0x12u);
    OLED_WriteCommand(0x81u);
    OLED_WriteCommand(0x80u);
    OLED_WriteCommand(0xD9u);
    OLED_WriteCommand(0x1Fu);
    OLED_WriteCommand(0xDBu);
    OLED_WriteCommand(0x40u);
    OLED_WriteCommand(0xA4u);
    OLED_WriteCommand(0xA6u);
    OLED_WriteCommand(0xAFu);
    OLED_Clear();
}

/* 返回 OLED 是否已经正常应答。 */
uint8_t OLED_IsReady(void)
{
    return OLED_ActiveAddress != 0u;
}

/* 返回当前实际使用的 I2C 地址。 */
uint8_t OLED_GetAddress(void)
{
    return OLED_ActiveAddress;
}
