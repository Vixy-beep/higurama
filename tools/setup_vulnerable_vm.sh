#!/bin/bash
# Script para configurar una VM Ubuntu como objetivo vulnerable

echo "========================================="
echo "  CONFIGURACIÓN DE VM VULNERABLE"
echo "========================================="
echo ""

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Verificar si es root
if [ "$EUID" -ne 0 ]; then 
    echo -e "${RED}[!] Por favor ejecuta como root (sudo)${NC}"
    exit 1
fi

echo -e "${GREEN}[1/5] Instalando OpenSSH Server...${NC}"
apt update -qq
apt install -y openssh-server python3 python3-pip

echo -e "${GREEN}[2/5] Configurando SSH para ser vulnerable...${NC}"
# Permitir autenticación por contraseña
sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/' /etc/ssh/sshd_config
sed -i 's/PasswordAuthentication no/PasswordAuthentication yes/' /etc/ssh/sshd_config
# Permitir root login (SOLO PARA PRUEBAS)
sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config

echo -e "${GREEN}[3/5] Creando usuarios vulnerables...${NC}"
# Crear usuarios con contraseñas débiles
users=("admin:admin" "test:test" "user:password" "guest:guest")
for user_pass in "${users[@]}"; do
    IFS=':' read -r username password <<< "$user_pass"
    
    if id "$username" &>/dev/null; then
        echo -e "${YELLOW}[*] Usuario $username ya existe${NC}"
    else
        useradd -m -s /bin/bash "$username"
        echo "$username:$password" | chpasswd
        echo -e "${GREEN}[+] Usuario creado: $username:$password${NC}"
    fi
done

echo -e "${GREEN}[4/5] Instalando Python y dependencias...${NC}"
apt install -y python3-pip
pip3 install paramiko cryptography

echo -e "${GREEN}[5/5] Reiniciando SSH...${NC}"
systemctl restart ssh
systemctl enable ssh

echo ""
echo -e "${GREEN}=========================================${NC}"
echo -e "${GREEN}  ✓ CONFIGURACIÓN COMPLETADA${NC}"
echo -e "${GREEN}=========================================${NC}"
echo ""
echo -e "${YELLOW}Usuarios vulnerables creados:${NC}"
for user_pass in "${users[@]}"; do
    echo "  - $user_pass"
done
echo ""
echo -e "${YELLOW}IP de esta máquina:${NC}"
ip addr show | grep "inet " | grep -v 127.0.0.1 | awk '{print "  - " $2}'
echo ""
echo -e "${RED}⚠️  ADVERTENCIA: Esta VM es INTENCIONALMENTE insegura${NC}"
echo -e "${RED}    Solo usar en redes aisladas/laboratorio${NC}"
echo ""
