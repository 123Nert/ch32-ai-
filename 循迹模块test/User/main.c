#include "debug.h"
#include "../Hardware/inc/line_tracing.h"


/*
    黑色不亮，白色亮（测量有效距离2cm左右）
    PC1 接收循迹传感器 AO 脚信号，通过 ADC 采集并通过串口打印
*/
int main(void)
{
    uint16_t adc_value;
    uint16_t adc_avg;
    uint8_t line_status;
    uint16_t threshold = LINE_TRACING_DEFAULT_THRESHOLD;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);

    printf("\r\n");
    printf("========================================\r\n");
    printf("  Line Tracing Module Demo (CH32V307)\r\n");
    printf("  PC1 -> ADC1_CH11 -> Sensor Data\r\n");
    printf("========================================\r\n");

    /* 初始化循迹模块 */
    LineTracing_Init();
    printf("Threshold: %d (1.65V @ 12-bit ADC)\r\n", threshold);
    printf("========================================\r\n\r\n");
    printf("      Raw ADC    |    Avg(4x)    |    Status\r\n");
    printf("---------------------------------------------\r\n\r\n");

    while(1)
    {
        /* 单次采样 */
        adc_value = LineTracing_ReadADC();

        /* 4次采样平均 */
        adc_avg = LineTracing_ReadADC_Average(4);

        /* 基于平均值判断是否在黑线上 */
        line_status = (adc_avg < threshold) ? 1 : 0;

        /* 打印结果到串口 */
        printf("     %4d      |     %4d      |   %s\r\n",
               adc_value,
               adc_avg,
               line_status ? "WHITE" : "BLACK");

        Delay_Ms(500);
    }
}
