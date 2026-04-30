"""
循迹模块数据Web可视化应用
Flask后端 + WebSocket实时推送
"""
import serial
import serial.tools.list_ports
import threading
import json
import time
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
ser = None
data_buffer = deque(maxlen=MAX_DATA_POINTS)
is_running = False
connect_status = False
current_port = None
current_baud = 115200


def parse_serial_data(line):
    """
    解析串口数据 - 支持多种格式
    返回 (value, status) 元组或 None
    """
    try:
        line = line.strip()
        if not line:
            return None

        # 移除时间戳前缀（如 [18:29:34.327]Rx: 或 [18:29:34.327]）
        if '[' in line and ']' in line:
            line = line.split(']', 1)[-1]
            # 移除 Rx: 或 Tx: 前缀
            if ':' in line:
                line = line.split(':', 1)[-1]
            line = line.strip()

        if not line:
            return None

        # 识别状态（BLACK 或 WHITE）
        status = 'black' if 'BLACK' in line.upper() else 'white'

        # 尝试直接解析为整数
        try:
            value = int(line)
            if 0 <= value <= 4095:
                return (value, status)
        except ValueError:
            pass

        # 尝试提取第一个数字（用于格式如 "3762 | 3761 | BLACK"）
        import re
        numbers = re.findall(r'\d+', line)
        if numbers:
            # 取第一个数字
            value = int(numbers[0])
            if 0 <= value <= 4095:
                return (value, status)

        return None
    except Exception as e:
        logger.warning(f"解析数据异常: {line} - {e}")
        return None


def get_available_ports():
    """获取可用的串口列表"""
    ports = []
    try:
        for port_info in serial.tools.list_ports.comports():
            ports.append({
                'port': port_info.device,
                'description': port_info.description,
                'hwid': port_info.hwid
            })
    except Exception as e:
        logger.error(f"获取串口列表失败: {e}")
    return ports


def connect_serial(port, baud_rate):
    """连接到指定的串口"""
    global ser, connect_status, current_port, current_baud

    try:
        if ser and ser.is_open:
            ser.close()

        ser = serial.Serial(port, baud_rate, timeout=1)
        connect_status = True
        current_port = port
        current_baud = baud_rate
        logger.info(f"串口连接成功: {port} @ {baud_rate}")
        return True, f"已连接到 {port} ({baud_rate})"
    except Exception as e:
        connect_status = False
        logger.error(f"串口连接失败: {e}")
        return False, f"连接失败: {str(e)}"


def disconnect_serial():
    """断开串口连接"""
    global ser, connect_status, current_port

    try:
        if ser and ser.is_open:
            ser.close()
        connect_status = False
        current_port = None
        logger.info("串口已断开连接")
        return True, "已断开连接"
    except Exception as e:
        logger.error(f"断开连接失败: {e}")
        return False, f"断开失败: {str(e)}"


