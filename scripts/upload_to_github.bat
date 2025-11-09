@echo off
REM Script para subir Mirai-Simulator a GitHub (Windows)

echo.
echo ════════════════════════════════════════════════════════════
echo   SUBIENDO MIRAI-SIMULATOR A GITHUB
echo ════════════════════════════════════════════════════════════
echo.

REM Verificar si git está instalado
where git >nul 2>nul
if %errorlevel% neq 0 (
    echo [X] Git no esta instalado. Descargalo desde:
    echo     https://git-scm.com/download/win
    pause
    exit /b 1
)

REM Solicitar información
set /p GITHUB_USER="Tu usuario de GitHub: "
set /p REPO_NAME="Nombre del repositorio [Mirai-Simulator]: "
if "%REPO_NAME%"=="" set REPO_NAME=Mirai-Simulator

echo.
echo [*] Inicializando repositorio Git...
git init

REM Configurar usuario si no existe
for /f "delims=" %%i in ('git config user.name') do set GIT_NAME=%%i
if "%GIT_NAME%"=="" (
    set /p GIT_NAME="Tu nombre para commits: "
    git config user.name "%GIT_NAME%"
)

for /f "delims=" %%i in ('git config user.email') do set GIT_EMAIL=%%i
if "%GIT_EMAIL%"=="" (
    set /p GIT_EMAIL="Tu email para commits: "
    git config user.email "%GIT_EMAIL%"
)

echo.
echo [*] Agregando archivos...
git add .

echo [*] Creando commit inicial...
git commit -m "Initial commit - Educational botnet simulator"

echo [*] Creando rama main...
git branch -M main

echo [*] Conectando con GitHub...
git remote add origin https://github.com/%GITHUB_USER%/%REPO_NAME%.git

echo.
echo ════════════════════════════════════════════════════════════
echo  LISTO PARA SUBIR
echo ════════════════════════════════════════════════════════════
echo.
echo ANTES DE CONTINUAR:
echo.
echo 1. Crea el repositorio en GitHub:
echo    https://github.com/new
echo.
echo 2. Nombre del repo: %REPO_NAME%
echo 3. Visibilidad: Public o Private
echo 4. NO inicialices con README (ya lo tenemos)
echo.
set /p CONFIRM="Ya creaste el repositorio? (s/n): "

if /i not "%CONFIRM%"=="s" (
    echo.
    echo [X] Proceso cancelado. Crea el repositorio primero.
    pause
    exit /b 0
)

echo.
echo [*] Subiendo archivos a GitHub...
git push -u origin main

if %errorlevel% equ 0 (
    echo.
    echo ════════════════════════════════════════════════════════════
    echo  EXITO! Repositorio subido correctamente
    echo ════════════════════════════════════════════════════════════
    echo.
    echo Tu repositorio esta en:
    echo https://github.com/%GITHUB_USER%/%REPO_NAME%
    echo.
    echo Proximos pasos:
    echo 1. Agregar topics en GitHub Settings
    echo 2. Escribir descripcion corta
    echo 3. Compartir con la comunidad
    echo.
) else (
    echo.
    echo [X] Error al subir. Verifica:
    echo    - Repositorio existe en GitHub
    echo    - Credenciales correctas
    echo    - Permisos de escritura
    echo.
    echo Reintenta: git push -u origin main
    echo.
)

pause
