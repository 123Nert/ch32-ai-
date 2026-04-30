#include "servo_pwm.h"

static uint16_t Servo_ClampPulse(uint16_t pulse_us)
{
    if(pulse_us < SERVO_PWM_MIN_US)
    {
        return SERVO_PWM_MIN_US;
    }

    if(pulse_us > SERVO_PWM_MAX_US)
    {
        return SERVO_PWM_MAX_US;
    }

    return pulse_us;
}

void Servo_PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = {0};
    TIM_OCInitTypeDef TIM_OCInitStructure = {0};

    RCC_APB2PeriphClockCmd(SERVO_PWM_RCC_GPIO | SERVO_PWM_RCC_TIM, ENABLE);

    GPIO_InitStructure.GPIO_Pin = SERVO_PWM_GPIO_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(SERVO_PWM_GPIO_PORT, &GPIO_InitStructure);

    TIM_TimeBaseStructure.TIM_Period = (uint16_t)(SERVO_PWM_PERIOD_US - 1U);
    TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t)(SystemCoreClock / 1000000U - 1U);
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(SERVO_PWM_TIM, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCInitStructure.TIM_Pulse = SERVO_PWM_MID_US;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    TIM_OC1Init(SERVO_PWM_TIM, &TIM_OCInitStructure);

    TIM_OC1PreloadConfig(SERVO_PWM_TIM, TIM_OCPreload_Enable);
    TIM_CtrlPWMOutputs(SERVO_PWM_TIM, ENABLE);
    TIM_Cmd(SERVO_PWM_TIM, ENABLE);
}

void Servo_SetPulseUs(uint16_t pulse_us)
{
    TIM_SetCompare1(SERVO_PWM_TIM, Servo_ClampPulse(pulse_us));
}

void Servo_Stop(void)
{
    Servo_SetPulseUs(SERVO_360_STOP_US);
}

void Servo_Clockwise(uint8_t level)
{
    if(level > SERVO_360_LEVEL_MAX)
    {
        level = SERVO_360_LEVEL_MAX;
    }

    if(level == 0U)
    {
        Servo_Stop();
        return;
    }

    Servo_SetPulseUs((uint16_t)(SERVO_360_STOP_US - level * SERVO_360_LEVEL_STEP_US));
}

void Servo_CounterClockwise(uint8_t level)
{
    if(level > SERVO_360_LEVEL_MAX)
    {
        level = SERVO_360_LEVEL_MAX;
    }

    if(level == 0U)
    {
        Servo_Stop();
        return;
    }

    Servo_SetPulseUs((uint16_t)(SERVO_360_STOP_US + level * SERVO_360_LEVEL_STEP_US));
}

void Servo_SetAngle(uint8_t angle)
{
    uint32_t pulse;

    if(angle > 180U)
    {
        angle = 180U;
    }

    pulse = SERVO_PWM_MIN_US;
    pulse += ((uint32_t)angle * (SERVO_PWM_MAX_US - SERVO_PWM_MIN_US)) / 180U;

    Servo_SetPulseUs((uint16_t)pulse);
}

void Servo_SetSpeed(int8_t speed)
{
    int16_t safe_speed = speed;
    uint8_t level;

    if(safe_speed > 100)
    {
        safe_speed = 100;
    }
    else if(safe_speed < -100)
    {
        safe_speed = -100;
    }

    if(safe_speed == 0)
    {
        Servo_Stop();
        return;
    }

    level = (uint8_t)((safe_speed > 0 ? safe_speed : -safe_speed) + 19) / 20;

    if(safe_speed > 0)
    {
        Servo_CounterClockwise(level);
    }
    else
    {
        Servo_Clockwise(level);
    }
}
