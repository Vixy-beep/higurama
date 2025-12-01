#!/bin/bash
# Compilar higurashi_mobile en VPS y servirlo para UserLAnd

echo "╔══════════════════════════════════════════════════════════╗"
echo "║  HIGURASHI MOBILE - Compilación para Android/UserLAnd   ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""

cd /opt/higurama || { echo "Error: directorio /opt/higurama no existe"; exit 1; }

# Actualizar repo
echo "[*] Actualizando repositorio..."
git pull origin main

# Compilar versión estática (compatible con UserLAnd/Android)
echo "[*] Compilando higurashi_mobile (estático)..."
gcc -static -o higurashi_mobile higurashi_mobile.c -lpthread -O2

if [ $? -ne 0 ]; then
    echo "[-] Error en compilación. Intentando con musl-gcc..."
    musl-gcc -static -o higurashi_mobile higurashi_mobile.c -lpthread -O2
fi

if [ ! -f higurashi_mobile ]; then
    echo "[-] Error: No se pudo compilar higurashi_mobile"
    exit 1
fi

# Strip para reducir tamaño
strip higurashi_mobile 2>/dev/null

SIZE=$(ls -lh higurashi_mobile | awk '{print $5}')
echo "[✓] Compilación exitosa! Tamaño: $SIZE"

# Verificar arquitectura
echo ""
echo "[*] Información del binario:"
file higurashi_mobile

# Mover a directorio web
echo ""
echo "[*] Preparando para descarga..."
cp higurashi_mobile /var/www/html/ 2>/dev/null || cp higurashi_mobile .

# Iniciar servidor HTTP simple en puerto 8080
echo ""
echo "╔══════════════════════════════════════════════════════════╗"
echo "║              SERVIDOR HTTP INICIADO                      ║"
echo "╚══════════════════════════════════════════════════════════╝"
echo ""
echo "Binario disponible en:"
echo "  http://93.95.231.134:8080/higurashi_mobile"
echo ""
echo "Desde tu UserLAnd ejecuta:"
echo "  wget http://93.95.231.134:8080/higurashi_mobile"
echo "  chmod +x higurashi_mobile"
echo "  ./higurashi_mobile"
echo ""
echo "O usa el script automático:"
echo "  curl -s http://93.95.231.134:8080/setup_mobile.sh | bash"
echo ""
echo "[*] Servidor corriendo... (Ctrl+C para detener)"
echo ""

# Servidor HTTP simple con Python
python3 -m http.server 8080
