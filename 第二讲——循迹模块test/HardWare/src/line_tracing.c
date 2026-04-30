#include "line_tracing.h"

/* 初始化循迹模块 */
void LineTracing_Init(void)
{
    ADC_InitTypeDef ADC_InitStructure = {0};
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    /* 启用时钟 */
    RCC_APB2PeriphClockCmd(LINE_TRACING_RCC_GPIO | LINE_TRACING_RCC_ADC, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    /* GPIO 配置为模拟输入 */
    GPIO_InitStructure.GPIO_Pin = LINE_TRACING_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(LINE_TRACING_GPIO_PORT, &GPIO_InitStructure);

    /* ADC 结构体初始化 */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(LINE_TRACING_ADC, &ADC_InitStructure);

    /* 配置 ADC 通道和采样时间 */
    ADC_RegularChannelConfig(LINE_TRACING_ADC, LINE_TRACING_ADC_CHANNEL,
                             1, ADC_SampleTime_239Cycles5);

    /* 启用 ADC */
    ADC_Cmd(LINE_TRACING_ADC, ENABLE);

    /* ADC 校准 */
    ADC_ResetCalibration(LINE_TRACING_ADC);
    while(ADC_GetResetCalibrationStatus(LINE_TRACING_ADC));
    ADC_StartCalibration(LINE_TRACING_ADC);
    while(ADC_GetCalibrationStatus(LINE_TRACING_ADC));

    printf("Line Tracing Module Initialized (PC1 - ADC1_CH11)\r\n");
}

/* 读取原始 ADC 值 */
uint16_t LineTracing_ReadADC(void)
{
    /* 启动 ADC 转换 */
    ADC_SoftwareStartConvCmd(LINE_TRACING_ADC, ENABLE);

    /* 等待转换完成 */
    while(!ADC_GetFlagStatus(LINE_TRACING_ADC, ADC_FLAG_EOC));

    /* 返回转换结果（12位，0-4095） */
    return ADC_GetConversionValue(LINE_TRACING_ADC);
}

/* 判断是否在黑线上 */
uint8_t LineTracing_IsOnLine(uint16_t threshold)
{
    uint16_t adc_val = LineTracing_ReadADC();

    /* ADC 值低于阈值时在黑线上 */
    if(adc_val < threshold)
        return 1;
    else
        return 0;
}

/* 多次采样取平均值（降低噪声） */
uint16_t LineTracing_ReadADC_Average(uint8_t samples)
{
    uint32_t sum = 0;
    uint8_t i;

    for(i = 0; i < samples; i++)
    {
        sum += LineTracing_ReadADC();
    }

    return (uint16_t)(sum / samples);
}
