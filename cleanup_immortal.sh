#!/bin/bash
# Script para ELIMINAR completamente Higurashi Immortal
# ⚠️ SOLO usar en tu propia máquina de pruebas

echo "🧹 Eliminando Higurashi Immortal..."
echo "⚠️  Esto eliminará TODOS los rastros del bot"
echo ""
read -p "¿Estás seguro? (escribe 'SI' para confirmar): " confirm

if [ "$confirm" != "SI" ]; then
    echo "Cancelado."
    exit 0
fi

echo ""
echo "[*] Matando procesos..."

# Matar todos los procesos relacionados
pkill -9 -f "systemd-.*helper"
pkill -9 -f "systemd-.*daemon"
pkill -9 -f "systemd-.*service"
pkill -9 -f "systemd-.*agent"
pkill -9 -f "systemd-.*worker"
pkill -9 -f "kworker-.*helper"
pkill -9 -f "kworker-.*daemon"
pkill -9 -f "update-.*helper"
pkill -9 -f "network-.*helper"
pkill -9 -f "cron-.*helper"
pkill -9 -f "dbus-.*helper"
pkill -9 -f "higurashi_immortal"

echo "[*] Eliminando archivos..."

# Eliminar binarios de todas las ubicaciones posibles
rm -f /usr/bin/systemd-* /usr/bin/kworker-* /usr/bin/update-* /usr/bin/network-* /usr/bin/cron-* /usr/bin/dbus-*
rm -f /usr/sbin/systemd-* /usr/sbin/kworker-* /usr/sbin/update-* /usr/sbin/network-* /usr/sbin/cron-* /usr/sbin/dbus-*
rm -f /usr/local/bin/systemd-* /usr/local/bin/kworker-* /usr/local/bin/update-* /usr/local/bin/network-* /usr/local/bin/cron-* /usr/local/bin/dbus-*
rm -f /bin/systemd-* /bin/kworker-* /bin/update-* /bin/network-* /bin/cron-* /bin/dbus-*
rm -f /sbin/systemd-* /sbin/kworker-* /sbin/update-* /sbin/network-* /sbin/cron-* /sbin/dbus-*
rm -f /var/tmp/.systemd-* /var/tmp/.kworker-* /var/tmp/.update-* /var/tmp/.network-* /var/tmp/.cron-* /var/tmp/.dbus-*
rm -f /tmp/.systemd-* /tmp/.kworker-* /tmp/.update-* /tmp/.network-* /tmp/.cron-* /tmp/.dbus-*
rm -f /dev/shm/.systemd-* /dev/shm/.kworker-* /dev/shm/.update-* /dev/shm/.network-* /dev/shm/.cron-* /dev/shm/.dbus-*
rm -f /opt/.systemd-* /opt/.kworker-* /opt/.update-* /opt/.network-* /opt/.cron-* /opt/.dbus-*

echo "[*] Limpiando crontab..."

# Limpiar crontab
crontab -l 2>/dev/null | grep -v "systemd-" | grep -v "kworker-" | grep -v "update-" | grep -v "network-" | grep -v "cron-" | grep -v "dbus-" | crontab - 2>/dev/null

echo "[*] Eliminando servicios systemd..."

# Eliminar servicios systemd
systemctl stop systemd-*helper* systemd-*daemon* systemd-*service* systemd-*agent* systemd-*worker* 2>/dev/null
systemctl disable systemd-*helper* systemd-*daemon* systemd-*service* systemd-*agent* systemd-*worker* 2>/dev/null
rm -f /etc/systemd/system/systemd-*.service
rm -f /etc/systemd/system/kworker-*.service
rm -f /etc/systemd/system/update-*.service
rm -f /etc/systemd/system/network-*.service
rm -f /etc/systemd/system/cron-*.service
rm -f /etc/systemd/system/dbus-*.service
systemctl daemon-reload 2>/dev/null

echo "[*] Limpiando rc.local..."

# Limpiar rc.local
if [ -f /etc/rc.local ]; then
    sed -i '/systemd-/d' /etc/rc.local
    sed -i '/kworker-/d' /etc/rc.local
    sed -i '/update-/d' /etc/rc.local
    sed -i '/network-/d' /etc/rc.local
    sed -i '/cron-/d' /etc/rc.local
    sed -i '/dbus-/d' /etc/rc.local
fi

echo "[*] Limpiando .bashrc de usuarios..."

# Limpiar .bashrc
find /home -name .bashrc -exec sed -i '/systemd-/d; /kworker-/d; /update-/d; /network-/d; /cron-/d; /dbus-/d' {} \; 2>/dev/null

echo "[*] Eliminando init scripts..."

# Eliminar init scripts
rm -f /etc/init.d/systemd-* /etc/init.d/kworker-* /etc/init.d/update-* /etc/init.d/network-* /etc/init.d/cron-* /etc/init.d/dbus-*
rm -f /etc/rc.d/init.d/systemd-* /etc/rc.d/init.d/kworker-* /etc/rc.d/init.d/update-* /etc/rc.d/init.d/network-* /etc/rc.d/init.d/cron-* /etc/rc.d/init.d/dbus-*

echo ""
echo "✅ Limpieza completa"
echo ""
echo "Verificando procesos restantes:"
ps aux | grep -E "(systemd-|kworker-|update-|network-|cron-|dbus-).*(helper|daemon|service|agent|worker)" | grep -v grep

echo ""
echo "Si aún ves procesos, reinicia el sistema para eliminarlos completamente."
