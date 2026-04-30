#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
WS2812 LED 控制系统 - 快速启动脚本
"""

import subprocess
import sys
import os

def check_dependencies():
    """检查依赖"""
    print("检查 Python 依赖...")
    required = ['flask', 'flask_cors', 'serial']
    missing = []

    for module in required:
        try:
            __import__(module)
            print(f"  ✓ {module}")
        except ImportError:
            missing.append(module)
            print(f"  ✗ {module}")

    if missing:
        print(f"\n缺少依赖，安装: pip install -r requirements.txt")
        return False
    return True

def main():
    print("\n" + "="*60)
    print("  WS2812 LED 控制系统")
    print("="*60)

    if not check_dependencies():
        sys.exit(1)

    print("\n启动后端服务...")
    print("访问地址: http://localhost:5000\n")
    print("按 Ctrl+C 停止程序\n")
    print("="*60 + "\n")

    try:
        subprocess.run([sys.executable, 'run_all_no_mqtt.py'])
    except KeyboardInterrupt:
        print("\n\n程序已停止")

if __name__ == '__main__':
    main()
