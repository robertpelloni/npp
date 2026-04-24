@echo off
setlocal
title NPP (CMake build)
cd /d "%~dp0"

echo [NPP (CMake build)] Starting...
where cmake >nul 2>nul
if errorlevel 1 (
    echo [NPP (CMake build)] cmake not found. Please install it.
    pause
    exit /b 1
)

cmake --build build --config Release

if errorlevel 1 (
    echo [NPP (CMake build)] Exited with error code %errorlevel%.
    pause
)
endlocal
