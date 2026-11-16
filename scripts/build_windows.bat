@echo off
setlocal
cd /d "%~dp0\.."
where cmake >nul 2>nul || (echo ERROR: CMake is not installed or not on PATH.& pause& exit /b 1)
cmake -S . -B build -A x64
if errorlevel 1 (echo CMake configure failed.& pause& exit /b 1)
cmake --build build --config Release
if errorlevel 1 (echo Build failed.& pause& exit /b 1)
echo.
echo Build complete. Run scripts\start_windows.bat
pause
