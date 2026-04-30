# 诊断指南 - 无法接收串口数据

如果你连接上了但看不到数据，按照这个诊断步骤进行：

## 🔍 诊断步骤

### 步骤1：检查诊断面板

连接后，右侧控制面板会自动显示**诊断信息**面板。

查看是否显示：
```
📥 原始: ...
🔤 16进制: ...
```

- ✅ **有数据显示** → 说明串口有数据，问题在解析
- ❌ **显示"--"** → 说明串口没有接收到数据

---

## 如果诊断面板显示数据

### 问题：接收到数据但无法解析

**可能原因：**
1. 数据格式与预期不符
2. 数据超出ADC范围（0-4095）
3. 数据包含特殊字符

**解决方案：**

1. **查看诊断面板的16进制数据**
   - 例如：`32 30 34 38 0a` 代表 "2048\n"
   - 如果看到其他字符，需要修改解析函数

2. **检查服务器日志**
   - 打开终端窗口，查看 python app.py 的输出
   - 会显示接收到的原始数据和解析结果

3. **修改 parse_serial_data() 函数**

**常见数据格式示例：**

```python
# 如果接收到: "2048\n"
# 16进制: 32 30 34 38 0a
# ✅ 支持（默认已支持）

# 如果接收到: "ADC=2048\n"
# 16进制: 41 44 43 3d 32 30 34 38 0a
# ✅ 支持（已自动检测）

# 如果接收到: "adc_val:2048;"
# 需要修改parse_serial_data()函数
def parse_serial_data(line):
    if 'adc_val:' in line:
        value = int(line.split(':')[1].split(';')[0])
        if 0 <= value <= 4095:
            return value
    return None
```

---

## 如果诊断面板显示 "--"（无数据）

### 问题：串口连接了但没有数据

**检查清单：**

1. ✓ 单片机程序是否正在运行？
   ```bash
   # 用串口工具（putty）直接测试
   # 或用Windows自带的串口助手
   ```

2. ✓ 串口波特率是否正确？
   ```c
   // 检查你的代码
   USART_Printf_Init(115200);  // 确保与应用匹配
   ```

3. ✓ printf输出是否打开了？
   ```c
   // 检查是否有这样的初始化
   USART_Printf_Init(115200);
   SDI_Printf_Enable();  // 某些项目需要这行
   ```

4. ✓ 数据输出是否有延迟？
   ```c
   // 应该定期输出，例如：
   while(1) {
       uint16_t val = LineTracing_ReadADC();
       printf("%u\n", val);
       Delay_Ms(100);  // 100ms输出一次
   }
   ```

---

## 🧪 用测试模式验证

如果用硬件无法调试，用测试模式验证应用是否正常：

```bash
python app_demo.py
```

1. 选择模拟串口（COM1、COM3或COM5）
2. 点击连接
3. 点击开始采集
4. **应该立即看到曲线和数据**

如果测试模式工作正常，说明应用没问题，问题在硬件或数据格式。

---

## 常见单片机代码问题

### ❌ 问题1：没有初始化printf

```c
int main(void) {
    SystemInit();
    // ❌ 缺少这一行
    // USART_Printf_Init(115200);
    
    while(1) {
        printf("%u\n", 2048);  // 无法输出！
    }
}
```

✅ **修复：** 添加初始化
```c
int main(void) {
    SystemInit();
    USART_Printf_Init(115200);  // ✓ 添加这行
    
    while(1) {
        printf("%u\n", 2048);
    }
}
```

### ❌ 问题2：没有调用printf

```c
int main(void) {
    SystemInit();
    USART_Printf_Init(115200);
    
    while(1) {
        uint16_t value = LineTracing_ReadADC();
        // ❌ 缺少printf！
        // 什么都没输出
    }
}
```

✅ **修复：** 添加printf
```c
int main(void) {
    SystemInit();
    USART_Printf_Init(115200);
    
    while(1) {
        uint16_t value = LineTracing_ReadADC();
        printf("%u\n", value);  // ✓ 添加这行
        Delay_Ms(100);
    }
}
```

### ❌ 问题3：输出频率太高

```c
while(1) {
    printf("%u\n", LineTracing_ReadADC());
    // ❌ 没有延迟！每毫秒输出1000次
}
```

✅ **修复：** 添加延迟
```c
while(1) {
    printf("%u\n", LineTracing_ReadADC());
    Delay_Ms(100);  // ✓ 每100ms输出一次
}
```

---

## 📝 完整的工作代码示例

```c
#include "debug.h"
#include "line_tracing.h"

int main(void) {
    // 1. 初始化系统和UART
    SystemInit();
    USART_Printf_Init(115200);
    
    // 2. 初始化循迹模块
    LineTracing_Init();
    
    // 3. 开始采集
    printf("System initialized\n");
    
    while(1) {
        // 4. 读取ADC值
        uint16_t adc_value = LineTracing_ReadADC_Average(5);
        
        // 5. 发送数据
        printf("%u\n", adc_value);
        
        // 6. 等待
        Delay_Ms(100);
    }
    
    return 0;
}
```

---

## 🔧 快速修复建议

**如果还是无法接收数据，按优先级尝试：**

1. 📍 用putty或串口助手直接测试硬件
2. 📍 重新上传单片机程序
3. 📍 检查USB驱动是否正确安装
4. 📍 换一根USB线
5. 📍 用不同的USB口
6. 📍 重启电脑和设备

---

## 📞 反馈日志信息

如果以上都不行，收集以下信息：

1. **诊断面板的原始数据** - 截图或复制文本
2. **服务器日志** - python窗口的完整输出
3. **单片机代码** - 特别是printf那部分
4. **硬件信息** - 设备型号、USB芯片等

提供这些信息后更容易诊断问题！
