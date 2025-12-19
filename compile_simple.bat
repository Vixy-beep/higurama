@echo off
REM compile_simple.bat - Compile Higurashi using Docker or online compiler
echo ====================================
echo   Higurama Compilation Helper
echo ====================================
echo.

echo Opciones de compilacion:
echo.
echo 1. Usar Docker (Recomendado)
echo 2. Subir a VPS y compilar ahi
echo 3. Instalar WSL Ubuntu completo
echo.

echo ====================================
echo OPCION 1: Docker Compilation
echo ====================================
echo.
echo docker run --rm -v "%cd%":/src -w /src gcc:latest gcc -Wall -O2 -o higurama c2_master.c -lssl -lcrypto -ljson-c -lpthread -lcurl
echo docker run --rm -v "%cd%":/src -w /src gcc:latest gcc -Wall -O2 -o higurashi higurashi.c -lssl -lcrypto -ljson-c -lpthread -lcurl -lssh
echo docker run --rm -v "%cd%":/src -w /src gcc:latest gcc -Wall -O2 -o test_animations test_animations.c -lpthread
echo.
echo Ejecuta manualmente estos comandos si tienes Docker Desktop corriendo
echo.

echo ====================================
echo OPCION 2: VPS Compilation
echo ====================================
echo.
echo 1. Subir archivos al VPS:
echo    scp *.h *.c root@93.95.231.134:/opt/higurama/
echo.
echo 2. Conectar via SSH:
echo    ssh root@93.95.231.134
echo.
echo 3. Compilar en VPS:
echo    cd /opt/higurama
echo    gcc -Wall -O2 -o higurama c2_master.c -lssl -lcrypto -ljson-c -lpthread
echo    gcc -Wall -O2 -o higurashi higurashi.c -lssl -lcrypto -ljson-c -lpthread -lcurl -lssh
echo.

echo ====================================
echo OPCION 3: Instalar WSL Ubuntu
echo ====================================
echo.
echo 1. Instalar Ubuntu en WSL:
echo    wsl --install -d Ubuntu
echo.
echo 2. Una vez instalado, ejecutar:
echo    wsl
echo    sudo apt update
echo    sudo apt install build-essential libssl-dev libssh-dev libjson-c-dev libcurl4-openssl-dev
echo    gcc -Wall -O2 -o higurama c2_master.c -lssl -lcrypto -ljson-c -lpthread
echo.

pause
