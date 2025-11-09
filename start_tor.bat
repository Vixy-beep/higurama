@echo off
title Tor SOCKS5 Proxy - Higurama
color 0A
echo.
echo ========================================
echo    TOR SOCKS5 PROXY - HIGURAMA
echo ========================================
echo.
echo [*] Iniciando Tor en 127.0.0.1:9050...
echo [*] Presiona Ctrl+C para detener
echo.

cd /d "%~dp0"
tor\tor.exe -f tor\torrc

pause
