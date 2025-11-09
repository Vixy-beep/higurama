@echo off
cd /d "%~dp0"

echo ========================================
echo   LUCKY STAR - LABORATORIO DOCKER
echo ========================================
echo.

echo [1/4] Verificando Docker...
docker --version >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [!] Docker no esta instalado
    echo [*] Descarga: https://www.docker.com/products/docker-desktop
    pause
    exit /b 1
)

echo [2/4] Deteniendo contenedores antiguos...
docker-compose -f docker-vulnerable-ssh.yml down 2>nul

echo [3/4] Iniciando contenedores vulnerables...
docker-compose -f docker-vulnerable-ssh.yml up -d

if %ERRORLEVEL% NEQ 0 (
    echo [!] Error iniciando contenedores
    pause
    exit /b 1
)

echo [4/4] Esperando que SSH inicie...
timeout /t 15 /nobreak >nul

echo.
echo ========================================
echo   CONTENEDORES LISTOS
echo ========================================
echo.
docker ps --format "table {{.Names}}\t{{.Status}}\t{{.Ports}}"

echo.
echo ========================================
echo   OBJETIVOS VULNERABLES
echo ========================================
echo.
echo [1] localhost:2201 - admin:admin
echo [2] localhost:2202 - test:test
echo [3] localhost:2203 - root:root
echo.

echo ========================================
echo   PROBAR MANUALMENTE
echo ========================================
echo.
echo ssh admin@localhost -p 2201
echo Password: admin
echo.

echo ========================================
echo   EJECUTAR BOTNET
echo ========================================
echo.
echo Terminal 1: python c2_commander.py
echo Terminal 2: python bot_soldier.py
echo.
echo En C2:
echo   recon 1
echo   list 1
echo   auto_rep 1 on
echo   exploit 1 1
echo.

pause
