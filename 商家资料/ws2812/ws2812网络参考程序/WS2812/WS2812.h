/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : WS2812.h
  * @brief          : Header for WS2812.c file.
  *                   This file provides code for the configuration
  *                   of the WS2812 instances
	* @author         : 「千樊」
	*	@time						:	2024/11/10
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WS2812_H_
#define __WS2812_H_

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

	#include "main.h"
	#include "tim.h"
	
/* USER CODE END Includes */
/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PT */

	#define WS_H           95   // 1 码相对计数值
	#define WS_L           30   // 0 码相对计数值
	#define LED_NUM        64   // WS2812灯个数
	#define DATA_LEN       24   // WS2812数据长度，单个需要24个字节
	#define WS2812_RST_NUM 60   // 官方复位时间为50us（40个周期），保险起见使用50个周期
	#define DataBuffLen		 1596	//缓冲区数组长度 = LED_NUM * DATA_LEN + WS2812_RST_NUM

/* USER CODE END PT */
/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */

	void WS2812_Init(void);
	void WS2812_Set(uint16_t num,uint8_t R,uint8_t G,uint8_t B);
	void QF_RGB_SetPos(uint16_t x,uint16_t y,uint16_t color);
	
	void RGB_ShowChar_8_8(uint8_t chr,uint16_t BackGroundColor,uint16_t Color);
	void QF_RGB8_printf(uint16_t BackGroundColor,uint16_t Color,const char* format, ...);

/* USER CODE END EFP */
#endif /* __WS2812_H_ */
/************************ (C) COPYRIGHT Lesterbor *****END OF FILE*************/
