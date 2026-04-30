/********************     (C) COPYRIGHT 2017      **************************
 * 文件名  ：Sonic.c
 * 描述    ：超声波模块 测试例程       
 * 实验平台：STM32F103VET6
 * 库版本  ：ST3.5.0
 *
 * 编写日期：2017-04-10
 * 修改日期：2017-04-14
 * 作者    :
****************************************************************************/
#include "Sonic.h"
#include "delay.h"

/*******************************************************************************
*	Sonic  Init
*******************************************************************************/

u32  Distance = 0;
u8   Done;
u32 __IO time_1ms = 0;

void TIM2_Init(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  //NVIC_InitTypeDef 		   NVIC_InitStructure;
  /* TIM2 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF; 
	TIM_TimeBaseStructure.TIM_Prescaler = 142;          //144分频，500K的计数器
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
  TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);	
	TIM_Cmd(TIM2, DISABLE);
}

void Sonic_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA| RCC_APB2Periph_AFIO,ENABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;						//PA6 Trig
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_5;           		//PA5 Echo
	GPIO_Init(GPIOA,&GPIO_InitStructure);   
	
	GPIO_WriteBit(GPIOA,GPIO_Pin_6,(BitAction)0); 			//trig
	
	//EXTI_DeInit();
	EXTI_ClearITPendingBit(EXTI_Line5);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource5);
	EXTI_InitStructure.EXTI_Line= EXTI_Line5; 
	EXTI_InitStructure.EXTI_Mode= EXTI_Mode_Interrupt; 
	EXTI_InitStructure.EXTI_Trigger= EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd=ENABLE;
  EXTI_Init(&EXTI_InitStructure);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority= 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority= 0;        
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;   
	NVIC_Init(&NVIC_InitStructure);
	
	Distance = 0;
	Done = 1;
}

//void Sonic_Trig(void)
//{
//	u16 i = 0;
//	if((Done == 1)&&(time_1ms > 100))
//	{
//		time_1ms = 0;
//		GPIO_WriteBit(GPIOA,GPIO_Pin_6,(BitAction)1);
//		for(i=0;i<0xf0;i++);
//		GPIO_WriteBit(GPIOA,GPIO_Pin_6,(BitAction)0); 
//		Done = 0;
//	}
//}


void Wave_SRD_Strat(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_6);   //将Trig设置为高电平
	delay_us(20);               //持续大于10us触发，触发超声波模块工作
	GPIO_ResetBits(GPIOA,GPIO_Pin_6); 
	
}



void EXTI9_5_IRQHandler(void)
{
	static u8 flag_Sta = 0;
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)		
	{
		EXTI_ClearITPendingBit(EXTI_Line5);  
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_5)==1)
		{
			 TIM_SetCounter(TIM2,0);	
			 flag_Sta=1;
			 TIM_Cmd(TIM2, ENABLE);     
		}
		else
		{
			TIM_Cmd(TIM2, DISABLE); 					
			if(flag_Sta)
			{		
				Distance = TIM_GetCounter(TIM2);
				Distance = Distance /29;
				if(Distance > 300)
					Distance = 300;
				Done = 1;
			}
			flag_Sta=0;
		}
	}
}

/*******************      (C) COPYRIGHT 2017       *END OF FILE************/
