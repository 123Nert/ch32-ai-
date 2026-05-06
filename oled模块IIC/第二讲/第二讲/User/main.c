#include "debug.h"
#include "oled_demo.h"
#include "../Driver/OLED.h"

/*
 * 文件说明：
 * 1. main.c 只负责板级初始化和主循环调度。
 * 2. 具体的 OLED 演示逻辑已经拆分到 oled_demo 模块。
 */

/* 集中完成板级初始化，方便后续扩展其他外设。 */
static void Board_Init(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);
}

/* 主流程：初始化 OLED 后持续运行演示模块。 */
int main(void)
{
    Board_Init();

    OLED_Init();
    OLED_Demo_PrintStartup();
    OLED_Demo_BootSplash();

    while(1)
    {
        OLED_Demo_RunCycle();
    }
}
