#!/bin/bash

# 循迹模块Web可视化应用启动脚本

echo "========================================"
echo "循迹模块Web可视化系统"
echo "========================================"
echo ""

# 检查Python是否安装
if ! command -v python3 &> /dev/null; then
    echo "[错误] 未找到Python 3，请先安装Python"
    exit 1
fi

# 检查虚拟环境
if [ ! -d "venv" ]; then
    echo "[提示] 第一次运行，正在创建虚拟环境..."
    python3 -m venv venv
fi

# 激活虚拟环境
source venv/bin/activate

# 检查依赖
echo "[提示] 检查并安装依赖..."
pip install -r requirements.txt -q

# 启动应用
echo ""
echo "[信息] 应用启动中..."
echo "[信息] 访问地址: http://localhost:5000"
echo "[信息] 按 Ctrl+C 停止应用"
echo ""

python3 app.py
