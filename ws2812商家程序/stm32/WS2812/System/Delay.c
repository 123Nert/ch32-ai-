#include "delay.h"
uint16_t ledtime=0;
extern uint8_t reciceover;
void delay_1us(void)
{
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
  __nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();
}

void delay_us(uint16_t t)
{
	for(uint16_t i=0;i<t;i++)
	{
	 delay_1us();
	}
}

void delay_ms(uint32_t t)
{
	t=t*1000;
	for(uint32_t i=0;i<t;i++)
	{
	 delay_1us();
	}
}

/**
  * @brief  微秒级延时（72MHz主频）
  */
void Delay_us(uint32_t us) {
    us *= 72 / 5;  // 粗略调整（实际需校准）
    while (us--) {
        __NOP();
    }
}

void Delay_ms(uint32_t t)
{
	uint64_t T = t*1000;
	while(T--) {
		delay_1us();
	}
}
  
