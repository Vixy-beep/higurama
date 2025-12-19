#!/bin/bash
# Script de Testing Seguro - Laboratorio Controlado
# Para demostración académica sin riesgo

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  LABORATORIO SEGURO - Testing Arquitectura Distribuida      ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
CYAN='\033[0;36m'
NC='\033[0m'

# Verificar dependencias
echo -e "${CYAN}[*]${NC} Verificando dependencias..."

if ! command -v docker &> /dev/null; then
    echo -e "${RED}[!]${NC} Docker no instalado. Instalar con:"
    echo "    sudo apt install docker.io docker-compose"
    exit 1
fi

echo -e "${GREEN}✓${NC} Docker disponible"
echo ""

# Crear docker-compose para lab
cat > docker-compose-lab.yml << 'EOF'
version: '3.8'

services:
  # Dispositivos IoT vulnerables (simulados)
  iot_camera_1:
    image: alpine:latest
    hostname: camera-01
    networks:
      labnet:
        ipv4_address: 172.30.0.10
    command: >
      sh -c "
      echo 'Setting up vulnerable Telnet device...' &&
      apk add --no-cache busybox-extras openssh &&
      adduser -D -h /home/root root &&
      echo 'root:xc3511' | chpasswd &&
      echo 'PermitRootLogin yes' >> /etc/ssh/sshd_config &&
      telnetd -F -p 23 -l /bin/sh &
      /usr/sbin/sshd -D
      "
  
  iot_router_1:
    image: alpine:latest
    hostname: router-01
    networks:
      labnet:
        ipv4_address: 172.30.0.11
    command: >
      sh -c "
      echo 'Setting up vulnerable router...' &&
      apk add --no-cache busybox-extras &&
      adduser -D -h /home/admin admin &&
      echo 'admin:admin' | chpasswd &&
      telnetd -F -p 2323 -l /bin/sh
      "
  
  iot_device_2:
    image: alpine:latest
    hostname: device-02
    networks:
      labnet:
        ipv4_address: 172.30.0.12
    command: >
      sh -c "
      echo 'Setting up IoT device...' &&
      apk add --no-cache busybox-extras &&
      adduser -D -h /home/root root &&
      echo 'root:admin' | chpasswd &&
      telnetd -F -p 23 -l /bin/sh
      "
  
  iot_device_3:
    image: alpine:latest
    hostname: device-03
    networks:
      labnet:
        ipv4_address: 172.30.0.13
    command: >
      sh -c "
      echo 'Setting up IoT device...' &&
      apk add --no-cache busybox-extras &&
      adduser -D -h /home/root root &&
      echo 'root:888888' | chpasswd &&
      telnetd -F -p 23 -l /bin/sh
      "

  # Honeypot (para ver intentos de ataque)
  honeypot:
    image: alpine:latest
    hostname: honeypot
    networks:
      labnet:
        ipv4_address: 172.30.0.20
    command: >
      sh -c "
      echo 'Starting honeypot...' &&
      apk add --no-cache tcpdump busybox-extras &&
      telnetd -F -p 23 -l /bin/sh &
      echo 'root:root' | chpasswd &&
      tcpdump -i eth0 -w /tmp/capture.pcap &
      tail -f /dev/null
      "

  # Test bot (para infectar manualmente)
  test_bot:
    image: alpine:latest
    hostname: test-bot
    networks:
      labnet:
        ipv4_address: 172.30.0.100
    volumes:
      - ./higurashi_immortal:/tmp/higurashi_immortal:ro
    command: >
      sh -c "
      echo 'Test bot ready. To infect manually:' &&
      echo 'docker exec -it test_bot sh' &&
      echo 'cd /tmp && ./higurashi_immortal --no-daemon' &&
      tail -f /dev/null
      "

networks:
  labnet:
    driver: bridge
    ipam:
      config:
        - subnet: 172.30.0.0/24

EOF

echo -e "${GREEN}✓${NC} docker-compose-lab.yml creado"
echo ""

