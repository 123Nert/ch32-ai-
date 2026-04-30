# EC11 舵机控制串口输出说明

程序通过 `USART3` 打印 EC11 和舵机动作状态，方便确认代码是否已经运行。

## 启动输出

```text
========================================
  EC11 Servo Control (CH32V307)
  PA8 -> TIM1_CH1 -> Servo Signal
  EC11 A -> PC1, B -> PC2
  Turn EC11 to move servo, stop when idle
========================================
Servo PWM and EC11 initialized.
Servo speed level: 1-5, hold time: 80 ms
```

## 运行现象

启动后舵机先停止。旋转 EC11 时，串口会输出方向和当前速度档位；拧得越快，档位越高，舵机转得越快。

```text
EC11 CW, level 1
EC11 CW, level 3
EC11 CW, level 5
EC11 CCW, level 4
```

## 串口参数

| 参数 | 值 |
| --- | --- |
| 波特率 | 115200 |
| 数据位 | 8 |
| 停止位 | 1 |
| 校验 | 无 |
| 默认调试串口 | USART3 TX |

当前 `Debug/debug.h` 中默认 `DEBUG_UART3`，`debug.c` 配置的是 `PB10` 作为 USART3 TX。
