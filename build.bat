@echo off
REM Build script for wall_robot project
REM This script uses PlatformIO CLI to build the project

cd /d "%~dp0"

echo.
echo ============================================
echo Wall Robot - PlatformIO Build Script
echo ============================================
echo.
echo Building STM32F401RC project...
echo.

REM Try to find and use pio command
where pio >nul 2>nul
if %errorlevel% equ 0 (
    echo Found PlatformIO CLI, building...
    pio run
) else (
    echo.
    echo ERROR: PlatformIO CLI not found in PATH
    echo.
    echo SOLUTION: Use VS Code instead:
    echo   1. Open VS Code
    echo   2. Click PlatformIO icon (left sidebar)
    echo   3. Project Tasks ^> Build
    echo.
    echo Or install PlatformIO CLI:
    echo   python -m pip install platformio
    echo.
    pause
    exit /b 1
)

if %errorlevel% equ 0 (
    echo.
    echo ============================================
    echo Build SUCCESS! ^✓
    echo ============================================
    echo.
) else (
    echo.
    echo ============================================
    echo Build FAILED! ^✗
    echo ============================================
    echo.
    pause
    exit /b 1
)
