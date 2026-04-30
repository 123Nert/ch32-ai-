@echo off
REM 循迹模块Web可视化应用启动脚本

echo ========================================
echo 循迹模块Web可视化系统
echo ========================================
echo.

REM 检查Python是否安装
python --version >nul 2>&1
if errorlevel 1 (
    echo [错误] 未找到Python，请先安装Python
    pause
    exit /b 1
)

REM 检查虚拟环境
if not exist "venv" (
    echo [提示] 第一次运行，正在创建虚拟环境...
    python -m venv venv
)

REM 激活虚拟环境
call venv\Scripts\activate.bat

REM 检查依赖
echo [提示] 检查并安装依赖...
pip install -r requirements.txt -q

REM 启动应用
echo.
echo [信息] 应用启动中...
echo [信息] 访问地址: http://localhost:5000
echo [信息] 按 Ctrl+C 停止应用
echo.

python app.py

pause
