# 循迹模块代码移植说明

## 硬件配置

- **微控制器**: CH32V307
- **传感器**: 循迹传感器
- **接线**: 传感器 AO 脚接 PC1（ADC1 通道 11）
- **工作电压**: 3.3V
- **串口**: 115200 波特率用于数据输出

## 代码结构

### 文件说明

| 文件名 | 说明 |
|--------|------|
| `main.c` | 主程序，循迹模块演示 |
| `line_tracing.h` | 循迹模块头文件，定义接口 |
| `line_tracing.c` | 循迹模块实现文件 |

## 移植要点

### 1. 从 51 单片机到 CH32V307 的主要差异

| 功能 | 51 单片机 | CH32V307 |
|------|----------|---------|
| 串口初始化 | TMOD + SCON | USART_Printf_Init() |
| GPIO 配置 | sbit + bit 操作 | GPIO_InitTypeDef 结构体 |
| ADC 采样 | 需要外部电路 | 内置 12 位 ADC，可直接使用 |
| 延时 | 循环计数 | Delay_Ms() 函数 |
| 时钟配置 | 设置 TH、TL 寄存器 | RCC_ADCCLKConfig() 函数 |

### 2. PC1 引脚映射

- **PC1 → ADC1 通道 11**
- **模式**: GPIO_Mode_AIN (模拟输入)
- **采样时间**: ADC_SampleTime_239Cycles (长采样时间，精度更高)

### 3. ADC 配置参数

- **分辨率**: 12 位 (0-4095)
- **参考电压**: 3.3V
- **阈值**: 2048 (对应约 1.65V)

### 4. 工作原理

```
传感器 AO 输出
    ↓
模拟电压 (0-3.3V)
    ↓
PC1 → ADC1
    ↓
ADC 转换 (0-4095)
    ↓
与阈值比较判断黑白线
```

## 函数接口

### `void LineTracing_Init(void)`

初始化循迹模块，包括：
- GPIO 时钟和 ADC 时钟使能
- PC1 配置为模拟输入
- ADC 参数初始化
- ADC 校准

**使用示例**:
```c
LineTracing_Init();
```

### `uint16_t LineTracing_ReadADC(void)`

读取原始 ADC 值（0-4095）

**返回值**: 12 位 ADC 转换结果

**使用示例**:
```c
uint16_t adc_val = LineTracing_ReadADC();
```

### `uint8_t LineTracing_IsOnLine(uint16_t threshold)`

判断是否在黑线上

**参数**:
- `threshold`: 判断阈值 (0-4095)

**返回值**:
- 1: 在黑线上（ADC 值 < 阈值）
- 0: 在白线上（ADC 值 ≥ 阈值）

**使用示例**:
```c
uint8_t status = LineTracing_IsOnLine(2048);
```

### `uint16_t LineTracing_ReadADC_Average(uint8_t samples)`

多次采样取平均值，降低噪声

**参数**:
- `samples`: 采样次数

**返回值**: 平均 ADC 值

**使用示例**:
```c
uint16_t avg = LineTracing_ReadADC_Average(4);  // 采样 4 次
```

## 程序流程

```
系统初始化
    ↓
串口初始化 (115200)
    ↓
循迹模块初始化 (LineTracing_Init)
    ↓
主循环:
    - 读取 ADC 原始值
    - 读取 4 次平均值
    - 判断黑白线
    - 串口打印结果
    - 延时 500ms
    ↓
返回主循环
```

## 调试建议

1. **阈值调整**: 根据实际环境光线调整阈值（默认 2048）
   - 黑线 ADC 值通常在 500-1500
   - 白线 ADC 值通常在 2500-4095

2. **采样次数**: 增加采样次数可以降低噪声，但会增加延时

3. **串口监控**: 通过串口观察 ADC 值变化，确定合适阈值
   ```
   Raw ADC: 1234  |  Avg(4x): 1245  |  Status: BLACK
   Raw ADC: 3456  |  Avg(4x): 3445  |  Status: WHITE
   ```

4. **采样时间**: ADC_SampleTime_239Cycles 采样时间最长，精度最高
   - 若需要更高速度，可改为 ADC_SampleTime_71Cycles 或更短

## 应用示例

### 简单的黑线检测

```c
while(1)
{
    if(LineTracing_IsOnLine(2048))
    {
        /* 在黑线上，执行相应控制 */
        printf("On black line\r\n");
    }
    else
    {
        /* 在白线上 */
        printf("On white line\r\n");
    }
    Delay_Ms(100);
}
```

### 多传感器扩展（模板）

如需接多个循迹传感器，可扩展本模块：
- PA0, PA1 等其他 ADC 通道
- 创建传感器数组
- 分别初始化和读取各通道

## 常见问题

### 1. ADC 值始终为 0 或 4095
- 检查 PC1 接线是否正确
- 检查是否完成了 ADC 校准
- 检查传感器供电

### 2. ADC 值波动大
- 增加采样次数取平均
- 增加采样时间 (ADC_SampleTime_xxx)
- 检查电源纹波

### 3. 阈值无法调节
- 观察串口输出的 ADC 值范围
- 根据范围调整阈值
- 进行黑白线实际测试

## 与 51 代码的对应关系

| 51 程序 | CH32V307 移植 |
|---------|-------------|
| `Initial_com()` | `LineTracing_Init()` |
| `if(key1==0)` | 可扩展为按键输入改变阈值 |
| `SBUF=0X01` | 串口发送数据 (可扩展) |
| 获取传感器值 | `LineTracing_ReadADC()` |
| 按键消抖延时 | `ADC_SampleTime_239Cycles` |

## 编译注意

确保项目配置中包含以下文件：
- `line_tracing.c` - 编译到项目
- `line_tracing.h` - 在 main.c 中包含
- 外设库文件（ch32v30x_adc.c、ch32v30x_gpio.c 等）

---

**修改历史**
- 2024: 初版 - 完成基础功能移植
