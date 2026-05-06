# CH32V307 SH1106 OLED 演示工程

这是一个基于 `CH32V307` 的 `SH1106 128x64` 单色 OLED 演示工程，使用 `I2C1` 重映射到 `PB8/PB9` 的硬件 I2C 驱动 4 针脚 OLED 模块。

## 功能说明

- SH1106 初始化与地址探测
- 4 行 x 16 列 ASCII 文本显示
- 十进制、十六进制、二进制数字显示
- 跑马灯文字效果
- 条纹、波浪、扫描条动画
- 对比度渐变与整屏反显演示
- 串口输出当前初始化状态

## 默认接线

- `VCC -> 3.3V`
- `GND -> GND`
- `SCL -> PB8`
- `SDA -> PB9`
- `USART1_TX(PA9) -> USB 转串口 RX`
- `GND -> USB 转串口 GND`

串口波特率为 `115200`。

## 目录结构

- [User/main.c](./User/main.c): 板级初始化与主循环入口
- [User/oled_demo.c](./User/oled_demo.c): OLED 演示逻辑与动画效果
- [User/oled_demo.h](./User/oled_demo.h): 演示模块对外接口
- [Driver/OLED.c](./Driver/OLED.c): SH1106 硬件 I2C 驱动
- [Driver/OLED.h](./Driver/OLED.h): OLED 驱动头文件
- [Driver/OLED_Font.h](./Driver/OLED_Font.h): 8x16 ASCII 字模

## 编译下载

1. 使用 MounRiver Studio 打开工程。
2. 先执行 `Clean Project`。
3. 再执行 `Build`。
4. 连接下载器后执行 `Download`。

## 串口输出示例

```text
CH32V307 SH1106 I2C start
OLED pins: SCL PB8, SDA PB9, VCC 3.3V, GND GND
OLED init: OK, addr=0x78
```

如果 `OLED init: FAIL`，说明程序没有从 OLED 收到 ACK，应优先检查供电、引脚和接线顺序。
