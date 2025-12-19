#!/bin/bash
# cleanup_old_files.sh - Eliminar archivos antiguos de Venice.ai

echo "======================================"
echo "🧹 Limpiando archivos antiguos..."
echo "======================================"
echo ""

# Archivos antiguos a eliminar
OLD_FILES=(
    "bot_soldier_v2.c"
    "bot_soldier.c"
    "bot_simple.c"
    "slave_c2.c"
    "master_c2.c"
    "c2_simple.c"
    "old_config.h"
    "version_old.md"
    "README_old.md"
)

# Eliminar archivos si existen
for file in "${OLD_FILES[@]}"; do
    if [ -f "$file" ]; then
        rm -f "$file"
        echo "✓ Eliminado: $file"
    fi
done

echo ""
echo "======================================"
echo "✅ Limpieza completada!"
echo "======================================"
echo ""
echo "Archivos actuales que deben quedar:"
ls -1 *.h *.c 2>/dev/null | grep -v "test_"
echo ""
echo "Ahora puedes subir los archivos nuevos con:"
echo "  scp *.h *.c compile_all.sh root@93.95.231.134:/opt/higurama/"
