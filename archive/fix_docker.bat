@echo off
echo ========================================
echo   REPARAR CONTENEDORES DOCKER
echo ========================================
echo.

echo [1/3] Deteniendo contenedores antiguos...
docker-compose -f docker-vulnerable-ssh.yml down

echo.
echo [2/3] Limpiando contenedores...
docker rm -f vulnerable_ssh_1 vulnerable_ssh_2 vulnerable_ssh_3 2>nul

echo.
echo [3/3] Iniciando contenedores frescos...
docker-compose -f docker-vulnerable-ssh.yml up -d

echo.
echo [*] Esperando 20 segundos para que SSH inicie...
timeout /t 20 /nobreak >nul

echo.
echo ========================================
echo   VERIFICANDO ESTADO
echo ========================================
echo.

docker ps --format "table {{.Names}}\t{{.Status}}\t{{.Ports}}"

echo.
echo ========================================
echo   PROBANDO CONEXIONES
echo ========================================
echo.

python verify_docker.py

pause
