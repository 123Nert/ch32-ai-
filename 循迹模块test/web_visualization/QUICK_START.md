# 快速开始指南

## 5分钟快速启动

### 步骤1：找到你的串口号

**Windows:**
1. 右键点击"此电脑" → 管理
2. 找到"设备管理器" → 端口（COM和LPT）
3. 找到你的USB设备，记下COM号（如COM3）

**Linux:**
```bash
ls /dev/ttyUSB*  # 或 /dev/ttyACM*
```

**macOS:**
```bash
ls /dev/tty.*
```

### 步骤2：修改串口配置

编辑 `app.py` 第11-12行：

```python
SERIAL_PORT = 'COM3'      # 改为你的串口号
BAUD_RATE = 115200         # 改为你的波特率
```

### 步骤3：运行应用

**Windows:**
```bash
双击 run.bat
```

**Linux/macOS:**
```bash
bash run.sh
```

### 步骤4：打开浏览器

访问 `http://localhost:5000`

---

## 确认串口设备

### 使用Windows串口助手验证

1. 下载串口调试工具（如 putty 或 XCom）
2. 选择对应的COM口和波特率
3. 验证能否接收到数据
4. 确认数据格式

### 常见串口数据格式

```
2048              # 纯数字
ADC: 2048         # 标签格式
ADC值:2048        # 中文标签
```

---

## 检查数据格式是否匹配

如果你的单片机输出格式与默认不同，需要修改 `parse_serial_data()` 函数。

### 例子1：输出格式为 "Value: 2048"

```python
def parse_serial_data(line):
    try:
        line = line.strip()
        if 'Value:' in line:
            return int(line.split(':')[1].strip())
    except:
        return None
```

### 例子2：输出格式为 "2048 2049 2050" (多个空格分隔)

```python
def parse_serial_data(line):
    try:
        values = [int(x) for x in line.strip().split()]
        return values[0] if values else None
    except:
        return None
```

---

## 常见问题

### Q: 启动时提示找不到COM口怎么办？

A: 
1. 检查设备管理器中的COM号是否正确
2. 检查USB设备是否正确连接
3. 尝试重新插拔USB设备

### Q: 收不到数据怎么办？

A:
1. 确认单片机程序正在运行
2. 用串口工具验证是否有数据输出
3. 检查波特率是否正确
4. 检查数据格式解析函数

### Q: 如何让其他电脑访问？

A: 修改 `app.py` 最后一行，改为你的IP地址：

```python
socketio.run(app, host='0.0.0.0', port=5000)  # 允许所有IP访问
```

然后在其他电脑访问：`http://<你的IP>:5000`

---

## 单片机代码示例

### CH32V30x循迹模块输出代码

```c
#include "line_tracing.h"
#include "debug.h"

int main(void) {
    SystemInit();
    USART_Printf_Init(115200);
    LineTracing_Init();
    
    printf("循迹模块启动\n");
    
    while(1) {
        uint16_t adc_value = LineTracing_ReadADC_Average(5);
        printf("%u\n", adc_value);  // 推荐格式：纯数字
        // 或使用: printf("ADC: %u\n", adc_value);
        
        Delay_Ms(100);  // 每100ms采样一次
    }
    
    return 0;
}
```

---

## 性能调整

### 如果数据更新太快卡顿

1. 增加单片机的延迟：
```c
Delay_Ms(200);  // 改为更大的值
```

2. 减少网页更新频率：修改 `index.html` 中的WebSocket处理逻辑

### 如果反应不够快

1. 减少单片机的延迟
2. 减少 `MAX_DATA_POINTS` 的值

---

## 测试应用而不连接实际设备

编辑 `app.py`，注释掉串口连接代码，使用模拟数据：

```python
# 在 read_serial() 函数中添加:
import random
import time

def read_serial():
    global ser, is_running, connect_status
    
    while is_running:
        try:
            # 模拟数据 - 用于测试
            value = random.randint(1000, 3000)
            timestamp = datetime.now().strftime('%H:%M:%S.%f')[:-3]
            data_point = {
                'time': timestamp,
                'value': value
            }
            data_buffer.append(data_point)
            socketio.emit('data_update', data_point, broadcast=True)
            time.sleep(0.2)
        except Exception as e:
            logger.error(f"错误: {e}")
```

---

祝你使用愉快！如有问题，参考README.md获取更多帮助。
