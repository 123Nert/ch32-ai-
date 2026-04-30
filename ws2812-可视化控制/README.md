# WS2812 LED 控制系统

一个基于 Flask + 网页界面的 WS2812 RGB LED 控制系统，支持个别 LED 控制、全局填充、动画效果等功能。

## 项目结构

```
.
├── run_all_no_mqtt.py          # 核心后端程序（Flask + 串口控制）
├── templates/
│   └── mqtt_web_control.html   # 网页控制界面
├── User/
│   └── main.c                  # CH32V30X 固件源代码
├── HardWare/
│   ├── ws2812.c                # WS2812 驱动实现
│   └── ws2812.h                # WS2812 驱动头文件
├── Peripheral/                 # CH32V30X 外设库
├── Core/                        # RISC-V 核心库
├── Startup/                     # 启动代码
└── README.md                   # 本文件
```

## 功能特性

- **个别 LED 控制**：指定 LED 编号和 RGB 颜色
- **全局控制**：同时控制全部 8 个 LED
- **动画效果**：
  - 🏃 流水灯（支持正向/反向）
  - 🌈 彩虹灯（循环显示颜色变化）
  - 💨 呼吸灯（支持自定义颜色）
- **实时状态显示**：网页界面实时显示每个 LED 的状态

## 硬件要求

- CH32V30X 微控制器
- WS2812B RGB LED 灯条（8 颗 LED）
- USB 串口适配器（或集成 USB-UART）

## 软件要求

```bash
pip install flask flask-cors pyserial
```

## 快速开始

### 1. 固件烧录

**方法1：使用 WCH Studio**
1. 打开 WCH Studio
2. 打开项目：文件 → 打开项目 → 选择本目录中的 `第二讲.wvproj`
3. 编译：Project → Build Project
4. 烧录：Run → Download and Run（或按 Ctrl+F11）

**方法2：命令行烧录**
```bash
# 在 WCH Studio 中编译生成 .bin 文件后
# 使用 WCH Link 工具烧录
```

**固件配置**：
- UART3 (PB10 TX, PB11 RX)
- 波特率: 115200
- 支持命令: `SET`, `FILL`, `CLEAR`

### 2. 启动后端服务

```bash
python run_all_no_mqtt.py
```

程序会：
1. 自动检测并连接到 COM11 串口
2. 启动 Flask 服务（http://localhost:5000）
3. 打开网页控制界面

### 3. 使用网页界面

在浏览器中访问 `http://localhost:5000`，可以：
- 选择 LED 编号和颜色进行控制
- 使用快速色按钮（红、绿、蓝、白）
- 启动动画效果
- 实时查看 LED 状态和通信日志

## 串口命令格式

固件支持以下串口命令：

```
SET,<index>,<r>,<g>,<b>   # 设置单个 LED，index=0-7
FILL,<r>,<g>,<b>          # 填充全部 LED
CLEAR                      # 关闭全部 LED
```

### 示例

```
SET,0,255,0,0       # LED 0 设置为红色
FILL,0,255,0        # 全部 LED 设置为绿色
CLEAR               # 关闭全部 LED
```

## 故障排除

### 串口无法连接

1. 检查 USB 串口驱动是否安装
2. 确认正确的 COM 口：
   ```bash
   python -m serial.tools.list_ports
   ```
3. 修改 `run_all_no_mqtt.py` 第 293 行的串口号

### 固件无响应

1. 确保固件已正确烧录到 CH32V30X
2. 检查 PB10/PB11 是否正确连接
3. 在固件中添加测试打印确认 UART3 初始化成功

### 动画卡顿

- 动画之间的延迟时间已优化，确保平滑切换
- 如需调整速度，修改 `run_all_no_mqtt.py` 中的 `time.sleep()` 值

## 文件说明

### run_all_no_mqtt.py

核心后端程序，包含：
- Flask 应用和路由处理
- 串口初始化和命令发送
- 动画效果实现（流水、彩虹、呼吸）
- 动画线程管理

主要 API 端点：
- `POST /api/led/set` - 设置单个 LED
- `POST /api/led/fill` - 全灯填充
- `POST /api/led/clear` - 全灯关闭
- `POST /api/led/animation` - 播放动画
- `GET /api/status` - 获取系统状态

### templates/mqtt_web_control.html

单页面网页应用，包含：
- LED 实时显示网格
- 单 LED 控制面板
- 全局控制面板
- 动画控制选项
- 通信日志面板

### User/main.c

CH32V30X 固件源代码，包含：
- UART3 串口初始化
- LED 命令解析和执行
- WS2812 LED 驱动调用

## 开发笔记

### 已知限制

- 只支持最多 8 个 LED（可修改代码扩展）
- 动画播放时无法同时接收手动控制（切换模式需等待动画停止）
- 网页界面需要现代浏览器支持 Fetch API

### 扩展建议

- 增加更多动画效果（彩色跑马灯、呼吸多色、随机闪烁等）
- 支持预设配置保存和加载
- 添加效果速度调整滑块
- 实现定时控制和场景管理

## 许可证

MIT License

## 维护者

用户项目 - WS2812 LED 控制系统
