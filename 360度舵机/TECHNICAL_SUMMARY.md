# PWM 舵机 + EC11 技术实现总结

本工程已移除原循迹 ADC 采样逻辑，改为使用 CH32V307 的 `TIM1_CH1` 输出舵机 PWM，并用 PC1/PC2 轮询 EC11 旋转编码器。

## 运行链路

```text
main()
├── SystemCoreClockUpdate()
├── Delay_Init()
├── USART_Printf_Init(115200)
├── Servo_PWM_Init()
├── EC11_Init()
├── Servo_Stop()
└── while(1)
    ├── EC11_ReadStep()
    ├── EC11_GetServoLevel(step_interval_ms)
    ├── Servo_Clockwise(level) / Servo_CounterClockwise(level)
    ├── Servo_Stop()
    └── Delay_Ms(1)
```

## 定时器配置

| 项目 | 配置 |
| --- | --- |
| 系统主频 | 96MHz |
| PWM 定时器 | TIM1 |
| PWM 通道 | CH1 |
| PWM 引脚 | PA8 |
| 预分频 | `SystemCoreClock / 1000000 - 1` |
| 计数频率 | 1MHz |
| 自动重装值 | 19999 |
| PWM 周期 | 20ms |

由于计数频率是 1MHz，`TIM1->CH1CVR` 的数值可以直接理解成高电平微秒数。例如：

| CCR1 | 含义 |
| --- | --- |
| 1000 | 1.0ms |
| 1500 | 1.5ms |
| 2000 | 2.0ms |

## 关键接口

```c
void Servo_PWM_Init(void);
void Servo_SetPulseUs(uint16_t pulse_us);
void Servo_Stop(void);
void Servo_Clockwise(uint8_t level);
void Servo_CounterClockwise(uint8_t level);
```

`level` 为 1~5 档，每档相差 0.1ms：

| 接口 | level 1 | level 2 | level 3 | level 4 | level 5 |
| --- | --- | --- | --- | --- | --- |
| `Servo_Clockwise()` | 1.4ms | 1.3ms | 1.2ms | 1.1ms | 1.0ms |
| `Servo_CounterClockwise()` | 1.6ms | 1.7ms | 1.8ms | 1.9ms | 2.0ms |

## EC11 配置

| 项目 | 配置 |
| --- | --- |
| A 相 | PC1 |
| B 相 | PC2 |
| GPIO 模式 | 内部上拉输入 `GPIO_Mode_IPU` |
| 轮询周期 | 1ms |
| 每格动作保持 | 80ms |
| 当前速度档位 | 根据旋转速度自动选择 1~5 档 |

当前主程序上电后先调用 `Servo_Stop()`，舵机不动。每识别到 EC11 一格旋转，就计算这次旋转和上一次旋转之间的时间间隔，再按方向调用 `Servo_Clockwise(level)` 或 `Servo_CounterClockwise(level)`，保持约 `80ms` 后回到停止脉宽 `1500us`。

| 两格间隔 | 舵机档位 |
| --- | --- |
| `<= 80ms` | 5 |
| `<= 140ms` | 4 |
| `<= 240ms` | 3 |
| `<= 400ms` | 2 |
| `> 400ms` | 1 |

## 注意事项

- 舵机信号线接 PA8。
- EC11 A 相接 PC1，B 相接 PC2，公共端接 GND。
- 舵机电源建议单独供 5V。
- 舵机电源 GND 必须和开发板 GND 共地。
- 如果 1500us 附近仍缓慢转动，需要调舵机上的中位电位器，或微调停止脉宽。
- 如果旋钮方向和预期相反，把 `User/main.c` 里的 `EC11_REVERSE_DIRECTION` 改成 `1U`。
