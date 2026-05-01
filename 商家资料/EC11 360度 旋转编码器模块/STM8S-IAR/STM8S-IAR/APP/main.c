#include "stm8s.h"
#include "ILI9163.h"
#include "SGP18T_ILI9163B_base.h"
#include "SGP18T_ILI9163B_figure.h"
#include "SGP18T_ILI9163B_ascii.h"
#include "ILI9163CHN.h"
#include "show.h"
#include "Picture.h"
#include "encoder.h"

/**********************华丽的分割线*******************************
#define		RED		0x001f
#define		GREEN		0x07e0
#define		BLUE		0xf800
#define		PURPLE		0xf81f        //紫色
#define		YELLOW		0xffe0
#define		CYAN		0x07ff 		//蓝绿色
#define		ORANGE		0xfc08
#define		BLACK		0x0000
#define		WHITE		0xffff
*********************华丽的分割线********************************/

void CLK_Init(void)
{
 // CLK_DeInit();
CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);   //1分频 16MKZ
 //   CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV2);//2分频 8MHZ
 // CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV4);//4分频  4MHZ
 // CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV8);//8分频  2MHZ
}


void ILI9163_GPIO_init()
{
  GPIO_Init(GPIOD,GPIO_PIN_4,GPIO_MODE_OUT_OD_LOW_FAST);         //CS
  GPIO_Init(GPIOD,GPIO_PIN_3,GPIO_MODE_OUT_OD_LOW_FAST);         //RS
  GPIO_Init(GPIOD,GPIO_PIN_2,GPIO_MODE_OUT_OD_LOW_FAST);           //RW
  GPIO_Init(GPIOD,GPIO_PIN_0,GPIO_MODE_OUT_OD_LOW_FAST);       //RD
  
  GPIO_Init(GPIOC,GPIO_PIN_1,GPIO_MODE_OUT_OD_LOW_FAST);       //D1
  GPIO_Init(GPIOC,GPIO_PIN_2,GPIO_MODE_OUT_OD_LOW_FAST);       //D2
  GPIO_Init(GPIOC,GPIO_PIN_3,GPIO_MODE_OUT_OD_LOW_FAST);       //D3
  GPIO_Init(GPIOC,GPIO_PIN_4,GPIO_MODE_OUT_OD_LOW_FAST);       //D4
  GPIO_Init(GPIOC,GPIO_PIN_5,GPIO_MODE_OUT_OD_LOW_FAST);       //D5
  GPIO_Init(GPIOC,GPIO_PIN_6,GPIO_MODE_OUT_OD_LOW_FAST);       //D6
  GPIO_Init(GPIOC,GPIO_PIN_7,GPIO_MODE_OUT_OD_LOW_FAST);       //D7
  GPIO_Init(GPIOE,GPIO_PIN_5,GPIO_MODE_OUT_OD_LOW_FAST);       //D0
  
  GPIO_Init(GPIOD,GPIO_PIN_7,GPIO_MODE_OUT_PP_HIGH_FAST);           //背光

}

main()
{
 CLK_Init();
 ILI9163_GPIO_init();
 GPIO_WriteHigh(GPIOD,GPIO_PIN_7); 
 ILI9163B_init();
 delay(20);
 MY_EXTI_Config();
 ILI9163B_display_full(WHITE);
 __enable_interrupt();//开总中断
 
 while(1)
 {
   unsigned char bai,shi,ge;
   
   bai=count/100;
   shi=count/10%10;
   ge =count%10;
   
   LCD_printc16(3, 8, bai+'0' ,RED,WHITE);
   LCD_printc16(3, 9, shi+'0' ,RED,WHITE);
   LCD_printc16(3, 10,ge +'0' ,RED,WHITE);
 }
}