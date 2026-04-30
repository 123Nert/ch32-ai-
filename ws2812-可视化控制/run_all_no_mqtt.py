#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
LED 控制系统 - 简化版（无需 MQTT Broker）
直接通过 Flask 和串口通信
"""

from flask import Flask, render_template, jsonify, request
from flask_cors import CORS
import serial
import serial.tools.list_ports
import json
import logging
import threading
import time

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = Flask(__name__)
CORS(app)

# 全局变量
ser = None
led_states = [[0, 0, 0] for _ in range(8)]
animation_thread = None
animation_running = False
animation_mode = None

def init_serial(port='COM11', baudrate=115200):
    """初始化串口"""
    global ser
    try:
        ser = serial.Serial(port, baudrate, timeout=1)
        logger.info(f"串口已连接: {port}")
        time.sleep(1)
        return True
    except Exception as e:
        logger.error(f"串口连接失败: {e}")
        return False

def send_serial_command(cmd):
    """发送串口命令"""
    global ser
    if not ser or not ser.is_open:
        logger.error("串口未连接")
        return False

    try:
        full_cmd = cmd + '\r\n'
        ser.write(full_cmd.encode())
        logger.info(f"发送命令: {cmd}")
        return True
    except Exception as e:
        logger.error(f"发送失败: {e}")
        return False

def animation_running_light(direction='forward', color=(255, 0, 0), speed=0.2):
    """流水灯动画"""
    global animation_running, led_states
    r, g, b = color
    # 确保颜色值是整数
    r, g, b = int(r), int(g), int(b)
    speed = float(speed)
    # 限制速度在合理范围内（0.15-0.5秒）
    speed = max(0.15, min(0.5, speed))
    logger.info(f"启动流水灯效果 ({direction}) RGB({r},{g},{b}) 速度{speed}s")

    while animation_running:
        if direction == 'forward':
            # 正向：0 -> 7
            for i in range(8):
                if not animation_running:
                    break
                send_serial_command('CLEAR')
                time.sleep(0.05)  # 关灯延迟
                send_serial_command(f'SET,{i},{r},{g},{b}')
                time.sleep(speed)  # 点亮延迟
        else:
            # 反向：7 -> 0
            for i in range(7, -1, -1):
                if not animation_running:
                    break
                send_serial_command('CLEAR')
                time.sleep(0.05)  # 关灯延迟
                send_serial_command(f'SET,{i},{r},{g},{b}')
                time.sleep(speed)  # 点亮延迟

        if not animation_running:
            break

    logger.info("流水灯效果停止")

def animation_rainbow():
    """彩虹动画"""
    global animation_running
    logger.info("启动彩虹效果")

    colors = [
        (255, 0, 0),      # 红
        (255, 127, 0),    # 橙
        (255, 255, 0),    # 黄
        (0, 255, 0),      # 绿
        (0, 127, 255),    # 青
        (0, 0, 255),      # 蓝
        (139, 0, 139),    # 紫
        (255, 0, 127),    # 品红
    ]

    while animation_running:
        for i, (r, g, b) in enumerate(colors):
            if not animation_running:
                break
            send_serial_command(f'SET,{i},{r},{g},{b}')
            time.sleep(0.15)  # 彩虹色延迟

        time.sleep(0.3)  # 循环间隔

        if not animation_running:
            break

    logger.info("彩虹效果停止")

def animation_breathing(color=(255, 0, 0)):
    """呼吸灯动画"""
    global animation_running
    r, g, b = color
    r, g, b = int(r), int(g), int(b)
    logger.info(f"启动呼吸灯效果 RGB({r},{g},{b})")

    while animation_running:
        # 亮起（0 -> 255）
        for brightness in range(0, 256, 15):  # 步进调整为15
            if not animation_running:
                break
            brightness_r = int(r * brightness / 255)
            brightness_g = int(g * brightness / 255)
            brightness_b = int(b * brightness / 255)
            send_serial_command(f'FILL,{brightness_r},{brightness_g},{brightness_b}')
            time.sleep(0.08)  # 呼吸亮起延迟

        # 暗下（255 -> 0）
        for brightness in range(255, -1, -15):  # 步进调整为15
            if not animation_running:
                break
            brightness_r = int(r * brightness / 255)
            brightness_g = int(g * brightness / 255)
            brightness_b = int(b * brightness / 255)
            send_serial_command(f'FILL,{brightness_r},{brightness_g},{brightness_b}')
            time.sleep(0.08)  # 呼吸暗下延迟

        if not animation_running:
            break

    logger.info("呼吸灯效果停止")

def stop_animation():
    """停止动画"""
    global animation_running, animation_thread
    animation_running = False
    time.sleep(0.5)  # 等待动画线程停止
    if animation_thread:
        animation_thread.join(timeout=2)
        animation_thread = None
    logger.info("所有动画已停止")

# ============ Web 路由 ============

@app.route('/')
def index():
    """主页"""
    return render_template('mqtt_web_control.html')

@app.route('/api/serial/list', methods=['GET'])
def api_list_serial_ports():
    """获取可用的串口列表"""
    try:
        ports = list(serial.tools.list_ports.comports())
        port_list = []
        for port in ports:
            port_list.append({
                'device': port.device,
                'description': port.description,
                'hwid': port.hwid
            })
        return jsonify({
            'success': True,
            'ports': port_list,
            'current_port': ser.port if ser and ser.is_open else None
        })
    except Exception as e:
        logger.error(f"获取串口列表失败: {e}")
        return jsonify({
            'success': False,
            'message': str(e),
            'ports': []
        })

@app.route('/api/serial/connect', methods=['POST'])
def api_connect_serial():
    """连接到指定串口"""
    global ser
    data = request.json
    port = data.get('port', 'COM11')

    # 如果已连接其他串口，先断开
    if ser and ser.is_open:
        try:
            ser.close()
            logger.info(f"关闭串口: {ser.port}")
        except:
            pass

    # 连接新串口
    try:
        ser = serial.Serial(port, 115200, timeout=1)
        time.sleep(0.5)
        logger.info(f"串口已连接: {port}")
        return jsonify({
            'success': True,
            'message': f'已连接到 {port}',
            'port': port
        })
    except Exception as e:
        logger.error(f"连接串口失败: {e}")
        ser = None
        return jsonify({
            'success': False,
            'message': f'连接失败: {e}',
            'port': port
        })

@app.route('/api/serial/disconnect', methods=['POST'])
def api_disconnect_serial():
    """断开串口连接"""
    global ser
    if ser and ser.is_open:
        try:
            port_name = ser.port
            ser.close()
            ser = None
            logger.info(f"已断开串口: {port_name}")
            return jsonify({
                'success': True,
                'message': '已断开连接'
            })
        except Exception as e:
            return jsonify({
                'success': False,
                'message': str(e)
            })
    return jsonify({
        'success': False,
        'message': '串口未连接'
    })

@app.route('/api/status')
def api_status():
    """获取系统状态"""
    global ser, animation_mode
    return jsonify({
        'mqtt_connected': True,
        'serial_connected': ser and ser.is_open,
        'system_ready': True,
        'animation_mode': animation_mode
    })

@app.route('/api/led/set', methods=['POST'])
def api_set_led():
    """设置单个 LED"""
    global led_states, animation_running

    # 停止任何正在运行的动画
    if animation_running:
        stop_animation()
        time.sleep(0.3)  # 等待动画完全停止

    data = request.json
    index = data.get('index', 0)
    r = data.get('r', 0)
    g = data.get('g', 0)
    b = data.get('b', 0)

    cmd = f"SET,{index},{r},{g},{b}"
    success = send_serial_command(cmd)

    if success:
        led_states[index] = [r, g, b]

    return jsonify({
        'success': success,
        'message': f'LED {index} 设置为 RGB({r},{g},{b})'
    })

@app.route('/api/led/fill', methods=['POST'])
def api_fill():
    """全灯填充"""
    global led_states, animation_running

    # 停止任何正在运行的动画
    if animation_running:
        stop_animation()

    data = request.json
    r = data.get('r', 0)
    g = data.get('g', 0)
    b = data.get('b', 0)

    cmd = f"FILL,{r},{g},{b}"
    success = send_serial_command(cmd)

    if success:
        for i in range(8):
            led_states[i] = [r, g, b]

    return jsonify({
        'success': success,
        'message': f'全灯设置为 RGB({r},{g},{b})'
    })

@app.route('/api/led/clear', methods=['POST'])
def api_clear():
    """全灯关闭"""
    global led_states, animation_running

    # 停止任何正在运行的动画
    if animation_running:
        stop_animation()

    success = send_serial_command('CLEAR')

    if success:
        for i in range(8):
            led_states[i] = [0, 0, 0]

    return jsonify({
        'success': success,
        'message': '全灯已关闭'
    })

@app.route('/api/led/animation', methods=['POST'])
def api_animation():
    """播放动画"""
    global animation_running, animation_thread, animation_mode

    # 停止之前的动画
    if animation_running:
        stop_animation()

    data = request.json
    mode = data.get('mode', 'running')
    animation_mode = mode

    animation_running = True

    # 根据模式启动相应的动画线程
    if mode == 'running':
        direction = data.get('direction', 'forward')
        color_data = data.get('color', {'r': 255, 'g': 0, 'b': 0})
        # 确保颜色值是整数
        color = (int(color_data.get('r', 255)), int(color_data.get('g', 0)), int(color_data.get('b', 0)))
        speed = float(data.get('speed', 0.2))
        logger.info(f"流水灯参数 - 方向: {direction}, 颜色: RGB{color}, 速度: {speed}s")
        animation_thread = threading.Thread(target=animation_running_light, args=(direction, color, speed), daemon=True)
    elif mode == 'rainbow':
        animation_thread = threading.Thread(target=animation_rainbow, daemon=True)
    elif mode == 'breathing':
        color_data = data.get('color', {'r': 255, 'g': 0, 'b': 0})
        color = (color_data['r'], color_data['g'], color_data['b'])
        animation_thread = threading.Thread(target=animation_breathing, args=(color,), daemon=True)
    else:
        return jsonify({
            'success': False,
            'message': f'未知动画模式: {mode}'
        })

    animation_thread.start()

    return jsonify({
        'success': True,
        'message': f'启动 {mode} 动画'
    })

@app.route('/api/led/status', methods=['GET'])
def api_get_status():
    """获取 LED 状态"""
    global led_states
    return jsonify({
        'success': True,
        'leds': led_states
    })

if __name__ == '__main__':
    print("\n" + "="*60)
    print("LED 控制系统（简化版 - 支持动画效果）")
    print("="*60)
    print("\n[启动] Flask 后端服务...")
    print("访问地址: http://localhost:5000")
    print("\n💡 请在网页界面选择串口并连接")
    print("\nAPI 端点:")
    print("  GET  /api/serial/list       - 获取可用串口")
    print("  POST /api/serial/connect    - 连接串口")
    print("  POST /api/serial/disconnect - 断开串口")
    print("  POST /api/led/set           - 设置单个 LED")
    print("  POST /api/led/fill          - 全灯填充")
    print("  POST /api/led/clear         - 全灯关闭")
    print("  POST /api/led/animation     - 播放动画")
    print("  GET  /api/status            - 获取状态")
    print("\n按 Ctrl+C 停止程序")
    print("="*60 + "\n")

    try:
        app.run(debug=True, host='localhost', port=5000, use_reloader=False)
    except KeyboardInterrupt:
        print("\n\n程序已停止")
    finally:
        if animation_running:
            stop_animation()
        if ser:
            ser.close()

