@echo off
echo ========================================
echo    INSTALANDO SHODAN HUNTER
echo ========================================
echo.

echo [1/2] Instalando modulo Shodan...
pip install shodan -q

echo.
echo [2/2] Instalando dependencias adicionales...
pip install censys zoomeye PySocks -q

echo.
echo ========================================
echo    INSTALACION COMPLETA
echo ========================================
echo.
echo Para obtener API KEY GRATIS:
echo 1. Ve a: https://account.shodan.io/register
echo 2. Registrate con email
echo 3. Tu API key estara en: https://account.shodan.io/
echo.
echo Alternativas gratuitas:
echo - Censys: https://censys.io/register
echo - ZoomEye: https://www.zoomeye.org/
echo.
pause
