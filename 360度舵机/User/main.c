#include "debug.h"
#include "../HardWare/inc/servo_pwm.h"

#define SERVO_360_SPEED_MIN_LEVEL   1U
#define SERVO_360_SPEED_MAX_LEVEL   5U
#define SERVO_RUN_AFTER_STEP_MS     80U
#define EC11_POLL_INTERVAL_MS       1U

#define EC11_LEVEL5_INTERVAL_MS     80U
#define EC11_LEVEL4_INTERVAL_MS     140U
#define EC11_LEVEL3_INTERVAL_MS     240U
#define EC11_LEVEL2_INTERVAL_MS     400U

#define EC11_A_GPIO_PORT            GPIOC
#define EC11_A_GPIO_PIN             GPIO_Pin_1
#define EC11_B_GPIO_PORT            GPIOC
#define EC11_B_GPIO_PIN             GPIO_Pin_2
#define EC11_RCC_GPIO               RCC_APB2Periph_GPIOC

/* 如果旋钮方向和舵机方向相反，把这里改成 1U。 */
#define EC11_REVERSE_DIRECTION      0U

static uint8_t ec11_last_state = 0;
static int8_t ec11_accumulator = 0;

static uint8_t EC11_GetServoLevel(uint32_t step_interval_ms)
{
    if(step_interval_ms <= EC11_LEVEL5_INTERVAL_MS)
    {
        return SERVO_360_SPEED_MAX_LEVEL;
    }
    if(step_interval_ms <= EC11_LEVEL4_INTERVAL_MS)
    {
        return 4U;
    }
    if(step_interval_ms <= EC11_LEVEL3_INTERVAL_MS)
    {
        return 3U;
    }
    if(step_interval_ms <= EC11_LEVEL2_INTERVAL_MS)
    {
        return 2U;
    }

    return SERVO_360_SPEED_MIN_LEVEL;
}

static void EC11_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(EC11_RCC_GPIO, ENABLE);

    GPIO_InitStructure.GPIO_Pin = EC11_A_GPIO_PIN | EC11_B_GPIO_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    ec11_last_state = 0;
    if(GPIO_ReadInputDataBit(EC11_A_GPIO_PORT, EC11_A_GPIO_PIN) == Bit_SET)
    {
        ec11_last_state |= 0x02U;
    }
    if(GPIO_ReadInputDataBit(EC11_B_GPIO_PORT, EC11_B_GPIO_PIN) == Bit_SET)
    {
        ec11_last_state |= 0x01U;
    }
}

static uint8_t EC11_ReadState(void)
{
    uint8_t state = 0;

    if(GPIO_ReadInputDataBit(EC11_A_GPIO_PORT, EC11_A_GPIO_PIN) == Bit_SET)
    {
        state |= 0x02U;
    }
    if(GPIO_ReadInputDataBit(EC11_B_GPIO_PORT, EC11_B_GPIO_PIN) == Bit_SET)
    {
        state |= 0x01U;
    }

    return state;
}

static int8_t EC11_ReadStep(void)
{
    static const int8_t transition_table[16] = {
         0, -1,  1,  0,
         1,  0,  0, -1,
        -1,  0,  0,  1,
         0,  1, -1,  0
    };
    uint8_t state;
    uint8_t index;
    int8_t delta;

    state = EC11_ReadState();
    index = (uint8_t)((ec11_last_state << 2) | state);
    delta = transition_table[index];
    ec11_last_state = state;

#if EC11_REVERSE_DIRECTION
    delta = (int8_t)-delta;
#endif

    ec11_accumulator = (int8_t)(ec11_accumulator + delta);

    if(ec11_accumulator >= 4)
    {
        ec11_accumulator = 0;
        return 1;
    }
    if(ec11_accumulator <= -4)
    {
        ec11_accumulator = 0;
        return -1;
    }

    return 0;
}

/*
    EC11 控制 360度连续舵机
    PA8 输出 TIM1_CH1 PWM，周期 20ms，1500us 停止
    EC11 A相接 PC1，B相接 PC2，旋钮越快，舵机速度档位越高
*/
int main(void)
{
    uint16_t servo_hold_ms = 0;
    uint32_t run_time_ms = 0;
    uint32_t last_step_time_ms = 0;
    uint32_t step_interval_ms;
    uint8_t has_step_time = 0;
    uint8_t servo_level = SERVO_360_SPEED_MIN_LEVEL;
    int8_t ec11_step;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init(115200);

    printf("\r\n");
    printf("========================================\r\n");
    printf("  EC11 Servo Control (CH32V307)\r\n");
    printf("  PA8 -> TIM1_CH1 -> Servo Signal\r\n");
    printf("  EC11 A -> PC1, B -> PC2\r\n");
    printf("  Turn EC11 to move servo, stop when idle\r\n");
    printf("========================================\r\n");

    Servo_PWM_Init();
    EC11_Init();
    Servo_Stop();
    printf("Servo PWM and EC11 initialized.\r\n");
    printf("Servo speed level: %u-%u, hold time: %u ms\r\n\r\n",
           SERVO_360_SPEED_MIN_LEVEL,
           SERVO_360_SPEED_MAX_LEVEL,
           SERVO_RUN_AFTER_STEP_MS);

    while(1)
    {
        ec11_step = EC11_ReadStep();

        if(ec11_step != 0)
        {
            if(has_step_time == 0U)
            {
                step_interval_ms = EC11_LEVEL2_INTERVAL_MS + 1U;
            }
            else
            {
                step_interval_ms = run_time_ms - last_step_time_ms;
            }

            last_step_time_ms = run_time_ms;
            has_step_time = 1U;
            servo_level = EC11_GetServoLevel(step_interval_ms);
        }

        if(ec11_step > 0)
        {
            Servo_Clockwise(servo_level);
            servo_hold_ms = SERVO_RUN_AFTER_STEP_MS;
            printf("EC11 CW, level %u\r\n", servo_level);
        }
        else if(ec11_step < 0)
        {
            Servo_CounterClockwise(servo_level);
            servo_hold_ms = SERVO_RUN_AFTER_STEP_MS;
            printf("EC11 CCW, level %u\r\n", servo_level);
        }

        if(servo_hold_ms > 0U)
        {
            servo_hold_ms--;
            if(servo_hold_ms == 0U)
            {
                Servo_Stop();
            }
        }

        Delay_Ms(EC11_POLL_INTERVAL_MS);
        run_time_ms += EC11_POLL_INTERVAL_MS;
    }
}
