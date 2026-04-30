#ifndef __SERVO_PWM_H__
#define __SERVO_PWM_H__

#include "debug.h"

/* Servo PWM output: PA8 -> TIM1_CH1 */
#define SERVO_PWM_TIM                  TIM1
#define SERVO_PWM_GPIO_PORT            GPIOA
#define SERVO_PWM_GPIO_PIN             GPIO_Pin_8
#define SERVO_PWM_RCC_GPIO             RCC_APB2Periph_GPIOA
#define SERVO_PWM_RCC_TIM              RCC_APB2Periph_TIM1

/* TIM1 runs at 1 MHz after prescale, so CCR value equals pulse width in us. */
#define SERVO_PWM_PERIOD_US            20000U
#define SERVO_PWM_MIN_US               500U
#define SERVO_PWM_MID_US               1500U
#define SERVO_PWM_MAX_US               2500U

/* MG996R 360 servo: 1500 us stops, every 100 us is one speed level. */
#define SERVO_360_LEVEL_MAX            5U
#define SERVO_360_LEVEL_STEP_US        100U
#define SERVO_360_STOP_US              SERVO_PWM_MID_US
#define SERVO_PWM_SPEED_MIN_US         1000U
#define SERVO_PWM_SPEED_MAX_US         2000U

void Servo_PWM_Init(void);
void Servo_SetPulseUs(uint16_t pulse_us);
void Servo_SetAngle(uint8_t angle);
void Servo_SetSpeed(int8_t speed);
void Servo_Stop(void);
void Servo_Clockwise(uint8_t level);
void Servo_CounterClockwise(uint8_t level);

#endif /* __SERVO_PWM_H__ */
