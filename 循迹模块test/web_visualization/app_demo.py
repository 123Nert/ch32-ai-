"""
循迹模块Web可视化应用 - 测试版本
不需要实际硬件即可测试应用功能
使用模拟数据而不是实际串口数据
"""
import threading
import json
import time
import random
from datetime import datetime
from collections import deque
from flask import Flask, render_template
from flask_socketio import SocketIO, emit
import logging

# 配置
MAX_DATA_POINTS = 50  # 保存最近50个数据点

# 初始化Flask应用
app = Flask(__name__)
app.config['SECRET_KEY'] = 'your-secret-key'
socketio = SocketIO(app, cors_allowed_origins="*")

# 日志配置
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# 全局变量
data_buffer = deque(maxlen=MAX_DATA_POINTS)
is_running = False
connect_status = False
current_port = None
current_baud = 115200
simulation_thread = None


def get_simulated_ports():
    """获取模拟的串口列表"""
    return [
        {
            'port': 'COM1',
            'description': '模拟设备1 (USB Serial Port)',
            'hwid': 'USB VID:PID=1234:5678'
        },
        {
            'port': 'COM3',
            'description': '模拟设备2 (CH32V30x)',
            'hwid': 'USB VID:PID=1A86:7523'
        },
        {
            'port': 'COM5',
            'description': '模拟设备3 (USB Serial Port)',
            'hwid': 'USB VID:PID=1234:5678'
        }
    ]


def simulate_sensor_data():
    """生成模拟的传感器数据"""
    # 生成类似循迹传感器的波形数据
    base_value = 2048
    noise = random.randint(-50, 50)

    # 添加一些周期性波动，模拟真实的传感器行为
    time_offset = time.time() % 20  # 20秒周期
    trend = int(1000 * (0.5 + 0.5 * ((time_offset - 10) / 10)))

    value = max(0, min(4095, base_value + trend + noise))
    # 模拟状态：值小于2048为BLACK，否则为WHITE
    status = 'black' if value < 2048 else 'white'
    return value, status


def simulate_data_reading():
    """模拟数据读取线程"""
    global is_running, connect_status

    logger.info("模拟数据读取线程已启动")

    while is_running:
        try:
            if connect_status:
                # 生成模拟ADC值和状态
                value, status = simulate_sensor_data()
                timestamp = datetime.now().strftime('%H:%M:%S.%f')[:-3]

                data_point = {
                    'time': timestamp,
                    'value': value,
                    'status': status
                }
                data_buffer.append(data_point)

                # 广播数据到所有连接的客户端
                socketio.emit('data_update', data_point, skip_sid=None)
                logger.info(f"模拟ADC值: {value} | 状态: {status}")

                # 100ms采样一次
                time.sleep(0.1)
            else:
                time.sleep(0.2)
        except Exception as e:
            logger.error(f"模拟数据读取错误: {e}")
            time.sleep(1)


def connect_serial(port, baud_rate):
    """模拟连接到串口"""
    global connect_status, current_port, current_baud

    try:
        # 模拟连接延迟
        time.sleep(0.5)

        connect_status = True
        current_port = port
        current_baud = baud_rate
        logger.info(f"模拟串口连接成功: {port} @ {baud_rate}")
        return True, f"已连接到 {port} ({baud_rate}) [模拟模式]"
    except Exception as e:
        connect_status = False
        logger.error(f"模拟串口连接失败: {e}")
        return False, f"连接失败: {str(e)}"


def disconnect_serial():
    """断开串口连接"""
    global connect_status, current_port

    try:
        connect_status = False
        current_port = None
        logger.info("模拟串口已断开连接")
        return True, "已断开连接"
    except Exception as e:
        logger.error(f"断开连接失败: {e}")
        return False, f"断开失败: {str(e)}"


@app.route('/')
def index():
    """主页路由"""
    return render_template('index.html')


@socketio.on('connect')
def handle_connect():
    """客户端连接事件"""
    logger.info('客户端已连接 (模拟模式)')
    # 发送连接状态和历史数据
    emit('connection_status', {
        'status': connect_status,
        'message': '已连接' if connect_status else '未连接'
    })
    emit('history_data', list(data_buffer))


@socketio.on('disconnect')
def handle_disconnect():
    """客户端断开连接事件"""
    logger.info('客户端已断开连接')


@socketio.on('get_available_ports')
def handle_get_ports():
    """获取模拟的串口列表"""
    ports = get_simulated_ports()
    logger.info(f"找到 {len(ports)} 个模拟串口")
    emit('available_ports', {
        'ports': ports,
        'current_port': current_port,
        'current_baud': current_baud
    })


@socketio.on('connect_port')
def handle_connect_port(data):
    """连接到指定的串口（模拟）"""
    port = data.get('port')
    baud_rate = data.get('baud_rate', 115200)

    if not port:
        emit('connection_status', {'status': False, 'message': '请选择串口'})
        return

    success, message = connect_serial(port, baud_rate)
    emit('connection_status', {
        'status': success,
        'message': message,
        'port': port if success else None,
        'baud': baud_rate if success else None
    }, skip_sid=None)


@socketio.on('disconnect_port')
def handle_disconnect_port():
    """断开串口连接"""
    success, message = disconnect_serial()
    emit('connection_status', {
        'status': False,
        'message': message
    }, skip_sid=None)


@socketio.on('start_capture')
def handle_start_capture():
    """开始采集数据"""
    global is_running
    if not connect_status:
        emit('error', {'message': '请先连接串口'})
        return
    is_running = True
    logger.info('开始采集数据（模拟）')
    emit('capture_status', {'status': 'running'}, skip_sid=None)


@socketio.on('stop_capture')
def handle_stop_capture():
    """停止采集数据"""
    global is_running
    is_running = False
    logger.info('停止采集数据')
    emit('capture_status', {'status': 'stopped'}, skip_sid=None)


@socketio.on('clear_data')
def handle_clear_data():
    """清除缓存数据"""
    data_buffer.clear()
    logger.info('数据已清除')
    emit('data_cleared', {}, skip_sid=None)


@socketio.on('request_config')
def handle_request_config():
    """客户端请求配置信息"""
    config = {
        'serial_port': current_port,
        'baud_rate': current_baud,
        'max_data_points': MAX_DATA_POINTS,
        'adc_max_value': 4095,
        'capture_status': 'running' if is_running else 'stopped',
        'connection_status': connect_status,
        'mode': 'simulation'
    }
    emit('config_update', config)


def start_simulation_thread():
    """启动模拟数据读取线程"""
    global simulation_thread

    simulation_thread = threading.Thread(target=simulate_data_reading, daemon=True)
    simulation_thread.start()
    logger.info("模拟数据线程已启动")


if __name__ == '__main__':
    try:
        # 启动模拟数据线程
        start_simulation_thread()

        # 启动Flask-SocketIO服务器
        logger.info("=" * 50)
        logger.info("🧪 测试模式 - Web可视化应用")
        logger.info("=" * 50)
        logger.info("✓ 使用模拟数据，无需实际硬件")
        logger.info("✓ 访问地址: http://localhost:5000")
        logger.info("✓ 按 Ctrl+C 停止应用")
        logger.info("=" * 50)

        socketio.run(app, host='0.0.0.0', port=5000, debug=True, allow_unsafe_werkzeug=True)
    except KeyboardInterrupt:
        logger.info("应用被中止")
        is_running = False
    except Exception as e:
        logger.error(f"应用错误: {e}")
        is_running = False
