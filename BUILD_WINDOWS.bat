@echo off
setlocal
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto fail
cmake --build build --config Release
if errorlevel 1 goto fail
echo.
echo Build finished. Check build\IRON_MODULAR_artefacts\Release\VST3
pause
exit /b 0
:fail
echo Build failed. Check that Visual Studio 2022 C++ tools, CMake and Git are installed.
pause
exit /b 1
