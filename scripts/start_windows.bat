@echo off
setlocal
cd /d "%~dp0\.."
if not exist "build\Release\aegis_soc.exe" if not exist "build\aegis_soc.exe" (
  echo First launch: configuring and building Aegis SOC...
  where cmake >nul 2>nul || (echo ERROR: Install CMake and Visual Studio 2022 C++ Build Tools first.& pause& exit /b 1)
  cmake -S . -B build -A x64
  if errorlevel 1 (echo Configure failed.& pause& exit /b 1)
  cmake --build build --config Release
  if errorlevel 1 (echo Build failed.& pause& exit /b 1)
)
if exist "build\Release\aegis_soc.exe" ("build\Release\aegis_soc.exe" %*) else if exist "build\aegis_soc.exe" ("build\aegis_soc.exe" %*) else (echo Executable not found after build.& pause& exit /b 1)
pause