def read_serial():
    """读取串口数据的线程函数"""
    global ser, is_running, connect_status

    read_count = 0
    success_count = 0
    last_log_time = time.time()

    logger.info("🔵 read_serial线程已启动，等待数据...")

    while True:  # 改为无限循环，不依赖is_running
        try:
            # 检查是否应该采集
            if is_running and ser and ser.is_open and connect_status:
                try:
                    # 读取可用的数据
                    if ser.in_waiting > 0:
                        line = ser.readline().decode('utf-8', errors='ignore')
                        read_count += 1

                        # 打印原始数据
                        logger.info(f"📥 原始数据[{read_count}]: {repr(line)}")

                        # 解析数据
                        parsed_result = parse_serial_data(line)

                        if parsed_result is not None:
                            value, status = parsed_result
                            if 0 <= value <= 4095:  # ADC是12位的
                                timestamp = datetime.now().strftime('%H:%M:%S.%f')[:-3]
                                data_point = {
                                    'time': timestamp,
                                    'value': value,
                                    'status': status
                                }
                                data_buffer.append(data_point)
                                success_count += 1

                                # 广播数据到所有连接的客户端
                                socketio.emit('data_update', data_point, skip_sid=None)
                                logger.info(f"✓ ADC值: {value} | 状态: {status}")
                            else:
                                logger.warning(f"⚠️ 值超出范围: {value}")
                        else:
                            logger.warning(f"⚠️ 解析失败: {repr(line)}")

                        # 定期输出统计
                        current_time = time.time()
                        if current_time - last_log_time > 5:
                            success_rate = (success_count * 100 // read_count) if read_count > 0 else 0
                            logger.info(f"📊 统计: 收{read_count}行 | 有效{success_count}个 | 成功率{success_rate}%")
                            last_log_time = current_time
                    else:
                        # 没有数据时短暂延迟
                        threading.Event().wait(0.01)
                except Exception as e:
                    logger.error(f"❌ 读取异常: {e}", exc_info=True)
                    threading.Event().wait(0.5)
            else:
                # 等待条件满足
                threading.Event().wait(0.1)

        except KeyboardInterrupt:
            logger.info("线程被中断")
            break
        except Exception as e:
            logger.error(f"❌ 线程错误: {e}", exc_info=True)
            threading.Event().wait(1)


@app.route('/')
def index():
    """主页路由"""
    return render_template('index.html')


@socketio.on('connect')
def handle_connect():
    """客户端连接事件"""
    logger.info('客户端已连接')
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
    """获取可用的串口列表"""
    ports = get_available_ports()
    logger.info(f"找到 {len(ports)} 个可用串口")
    emit('available_ports', {
        'ports': ports,
        'current_port': current_port,
        'current_baud': current_baud
    })


@socketio.on('connect_port')
def handle_connect_port(data):
    """连接到指定的串口"""
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
    })


@socketio.on('disconnect_port')
def handle_disconnect_port():
    """断开串口连接"""
    success, message = disconnect_serial()
    emit('connection_status', {
        'status': False,
        'message': message
    })


@socketio.on('start_capture')
def handle_start_capture():
    """开始采集数据"""
    global is_running
    if not connect_status:
        logger.error("无法开始采集：未连接到串口")
        emit('error', {'message': '请先连接串口'})
        return
    is_running = True
    logger.info('开始采集数据')
    emit('capture_status', {'status': 'running'})


@socketio.on('stop_capture')
def handle_stop_capture():
    """停止采集数据"""
    global is_running
    is_running = False
    logger.info('停止采集数据')
    emit('capture_status', {'status': 'stopped'})


@socketio.on('clear_data')
def handle_clear_data():
    """清除缓存数据"""
    data_buffer.clear()
    logger.info('数据已清除')
    emit('data_cleared', {})


@socketio.on('request_config')
def handle_request_config():
    """客户端请求配置信息"""
    config = {
        'serial_port': current_port,
        'baud_rate': current_baud,
        'max_data_points': MAX_DATA_POINTS,
        'adc_max_value': 4095,
        'capture_status': 'running' if is_running else 'stopped',
        'connection_status': connect_status
    }
    emit('config_update', config)


def start_serial_thread():
    """启动串口读取线程"""
    global is_running
    is_running = False  # 初始状态为停止

    # 启动读取线程（不自动连接）
    serial_thread = threading.Thread(target=read_serial, daemon=True)
    serial_thread.start()
    logger.info("串口读取线程已启动（等待手动连接）")


if __name__ == '__main__':
    try:
        # 启动串口线程
        start_serial_thread()

        # 启动Flask-SocketIO服务器
        logger.info("Web服务器启动在 http://localhost:5000")
        socketio.run(app, host='0.0.0.0', port=5000, debug=True, allow_unsafe_werkzeug=True)
    except KeyboardInterrupt:
        logger.info("应用被中止")
        is_running = False
        if ser and ser.is_open:
            ser.close()
    except Exception as e:
        logger.error(f"应用错误: {e}")
        is_running = False
        if ser and ser.is_open:
            ser.close()
