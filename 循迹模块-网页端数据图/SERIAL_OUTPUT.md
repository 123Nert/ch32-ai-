# PC1 循迹传感器数据串口输出说明

## 功能概览

CH32V307 通过以下方式采集和输出 PC1 传感器数据：

```
传感器 AO 脚
    ↓
PC1 (GPIO)
    ↓
ADC1 通道11 (12位转换)
    ↓
数据处理 (原始值 + 平均值)
    ↓
串口输出 (115200波特率)
    ↓
PC 端串口监控工具显示
```

## 串口配置

| 参数 | 值 |
|------|-----|
| **波特率** | 115200 |
| **数据位** | 8 |
| **停止位** | 1 |
| **校验位** | 无 |
| **流控** | 无 |

## 输出数据格式

### 初始化信息

```
========================================
  Line Tracing Module Demo (CH32V307)
  PC1 -> ADC1_CH11 -> Sensor Data
========================================
Line Tracing Module Initialized (PC1 - ADC1_CH11)
Threshold: 2048 (1.65V @ 12-bit ADC)
========================================

      Raw ADC    |    Avg(4x)    |    Status
---------------------------------------------
```

### 实时数据输出（每 500ms 更新一次）

```
     1234      |     1245      |   BLACK
     1256      |     1250      |   BLACK
     3456      |     3445      |   WHITE
     3400      |     3410      |   WHITE
     1200      |     1215      |   BLACK
```

## 数据字段说明

| 字段 | 说明 | 范围 |
|------|------|------|
| **Raw ADC** | 单次采样的原始 ADC 值 | 0-4095 |
| **Avg(4x)** | 连续 4 次采样的平均值 | 0-4095 |
| **Status** | 黑白线判断结果 | BLACK/WHITE |

### 阈值逻辑

```c
Avg(4x) < 2048  →  BLACK  (在黑线上，传感器照度低)
Avg(4x) ≥ 2048  →  WHITE  (在白线上，传感器照度高)
```

## ADC 值与电压对照

CH32V307 ADC 是 12 位分辨率，参考电压 3.3V：

| ADC 值 | 电压 | 含义 |
|--------|------|------|
| 0 | 0.0V | 完全黑色 |
| 1024 | 0.825V | 偏黑色 |
| **2048** | **1.65V** | **阈值** |
| 3072 | 2.475V | 偏白色 |
| 4095 | 3.3V | 完全白色 |

## 串口监控工具推荐

### Windows
- **串口助手**: CH340 官方提供的串口工具
- **putty**: 通用终端工具
- **SecureCRT**: 专业终端工具
- **VS Code 插件**: Serial Port Monitor

### Linux/Mac
```bash
# minicom
minicom -D /dev/ttyUSB0 -b 115200

# screen
screen /dev/ttyUSB0 115200

# picocom
picocom -b 115200 /dev/ttyUSB0
```

## 调试技巧

### 1. 验证串口连接
- 检查驱动是否正常加载
- 确认正确的 COM 口（如 COM3、COM4）
- 拔插 USB 数据线观察设备变化

### 2. 观察 ADC 值变化

**正常情况**:
```
在白纸上:  ADC 值约 3000-4000
在黑线上:  ADC 值约 500-1500
变化曲线:  平滑，无突跳
```

**异常情况**:
```
始终为 0    → 检查 PC1 接线
始终为 4095 → 检查传感器电源
大幅波动   → 降低延时，增加平均次数
```

### 3. 调整阈值

根据环境光线和传感器特性，修改阈值：

**main.c 中修改**:
```c
uint16_t threshold = 1800;  // 改为 1800
```

或在运行时动态调整（可扩展功能）。

### 4. 采样频率

当前设置为每 500ms 采样一次：
```c
Delay_Ms(500);  // 改为 100 可提高采样率
```

**注意**: 高频采样会消耗更多 CPU 资源。

## 扩展应用

### 多传感器支持

```c
// 可扩展为多个传感器
uint16_t sensor1_adc = LineTracing_ReadADC();  // PC1
uint16_t sensor2_adc = ADC_ReadChannel(ADC_Channel_10);  // PC0
uint16_t sensor3_adc = ADC_ReadChannel(ADC_Channel_9);   // PC2

printf("S1:%d  S2:%d  S3:%d\r\n", sensor1_adc, sensor2_adc, sensor3_adc);
```

### 自动阈值调整

```c
// 第一次运行时自动校准
void AutoCalibrate(void)
{
    uint16_t black_adc = LineTracing_ReadADC_Average(10);
    uint16_t white_adc = LineTracing_ReadADC_Average(10);
    threshold = (black_adc + white_adc) / 2;
    printf("Auto Threshold: %d\r\n", threshold);
}
```

### 录制数据到文件

通过串口工具的日志功能保存数据用于离线分析。

---

**最后更新**: 2024年
