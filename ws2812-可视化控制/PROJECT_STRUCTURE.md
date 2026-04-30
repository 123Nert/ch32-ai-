# 项目结构说明

## 根目录核心文件

```
run_all_no_mqtt.py      - 启动脚本，运行此文件启动整个系统
requirements.txt        - Python 依赖列表
README.md              - 项目说明文档
.gitignore             - Git 忽略文件
```

## 网页界面

```
templates/
  └── mqtt_web_control.html   - 网页控制界面（Flask 会自动加载）
```

## 固件代码（CH32V30X）

```
User/
  ├── main.c                  - 主程序，包含 UART3 和 WS2812 控制逻辑
  ├── ch32v30x_it.c/h         - 中断处理
  └── system_ch32v30x.c/h     - 系统初始化

HardWare/
  ├── ws2812.c                - WS2812 LED 驱动实现
  └── ws2812.h                - WS2812 驱动头文件

Peripheral/                   - CH32V30X 外设驱动库
Core/                         - RISC-V 核心库文件
Startup/                      - 启动代码和链接脚本
Ld/                          - 链接脚本
```

## IDE 配置（可忽略）

```
.mrs/                   - WCH Studio 工作区备份
.claude/                - Claude Code 项目配置
Debug/                  - 编译调试文件
Driver/                 - 驱动程序
```

## 快速开始

1. **安装依赖**
   ```bash
   pip install -r requirements.txt
   ```

2. **烧录固件**
   - 使用 WCH Studio 或 WCH Link 工具将 User/main.c 编译后的固件烧录到 CH32V30X

3. **启动服务**
   ```bash
   python run_all_no_mqtt.py
   ```

4. **使用网页界面**
   - 自动打开 http://localhost:5000
   - 或手动访问该地址

## 重要说明

- **不再需要**：MQTT Broker、flask_backend.py、start.py 等旧文件
- **核心依赖**：Flask, pyserial
- **默认串口**：COM11（可在 run_all_no_mqtt.py 第 293 行修改）
- **默认波特率**：115200
