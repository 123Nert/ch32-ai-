#ifndef __OLED_H
#define __OLED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ch32v30x.h"

/*
 * 文件说明：
 * 1. 本驱动面向 128x64 的 SH1106 单色 OLED 模块。
 * 2. 通信方式为 4 针脚硬件 I2C，默认使用 I2C1 重映射到 PB8/PB9。
 * 3. 字符显示按 8x16 ASCII 字模组织，逻辑上提供 4 行 x 16 列文本区域。
 */

/*
 * SH1106 4-pin I2C OLED default wiring on CH32V307:
 * SCL -> PB8
 * SDA -> PB9
 * VCC/GND -> 3.3 V/GND
 *
 * The driver uses hardware I2C1 remapped to PB8/PB9.
 */

/* 基础初始化与状态查询接口。 */
void OLED_Init(void);
uint8_t OLED_IsReady(void);
uint8_t OLED_GetAddress(void);

/* 底层命令/数据接口，方便上层做图案动画。 */
void OLED_WriteCommand(uint8_t Command);
void OLED_WriteData(uint8_t Data);
void OLED_SetCursor(uint8_t Y, uint8_t X);

/* 全屏控制接口。 */
void OLED_Clear(void);
void OLED_Fill(uint8_t Data);
void OLED_SetContrast(uint8_t Contrast);
void OLED_SetInverse(uint8_t Enable);

/* 文本显示接口。 */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, const char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

#ifdef __cplusplus
}
#endif

#endif
