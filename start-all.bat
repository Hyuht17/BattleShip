@echo off
REM BattleShip Game - Start All Servers Script (Windows)
REM This script starts all three servers in separate windows

echo ╔═══════════════════════════════════════╗
echo ║  Starting BattleShip Game Servers    ║
echo ╚═══════════════════════════════════════╝
echo.

REM Get script directory
set SCRIPT_DIR=%~dp0

REM Check if C++ server is compiled
if not exist "%SCRIPT_DIR%server\server_full.exe" (
    echo Compiling C++ server...
    cd /d "%SCRIPT_DIR%server"
    g++ -o server_full.exe server_full.cpp -lpthread -lws2_32
    if errorlevel 1 (
        echo Failed to compile C++ server!
        pause
        exit /b 1
    )
)

REM Check if dependencies are installed
if not exist "%SCRIPT_DIR%node-server\node_modules" (
    echo Installing node-server dependencies...
    cd /d "%SCRIPT_DIR%node-server"
    call npm install
)

if not exist "%SCRIPT_DIR%frontend\node_modules" (
    echo Installing frontend dependencies...
    cd /d "%SCRIPT_DIR%frontend"
    call npm install
)

echo Starting servers...
echo.

REM Start C++ server
start "C++ Server" cmd /k "cd /d %SCRIPT_DIR%server && server_full.exe"
timeout /t 2 /nobreak > nul

REM Start Node.js server
start "Node.js Server" cmd /k "cd /d %SCRIPT_DIR%node-server && npm start"
timeout /t 2 /nobreak > nul

REM Start React frontend
start "React Frontend" cmd /k "cd /d %SCRIPT_DIR%frontend && npm run dev"

echo.
echo ╔═══════════════════════════════════════╗
echo ║     All Servers Started!              ║
echo ╚═══════════════════════════════════════╝
echo.
echo C++ Server:      localhost:8080
echo Node.js Server:  localhost:3000
echo Frontend:        http://localhost:5173
echo.
echo Close the terminal windows to stop the servers
echo.
pause
