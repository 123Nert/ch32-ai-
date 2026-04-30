@echo off
chcp 65001 >nul
echo.
echo ============================================================
echo   WS2812 LED 控制系统
echo ============================================================
echo.
echo 检查依赖...
python -c "import flask, flask_cors, serial" 2>nul
if errorlevel 1 (
    echo 缺少依赖，正在安装...
    pip install -r requirements.txt
)
echo.
echo 启动后端服务...
echo 访问地址: http://localhost:5000
echo 按 Ctrl+C 停止程序
echo.
echo ============================================================
echo.
python run_all_no_mqtt.py
pause
