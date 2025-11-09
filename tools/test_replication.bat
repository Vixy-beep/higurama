@echo off
echo ========================================
echo   PRUEBA DE REPLICACION - LUCKY STAR
echo ========================================
echo.

echo [*] Verificando Docker...
docker --version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [!] Docker no esta instalado
    echo [*] Descargalo de: https://www.docker.com/products/docker-desktop
    pause
    exit /b 1
)

echo [*] Iniciando contenedores vulnerables...
docker-compose -f docker-vulnerable-ssh.yml up -d

echo.
echo [+] Contenedores iniciados:
docker ps --format "table {{.Names}}\t{{.Status}}\t{{.Ports}}"

echo.
echo [*] Esperando a que SSH inicie...
timeout /t 10 /nobreak >nul

echo.
echo ========================================
echo   INFORMACION DE OBJETIVOS
echo ========================================
echo.
echo Host 1: localhost:2201
echo   Usuario: admin
echo   Password: admin
echo.
echo Host 2: localhost:2202
echo   Usuario: test
echo   Password: test
echo.
echo Host 3: localhost:2121 (FTP)
echo   Usuario: admin
echo   Password: admin
echo.
echo ========================================
echo   PROBAR MANUALMENTE
echo ========================================
echo.
echo 1. Ejecuta: python c2_commander.py
echo 2. En otra terminal: python bot_soldier.py
echo 3. En C2: recon 1
echo 4. En C2: list 1
echo 5. En C2: exploit 1 1
echo.
echo Para detener: docker-compose -f docker-vulnerable-ssh.yml down
echo.
pause
