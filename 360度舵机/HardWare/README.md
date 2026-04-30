# HardWare 模块说明

当前硬件层只保留舵机 PWM 驱动，旧的循迹 ADC 模块已删除。

## 文件结构

```text
HardWare/
├── inc/
│   └── servo_pwm.h
└── src/
    └── servo_pwm.c
```

## servo_pwm.h

公开接口和硬件参数定义：

```c
#define SERVO_PWM_TIM        TIM1
#define SERVO_PWM_GPIO_PORT  GPIOA
#define SERVO_PWM_GPIO_PIN   GPIO_Pin_8
```

PA8 对应 TIM1_CH1，输出 50Hz 舵机 PWM。

## servo_pwm.c

实现内容：

- PA8 复用推挽输出初始化
- TIM1 预分频到 1MHz
- TIM1_CH1 输出 20ms 周期 PWM
- `Servo_SetPulseUs()` 直接设置脉宽
- `Servo_Clockwise()` 按 1~5 档顺时针旋转
- `Servo_CounterClockwise()` 按 1~5 档逆时针旋转
- `Servo_Stop()` 输出 1.5ms 停止信号
- `Servo_SetAngle()` 兼容普通 180 度角度舵机

## 使用示例

```c
#include "../HardWare/inc/servo_pwm.h"

Servo_PWM_Init();
Servo_Stop();                 // 1500us，停止
Servo_Clockwise(3);           // 1200us，顺时针中速
Servo_CounterClockwise(3);    // 1800us，逆时针中速
```
