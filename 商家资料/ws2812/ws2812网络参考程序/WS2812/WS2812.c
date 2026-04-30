/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : WS2812.c
  * @brief          : WS2812驱动库
	* @author					：「千樊」
	* @time						: 2024/11/10
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/

	#include "WS2812.h"
	#include "BSP_RGB_UI_FONT.h"
	#include "ST7735.h"
	
	#include <stdarg.h>
	#include <stdio.h>
	#include <string.h>
		
/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */


	//显存数组，长度为 灯的数量*24+复位周期
	uint32_t WS2812_RGB_Buff[DataBuffLen] = {0}; 
	
/* USER CODE END PV */
/* Function definition -------------------------------------------------------*/
/* USER CODE BEGIN FD */
	
/**
  * @Function name  AXP_WriteData
  * @Introduce  		向指定寄存器写入数据		
  * @Return 				NULL
  */
	void WS2812_Set(uint16_t num,uint8_t R,uint8_t G,uint8_t B){
		uint32_t indexx=(num*(3*8));
		for (uint8_t i = 0;i < 8;i++){
			//填充数组
			WS2812_RGB_Buff[indexx+i]      = (G << i) & (0x80)?WS_H:WS_L;
			WS2812_RGB_Buff[indexx+i + 8]  = (R << i) & (0x80)?WS_H:WS_L;
			WS2812_RGB_Buff[indexx+i + 16] = (B << i) & (0x80)?WS_H:WS_L;
		}
	}
	
	void QF_RGB_SetPos(uint16_t x,uint16_t y,uint16_t color){          
		
		uint8_t R_Ch = (color >> 11) & 0x1F;
		R_Ch = R_Ch << 3; // 直接左移3位，低位用0填充
		uint8_t G_Ch = (color >> 5) & 0x3F;
		G_Ch = G_Ch << 2; // 直接左移2位，低位用0填充
		uint8_t B_Ch = color & 0x1F;
		B_Ch = B_Ch << 3; // 直接左移3位，低位用0填充
		
		WS2812_Set(y*8+x,R_Ch,G_Ch,B_Ch);
	}
	
	
	//WS2812初始化函数
	void WS2812_Init(){
		//设置关闭所有灯
		for(int i=0;i<LED_NUM;i++){
			WS2812_Set(i,0,0,0);
		}
		//作用：调用DMA将显存中的内容实时搬运至定时器的比较寄存器
		HAL_TIM_PWM_Start_DMA(&htim2,TIM_CHANNEL_1,(uint32_t *)WS2812_RGB_Buff,DataBuffLen);
	}

	
	void RGB_ShowChar_8_8(uint8_t chr,uint16_t BackGroundColor,uint16_t Color){
		unsigned char c = chr - ' ';
		
		for(uint8_t i=0;i<8;i++){
			for(uint8_t j=0;j<8;j++){
					if(((Song8X8[c*8+i]>>j) & 0x01 ) == 0){
						QF_RGB_SetPos(j,i,BackGroundColor);
					}else{
						QF_RGB_SetPos(j,i,Color);
					}
			}
		}
	}
	
	

	void QF_RGB8_printf(uint16_t BackGroundColor,uint16_t Color,const char* format, ...){
		char buffer[50];
    va_list args;
    va_start(args, format);
 
    // 使用 vsnprintf 格式化字符串到缓冲区
    vsnprintf(buffer, 50, format, args);	
		
		LCD_16_printf(0,32,0x0000,0x00ff,"Strlen = %d",strlen(buffer));
		
		for(uint8_t m = 0;m<strlen(buffer)-1;m++){
			
			//完成两个字符过渡
			for(uint8_t k=0;k<8;k++){
				for(uint8_t i=0;i<8;i++){
					uint8_t HorBuff = (((Song8X8[(buffer[m] - ' ')*8+i])<<k)) | (((Song8X8[(buffer[m+1] - ' ')*8+i])>>(8-k)));
					for(uint8_t j=0;j<8;j++){
						if(((HorBuff>>j) & 0x01) == 0){
							QF_RGB_SetPos(7-j,i,BackGroundColor);
						}else{
							QF_RGB_SetPos(7-j,i,Color);
						}
					}
				}
				HAL_Delay(100);
			}
		}
	}
	
	
	
/* USER CODE END FD */
/************************ (C) COPYRIGHT 「千樊」 *****END OF FILE****/
