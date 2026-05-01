//#include <iostm8s105k6.h>
#include "stm8s.h"
#include "encoder.h"
#include "ILI9163.h"
//#include "stm8s105k6.h"
//#include "stm8S105k.h"


unsigned int count=500;

void MY_EXTI_Config(void)
{
  GPIO_Init(GPIOB, GPIO_PIN_0, GPIO_MODE_IN_PU_IT);
  EXTI_SetExtIntSensitivity(EXTI_PORT_GPIOB, EXTI_SENSITIVITY_RISE_ONLY);
  
  GPIO_Init(GPIOB, GPIO_PIN_1, GPIO_MODE_IN_PU_NO_IT);
}

#pragma vector=6
__interrupt void EXTI_PORTB_IRQHandler(void)
{

 // if(GPIOB->IDR&0x02==0)
  if(GPIO_ReadInputPin(GPIOB, GPIO_PIN_1)==0)
  {
    if(count<MAX) count++;
  }
  else
  {
    if(count>MIN) count--; 
  }
}