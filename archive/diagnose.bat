@echo off
cls
echo ========================================
echo   DIAGNOSTICO COMPLETO DEL SISTEMA
echo ========================================
echo.

REM Verificar binario
echo [1] Verificando binario compilado...
if exist "bots\linux\x86_64\systemd" (
    echo     [OK] Binario existe
    dir "bots\linux\x86_64\systemd" | findstr /C:"systemd"
) else (
    echo     [FALTA] Binario NO compilado
    echo     ^> Ejecuta: COMPILE_NOW.bat
)
echo.

REM Verificar archivos clave
echo [2] Verificando archivos clave...
if exist "bot_soldier.py" (echo     [OK] bot_soldier.py) else (echo     [FALTA] bot_soldier.py)
if exist "c2_commander.py" (echo     [OK] c2_commander.py) else (echo     [FALTA] c2_commander.py)
if exist "cert.pem" (echo     [OK] cert.pem) else (echo     [FALTA] cert.pem - Ejecuta generate_certs.py)
if exist "key.pem" (echo     [OK] key.pem) else (echo     [FALTA] key.pem - Ejecuta generate_certs.py)
echo.

REM Verificar Docker
echo [3] Verificando contenedores Docker...
docker ps --format "{{.Names}} - Puertos: {{.Ports}}" 2>nul
if %errorlevel% neq 0 (
    echo     [FALTA] Docker no esta corriendo o no hay contenedores
    echo     ^> Ejecuta: start_docker_lab.bat
)
echo.

REM Verificar puertos
echo [4] Verificando puertos...
netstat -ano | findstr /C:":8000 " | findstr /C:"LISTENING" >nul
if %errorlevel% equ 0 (
    echo     [OK] Servidor HTTP en 8000
) else (
    echo     [FALTA] Servidor HTTP en 8000
    echo     ^> Ejecuta: start_binary_server.bat
)

netstat -ano | findstr /C:":8443 " | findstr /C:"LISTENING" >nul
if %errorlevel% equ 0 (
    echo     [OK] C2 Server en 8443
) else (
    echo     [FALTA] C2 Server en 8443
    echo     ^> Ejecuta: python c2_commander.py
)

netstat -ano | findstr /C:":2201 " | findstr /C:"LISTENING" >nul
if %errorlevel% equ 0 (
    echo     [OK] Docker SSH en 2201
) else (
    echo     [FALTA] Docker SSH en 2201
)
echo.

REM Test de conectividad
echo [5] Test de conectividad...
python -c "import socket; sock = socket.socket(); sock.settimeout(1); result = sock.connect_ex(('127.0.0.1', 2201)); sock.close(); exit(0 if result == 0 else 1)" 2>nul
if %errorlevel% equ 0 (
    echo     [OK] Contenedor SSH 2201 accesible
) else (
    echo     [FALTA] No se puede conectar a SSH 2201
)

python -c "import socket; sock = socket.socket(); sock.settimeout(1); result = sock.connect_ex(('127.0.0.1', 8000)); sock.close(); exit(0 if result == 0 else 1)" 2>nul
if %errorlevel% equ 0 (
    echo     [OK] Servidor HTTP 8000 accesible
) else (
    echo     [FALTA] No se puede conectar a HTTP 8000
)
echo.

echo ========================================
echo   RESUMEN
echo ========================================
echo.
echo Para un sistema funcionando necesitas:
echo   1. Binario compilado (COMPILE_NOW.bat)
echo   2. Contenedores Docker (start_docker_lab.bat)
echo   3. Servidor HTTP (start_binary_server.bat)
echo   4. C2 Commander (python c2_commander.py)
echo   5. Bot test (test_binary.bat)
echo.
pause
