#include "stm32f10x.h"                  // Device header
#include "Delay.h" 
#include "usart.h" 
void HC140_Config(void)
{
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure = {0};//定义一个结构体变量 
			
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; //PA0
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IN_FLOATING; //浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化结构体
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
    GPIO_InitTypeDef fengmingqi;
    fengmingqi.GPIO_Mode=GPIO_Mode_Out_PP;
    fengmingqi.GPIO_Pin=GPIO_Pin_13;
    fengmingqi.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &fengmingqi);
	
	
}

void HC140_GET(void)
{
	if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)!=1)
	{
		printf("\xB4\xA5\xB7\xA2\r\n"); // 输出“触发”，保持原有 GBK 字节内容
		GPIO_SetBits(GPIOC,GPIO_Pin_14);
	}
}

