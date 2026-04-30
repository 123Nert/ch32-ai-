#include "debug.h"
#include "ws2812.h"
#include "string.h"
#include "stdlib.h"

#define CMD_BUFFER_SIZE 64
static char cmd_buffer[CMD_BUFFER_SIZE];
static uint16_t cmd_index = 0;

void parse_command(const char *cmd)
{
    int index, r, g, b;

    if (strncmp(cmd, "SET,", 4) == 0) {
        if (sscanf(cmd, "SET,%d,%d,%d,%d", &index, &r, &g, &b) == 4) {
            if (index >= 0 && index < LED_NUM) {
                WS2812_Set(index, r, g, b);
                WS2812_Show();
                printf("OK\r\n");
            }
        }
    }
    else if (strncmp(cmd, "FILL,", 5) == 0) {
        if (sscanf(cmd, "FILL,%d,%d,%d", &r, &g, &b) == 3) {
            WS2812_SetAll(r, g, b);
            WS2812_Show();
            printf("OK\r\n");
        }
    }
    else if (strcmp(cmd, "CLEAR") == 0) {
        WS2812_ClearAll();
        WS2812_Show();
        printf("OK\r\n");
    }
}

void USART3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USART3_IRQHandler(void)
{
    if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET) {
        uint8_t ch = USART_ReceiveData(USART3);

        if (ch == '\r' || ch == '\n') {
            if (cmd_index > 0) {
                cmd_buffer[cmd_index] = '\0';
                parse_command(cmd_buffer);
                cmd_index = 0;
            }
        }
        else if (cmd_index < CMD_BUFFER_SIZE - 1 && ch >= 32 && ch < 127) {
            cmd_buffer[cmd_index++] = ch;
        }

        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}

void USART3_Init(uint32_t baudrate)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef  NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baudrate;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART3, &USART_InitStructure);
    USART_Cmd(USART3, ENABLE);

    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);

    printf("\r\n\r\n");
    printf("=====================================\r\n");
    printf("WS2812 LED Control System v1.0\r\n");
    printf("=====================================\r\n");
    printf("\r\n");

    printf("[INIT] Initializing WS2812 LED...\r\n");
    WS2812_Init();
    Delay_Ms(100);

    printf("[TEST] Clearing all LEDs...\r\n");
    WS2812_ClearAll();
    WS2812_Show();
    Delay_Ms(200);

    printf("[TEST] Testing LED 0 (Red)...\r\n");
    WS2812_Set(0, 255, 0, 0);
    WS2812_Show();
    Delay_Ms(500);

    printf("[TEST] Testing LED 1 (Green)...\r\n");
    WS2812_Set(1, 0, 255, 0);
    WS2812_Show();
    Delay_Ms(500);

    printf("[TEST] Testing LED 2 (Blue)...\r\n");
    WS2812_Set(2, 0, 0, 255);
    WS2812_Show();
    Delay_Ms(500);

    printf("[TEST] Clearing all LEDs again...\r\n");
    WS2812_ClearAll();
    WS2812_Show();
    Delay_Ms(200);

    printf("\r\n");
    printf("[INIT] Initializing UART3 (PB10-TX, PB11-RX)...\r\n");
    USART3_Init(115200);
    Delay_Ms(100);

    printf("\r\n");
    printf("=====================================\r\n");
    printf("System Ready!\r\n");
    printf("Commands:\r\n");
    printf("  SET,0-7,R,G,B - Set LED color\r\n");
    printf("  FILL,R,G,B    - Fill all LEDs\r\n");
    printf("  CLEAR          - Clear all LEDs\r\n");
    printf("=====================================\r\n");
    printf("\r\n");

    while(1) {
        Delay_Ms(100);
    }

    return 0;
}
