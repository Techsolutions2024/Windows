@echo off
REM Canon Camera Control - Launcher

echo Canon Camera Control
echo =====================
echo.

REM Check if Python is installed
python --version >nul 2>&1
if errorlevel 1 (
    echo Python is not installed or not in PATH!
    echo Please install Python 3.8 or higher from https://python.org
    pause
    exit /b 1
)

REM Check if PyQt6 is installed
python -c "import PyQt6" >nul 2>&1
if errorlevel 1 (
    echo Installing required packages...
    pip install -r requirements.txt
    if errorlevel 1 (
        echo Failed to install packages!
        pause
        exit /b 1
    )
)

REM Run the application
echo Starting application...
python main.py

pause
