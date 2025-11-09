@echo off
echo ========================================
echo    TOR HIDDEN SERVICE SETUP
echo ========================================
echo.
echo [1/4] Descargando Tor Expert Bundle...
powershell -Command "Invoke-WebRequest -Uri 'https://dist.torproject.org/torbrowser/14.0.3/tor-expert-bundle-windows-x86_64-14.0.3.tar.gz' -OutFile 'tor-bundle.tar.gz'"

echo.
echo [2/4] Extrayendo archivos...
powershell -Command "tar -xf tor-bundle.tar.gz"

echo.
echo [3/4] Configurando Hidden Service...
mkdir tor\hidden_service 2>nul

(
echo # Configuracion Tor Hidden Service
echo SocksPort 9050
echo HiddenServiceDir %CD%\tor\hidden_service
echo HiddenServicePort 8443 127.0.0.1:8443
) > tor\torrc

echo.
echo [4/4] Iniciando Tor...
start /B tor\tor.exe -f tor\torrc

timeout /t 5 /nobreak >nul

echo.
echo ========================================
echo    TOR HIDDEN SERVICE ACTIVO
echo ========================================
type tor\hidden_service\hostname
echo.
echo Este es tu dominio .onion
echo Copia esta direccion para configurar los bots
echo ========================================
pause
