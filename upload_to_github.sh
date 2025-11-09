#!/bin/bash
# Script para subir Mirai-Simulator a GitHub

echo "════════════════════════════════════════════════════════════"
echo "  🚀 SUBIENDO MIRAI-SIMULATOR A GITHUB 🚀"
echo "════════════════════════════════════════════════════════════"
echo ""

# Verificar si git está instalado
if ! command -v git &> /dev/null; then
    echo "❌ Git no está instalado. Instálalo primero:"
    echo "   Windows: https://git-scm.com/download/win"
    echo "   Linux: sudo apt install git"
    exit 1
fi

# Solicitar información del usuario
echo "📝 Configuración de GitHub:"
read -p "Tu usuario de GitHub: " GITHUB_USER
read -p "Nombre del repositorio [Mirai-Simulator]: " REPO_NAME
REPO_NAME=${REPO_NAME:-Mirai-Simulator}

echo ""
echo "⚙️  Inicializando repositorio Git..."

# Inicializar Git
git init

# Configurar usuario (si no está configurado)
if [ -z "$(git config user.name)" ]; then
    read -p "Tu nombre para commits: " GIT_NAME
    git config user.name "$GIT_NAME"
fi

if [ -z "$(git config user.email)" ]; then
    read -p "Tu email para commits: " GIT_EMAIL
    git config user.email "$GIT_EMAIL"
fi

# Agregar archivos
echo "📦 Agregando archivos al staging..."
git add .

# Commit inicial
echo "💾 Creando commit inicial..."
git commit -m "Initial commit - Educational botnet simulator

Features:
- Multi-C2 architecture with failover
- Tor hidden service support
- SSH auto-replication
- Shodan integration
- Cross-platform (Windows/Linux)
- Educational purpose only with full disclaimers"

# Crear rama main
git branch -M main

# Agregar remote
REPO_URL="https://github.com/$GITHUB_USER/$REPO_NAME.git"
echo "🔗 Conectando con repositorio remoto..."
git remote add origin $REPO_URL

echo ""
echo "════════════════════════════════════════════════════════════"
echo "✅ LISTO PARA SUBIR"
echo "════════════════════════════════════════════════════════════"
echo ""
echo "⚠️  ANTES DE CONTINUAR:"
echo ""
echo "1. Asegúrate de haber creado el repositorio en GitHub:"
echo "   👉 https://github.com/new"
echo ""
echo "2. Nombre del repo: $REPO_NAME"
echo "3. Visibilidad: Public o Private (tu elección)"
echo "4. NO inicialices con README (ya lo tenemos)"
echo ""
read -p "¿Ya creaste el repositorio en GitHub? (s/n): " CONFIRM

if [ "$CONFIRM" != "s" ] && [ "$CONFIRM" != "S" ]; then
    echo ""
    echo "❌ Proceso cancelado. Crea el repositorio primero."
    exit 0
fi

# Subir a GitHub
echo ""
echo "🚀 Subiendo archivos a GitHub..."
git push -u origin main

# Verificar resultado
if [ $? -eq 0 ]; then
    echo ""
    echo "════════════════════════════════════════════════════════════"
    echo "✅ ¡ÉXITO! Repositorio subido correctamente"
    echo "════════════════════════════════════════════════════════════"
    echo ""
    echo "📂 Tu repositorio está en:"
    echo "   👉 https://github.com/$GITHUB_USER/$REPO_NAME"
    echo ""
    echo "📋 Próximos pasos sugeridos:"
    echo ""
    echo "1. Agregar topics/tags en GitHub:"
    echo "   Settings → Topics → Agregar:"
    echo "   • cybersecurity"
    echo "   • pentesting"
    echo "   • red-team"
    echo "   • educational"
    echo "   • botnet-simulation"
    echo "   • security-research"
    echo ""
    echo "2. Activar GitHub Pages (opcional):"
    echo "   Settings → Pages → Source: main branch"
    echo ""
    echo "3. Agregar descripción corta:"
    echo "   'Educational botnet simulator for security research'"
    echo ""
else
    echo ""
    echo "❌ Error al subir. Posibles causas:"
    echo "   • Repositorio no existe en GitHub"
    echo "   • Credenciales incorrectas"
    echo "   • Sin permisos de escritura"
    echo ""
    echo "Solución:"
    echo "1. Verifica que el repo existe: https://github.com/$GITHUB_USER/$REPO_NAME"
    echo "2. Configura tu token de acceso personal (PAT)"
    echo "3. Reintenta: git push -u origin main"
fi
