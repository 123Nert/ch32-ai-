# Hardware 目录结构说明

## 目录布局

```
Hardware/
├── inc/
│   └── line_tracing.h           # 循迹传感器模块头文件
└── src/
    └── line_tracing.c           # 循迹传感器模块实现文件
```

## 文件说明

### line_tracing.h
循迹传感器模块的公开接口定义，包含：
- 硬件配置宏定义
- 函数声明
- 常量定义

**关键定义**:
```c
#define LINE_TRACING_GPIO_PIN       GPIO_Pin_1
#define LINE_TRACING_ADC_CHANNEL    ADC_Channel_11
#define LINE_TRACING_DEFAULT_THRESHOLD    2048
```

### line_tracing.c
循迹传感器模块的实现，包含：
- ADC1 初始化
- GPIO PC1 配置
- ADC 校准
- 数据采集和处理

## 使用方式

### 在 main.c 中包含头文件

```c
#include "debug.h"
#include "../Hardware/inc/line_tracing.h"
```

### 项目编译配置

确保在编译器的源文件列表中加入：
- `Hardware/src/line_tracing.c`

并在包含路径中加入：
- `Hardware/inc`

## 集成步骤

1. **头文件搜索路径**
   - 添加 `${PROJECT_DIR}/Hardware/inc` 到包含目录

2. **源文件编译**
   - 将 `Hardware/src/line_tracing.c` 加入编译文件列表

3. **在应用代码中使用**
   ```c
   LineTracing_Init();                              // 初始化
   uint16_t adc = LineTracing_ReadADC();           // 读取原始值
   uint8_t status = LineTracing_IsOnLine(2048);   // 判断黑白线
   ```

## 扩展指南

### 添加新的传感器模块

可按此结构添加其他硬件模块：
```
Hardware/
├── inc/
│   ├── line_tracing.h
│   ├── motor_control.h
│   └── distance_sensor.h
└── src/
    ├── line_tracing.c
    ├── motor_control.c
    └── distance_sensor.c
```

### 模块命名约定

- **头文件**: `module_name.h`
- **实现文件**: `module_name.c`
- **函数前缀**: `ModuleName_FunctionName()`
- **宏定义**: `MODULE_NAME_CONSTANT`

---

**最后更新**: 2024年
