@echo off
echo ========================================
echo   PRUEBA DE BINARIO - LUCKY STAR
echo ========================================
echo.

if not exist dist\systemd.exe (
    echo [!] ERROR: dist\systemd.exe no existe
    echo [*] Ejecuta primero: COMPILE_NOW.bat
    pause
    exit /b 1
)

echo [*] Verificando binario...
dir dist\systemd.exe
echo.

echo [*] Verificando que C2 este corriendo...
netstat -an | findstr ":8443"
if %ERRORLEVEL% NEQ 0 (
    echo [!] ADVERTENCIA: C2 no esta corriendo en puerto 8443
    echo [*] Inicia C2 primero: python c2_commander.py
    echo.
    set /p continue="¿Continuar de todas formas? (s/n): "
    if /i not "%continue%"=="s" exit /b 1
)

echo.
echo [*] Ejecutando binario con output visible...
echo [*] Presiona Ctrl+C para detener
echo.
echo ========================================
echo.

REM Ejecutar el binario y mostrar output
dist\systemd.exe

echo.
echo ========================================
echo [*] Binario terminado
pause
