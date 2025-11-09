@echo off
echo ================================================
echo   FORCE PUSH A GITHUB - SOBRESCRIBIENDO TODO
echo ================================================
echo.

cd /d c:\Users\Rubir\Downloads\mirai

echo [*] Verificando estado...
git status

echo.
echo [*] Haciendo commit...
git commit -m "Complete organized structure with disclaimers"

echo.
echo [*] Cambiando a rama main...
git branch -M main

echo.
echo [*] Force push a GitHub (sobrescribiendo archivos basura)...
git push -u origin main --force

echo.
echo ================================================
if %errorlevel% equ 0 (
    echo   EXITO! Revisa: https://github.com/Vixy-beep/higurama
) else (
    echo   ERROR! Revisa los mensajes arriba
)
echo ================================================
echo.
pause
