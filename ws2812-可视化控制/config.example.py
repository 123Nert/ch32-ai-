# WS2812 LED 控制系统配置示例
# 将此文件复制为 config.py，并根据需要修改参数

# 串口配置
SERIAL_PORT = 'COM11'          # 串口号，使用 python -m serial.tools.list_ports 查看
SERIAL_BAUDRATE = 115200       # 波特率

# Flask 配置
FLASK_HOST = 'localhost'       # 服务绑定地址
FLASK_PORT = 5000             # 服务端口
FLASK_DEBUG = True            # 调试模式

# LED 配置
LED_COUNT = 8                 # LED 数量

# 动画配置
ANIMATION_SPEED = {
    'running': 0.2,           # 流水灯速度（秒）
    'rainbow': 0.1,           # 彩虹速度（秒）
    'breathing': 0.05         # 呼吸灯速度（秒）
}

# 默认颜色预设
PRESET_COLORS = {
    'red': (255, 0, 0),
    'green': (0, 255, 0),
    'blue': (0, 0, 255),
    'white': (255, 255, 255),
    'yellow': (255, 255, 0),
    'cyan': (0, 255, 255),
    'magenta': (255, 0, 255),
    'off': (0, 0, 0)
}
