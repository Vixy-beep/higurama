═══════════════════════════════════════════════════════════
  INSTRUCCIONES: Sistema de ASCII Arts Dinámicos
═══════════════════════════════════════════════════════════

## 📁 Cómo funciona:

1. **Crear archivos .txt** en esta carpeta (ascii_arts/)
2. **Nombrar según la acción**: 
   - exploit.txt     → Se muestra al explotar
   - recon.txt       → Se muestra al escanear
   - success.txt     → Éxito
   - fail.txt        → Error
   - vixy.txt        → Tu personaje
   - luckystar.txt   → Lucky Star
   - etc.

3. **El sistema los carga automáticamente** al iniciar C2


## 🎨 Ejemplos de nombres:

```
ascii_arts/
├── exploit.txt      ← python c2: "exploit 1 1" muestra este
├── recon.txt        ← "recon 1" muestra este
├── shodan.txt       ← "shodan 1" muestra este
├── success.txt      ← Éxito general
├── fail.txt         ← Error general
├── vixy.txt         ← Tu waifu
├── konata.txt       ← Konata Izumi
├── tsukasa.txt      ← Tsukasa Hiiragi
├── kagami.txt       ← Kagami Hiiragi
└── miyuki.txt       ← Miyuki Takara
```


## 💻 Cómo se usa en el código:

```python
# En c2_commander.py:
from banner import show_ascii

# Al explotar:
show_ascii('exploit', color='\033[92m')  # Verde

# Al escanear:
show_ascii('recon', color='\033[96m')    # Cyan

# Éxito:
show_ascii('success', color='\033[92m')

# Error:
show_ascii('fail', color='\033[91m')

# Random Lucky Star character:
show_ascii('vixy', color='\033[95m')
```


## 🌐 Generadores recomendados:

1. **ASCII Art Generator** (para fotos):
   https://www.ascii-art-generator.org/

2. **Image to ASCII** (mejor calidad):
   https://www.ascii-image-converter.org/

3. **Text to ASCII Art**:
   https://patorjk.com/software/taag/

4. **Anime ASCII Art** (manual):
   https://www.asciiart.eu/
   https://ascii.co.uk/art/anime


## ⚙️ Tips para mejores resultados:

### Para imágenes (como Vixy):
1. Reducir imagen a 200-300px de ancho
2. Convertir a blanco y negro
3. Aumentar contraste
4. Usar generador con caracteres: #%*+=-.

### Para texto grande:
- Fuente recomendada: "ANSI Shadow"
- Ancho máximo: ~100 caracteres
- Probar diferentes fuentes en patorjk.com


## 📋 Template de ejemplo (copiar y modificar):

Archivo: vixy.txt
```
        ⠀⠀⠀⠀⠀⣀⣤⣴⣶⣾⣿⣷⣶⣦⣤⣀⠀⠀⠀
        ⠀⠀⢀⣤⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣤⡀
        ⠀⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣦
        ⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿
              ★ VIXY-CHAN ★
         Sistema Neural Activado
```


## 🎭 Colores disponibles:

```python
'\033[91m'  # Rojo - Errores
'\033[92m'  # Verde - Éxito
'\033[93m'  # Amarillo - Advertencias
'\033[94m'  # Azul - Info
'\033[95m'  # Magenta - Principal
'\033[96m'  # Cyan - Secundario
'\033[97m'  # Blanco - Texto normal

# Especiales Lucky Star:
'\033[38;5;213m'  # Rosa kawaii
'\033[38;5;141m'  # Púrpura suave
'\033[38;5;219m'  # Rosa claro
```


## ✅ Checklist para agregar nuevo ASCII:

1. [ ] Crear imagen/texto ASCII
2. [ ] Guardar como .txt en ascii_arts/
3. [ ] Nombre descriptivo (sin espacios)
4. [ ] Reiniciar C2 para cargar
5. [ ] Testear con: show_ascii('nombre')


═══════════════════════════════════════════════════════════
  ¡Agrega tus waifus favoritas y personaliza tu C2! 🎀
═══════════════════════════════════════════════════════════