# Opciones de testing
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo -e "${YELLOW}Opciones de Testing:${NC}"
echo ""
echo "  [1] Iniciar laboratorio completo"
echo "  [2] Ver dispositivos vulnerables"
echo "  [3] Infectar bot manualmente"
echo "  [4] Ver logs de honeypot"
echo "  [5] Detener laboratorio"
echo "  [0] Salir"
echo ""
echo -n "Selecciona opción: "
read option

case $option in
    1)
        echo ""
        echo -e "${CYAN}[*]${NC} Iniciando laboratorio..."
        docker-compose -f docker-compose-lab.yml up -d
        
        echo ""
        echo -e "${GREEN}✓${NC} Laboratorio iniciado"
        echo ""
        echo "┌─────────────────────────────────────────────────────────────┐"
        echo "│ DISPOSITIVOS VULNERABLES                                    │"
        echo "├─────────────────────────────────────────────────────────────┤"
        echo "│ 172.30.0.10:23   - IoT Camera   [root:xc3511]              │"
        echo "│ 172.30.0.11:2323 - Router       [admin:admin]              │"
        echo "│ 172.30.0.12:23   - IoT Device   [root:admin]               │"
        echo "│ 172.30.0.13:23   - IoT Device   [root:888888]              │"
        echo "│ 172.30.0.20:23   - Honeypot     [root:root]                │"
        echo "└─────────────────────────────────────────────────────────────┘"
        echo ""
        echo "Para infectar bot:"
        echo "  docker exec -it test_bot sh"
        echo "  cd /tmp && ./higurashi_immortal --no-daemon"
        echo ""
        echo "El bot escaneará 172.30.0.0/24 y encontrará los dispositivos"
        ;;
    
    2)
        echo ""
        docker-compose -f docker-compose-lab.yml ps
        echo ""
        echo "Para conectar a un dispositivo:"
        echo "  telnet 172.30.0.10 23"
        echo "  (user: root, pass: xc3511)"
        ;;
    
    3)
        echo ""
        echo -e "${CYAN}[*]${NC} Infectando bot de prueba..."
        echo ""
        
        # Verificar que higurashi_immortal existe
        if [ ! -f "higurashi_immortal" ]; then
            echo -e "${RED}[!]${NC} higurashi_immortal no encontrado"
            echo "Compilar primero con: ./compile_distributed.sh"
            exit 1
        fi
        
        # Copiar binary al container
        docker cp higurashi_immortal $(docker ps -qf "name=test_bot"):/tmp/
        
        echo -e "${GREEN}✓${NC} Binary copiado"
        echo ""
        echo "Ejecutar en otra terminal:"
        echo ""
        echo "  docker exec -it $(docker ps -qf "name=test_bot") sh"
        echo "  cd /tmp"
        echo "  chmod +x higurashi_immortal"
        echo "  ./higurashi_immortal --no-daemon"
        echo ""
        echo "Observarás:"
        echo "  [*] Scanning 172.30.0.0/24..."
        echo "  [+] OPEN: 172.30.0.10:23"
        echo "  [+] CREDS: 172.30.0.10:23 [root:xc3511]"
        echo "  [+] INFECTED: 172.30.0.10:23"
        ;;
    
    4)
        echo ""
        echo -e "${CYAN}[*]${NC} Logs del honeypot:"
        echo ""
        docker logs $(docker ps -qf "name=honeypot") 2>&1 | tail -20
        echo ""
        echo "Para ver captura de paquetes:"
        echo "  docker exec $(docker ps -qf "name=honeypot") tcpdump -r /tmp/capture.pcap"
        ;;
    
    5)
        echo ""
        echo -e "${CYAN}[*]${NC} Deteniendo laboratorio..."
        docker-compose -f docker-compose-lab.yml down
        echo -e "${GREEN}✓${NC} Laboratorio detenido"
        ;;
    
    0)
        echo "Saliendo..."
        exit 0
        ;;
    
    *)
        echo -e "${RED}[!]${NC} Opción inválida"
        exit 1
        ;;
esac

echo ""
