@echo off
setlocal
cd /d "%~dp0"
where g++ >nul 2>nul
if errorlevel 1 (
    echo [ERROR] g++ not found in PATH. Please install MinGW-w64 first.
    pause
    exit /b 1
)
echo [1/2] Building console version: EndfieldGacha.exe ...
g++ -O2 -std=gnu++17 -finput-charset=UTF-8 -o EndfieldGacha.exe EndfieldGacha.cpp -static -lshell32
if errorlevel 1 (
    echo [ERROR] Console build failed. See the messages above.
    pause
    exit /b 1
)
echo [2/2] Building GUI version: EndfieldGachaGUI.exe ...
g++ -O2 -std=gnu++17 -finput-charset=UTF-8 -mwindows -o EndfieldGachaGUI.exe EndfieldGachaGUI.cpp -static -lgdiplus -lshell32 -lgdi32 -luser32
if errorlevel 1 (
    echo [ERROR] GUI build failed. See the messages above.
    pause
    exit /b 1
)
echo.
echo Build finished:
echo   EndfieldGacha.exe      console version
echo   EndfieldGachaGUI.exe   GUI version
pause
exit /b 0
