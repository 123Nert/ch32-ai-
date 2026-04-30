#ifndef __LINE_TRACING_H__
#define __LINE_TRACING_H__

#include "debug.h"

/* 循迹传感器配置 */
#define LINE_TRACING_ADC            ADC1
#define LINE_TRACING_GPIO_PORT      GPIOC
#define LINE_TRACING_GPIO_PIN       GPIO_Pin_1
#define LINE_TRACING_ADC_CHANNEL    ADC_Channel_11
#define LINE_TRACING_RCC_GPIO       RCC_APB2Periph_GPIOC
#define LINE_TRACING_RCC_ADC        RCC_APB2Periph_ADC1

/* 默认阈值（12位ADC值：0-4095） */
#define LINE_TRACING_DEFAULT_THRESHOLD    2048

/* 初始化循迹模块 */
void LineTracing_Init(void);

/* 读取原始 ADC 值 */
uint16_t LineTracing_ReadADC(void);

/* 判断是否在黑线上（基于阈值） */
uint8_t LineTracing_IsOnLine(uint16_t threshold);

/* 带平均的 ADC 读取（更稳定） */
uint16_t LineTracing_ReadADC_Average(uint8_t samples);

#endif /* __LINE_TRACING_H__ */
