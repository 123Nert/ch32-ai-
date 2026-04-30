# 循迹模块Web可视化应用配置

## 串口配置
SERIAL_PORT = 'COM3'  # 修改为你的串口号
BAUD_RATE = 115200     # 串口波特率

## Flask应用配置
FLASK_HOST = '0.0.0.0'
FLASK_PORT = 5000
DEBUG = True

## 数据缓冲配置
MAX_DATA_POINTS = 200  # 最多保存200个数据点
ADC_MIN_VALUE = 0      # ADC最小值
ADC_MAX_VALUE = 4095   # ADC最大值（12位）

## 日志配置
LOG_LEVEL = 'INFO'
