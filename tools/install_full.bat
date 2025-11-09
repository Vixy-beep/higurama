@echo off
echo ========================================
echo   INSTALACION COMPLETA - LUCKY STAR
echo ========================================
echo.

echo [*] Instalando dependencias basicas...
python -m pip install --upgrade pip
python -m pip install paramiko netaddr cryptography

echo.
echo [*] Instalando Scapy (requiere Npcap en Windows)...
python -m pip install scapy

echo.
echo [!] IMPORTANTE EN WINDOWS:
echo     Scapy require Npcap para funcionar
echo     Descarga: https://npcap.com/#download
echo.
pause

python generate_certs.py

echo.
echo [+] Instalacion completa!
pause
