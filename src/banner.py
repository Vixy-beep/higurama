# banner.py - Sistema de ASCII Art dinámico para el C2
import os
import random

# ═══════════════════════════════════════════════════════════
# FUNCIÓN DE CENTRADO
# ═══════════════════════════════════════════════════════════
def center_ascii(text, width=None):
    """Centra ASCII art en la terminal."""
    if width is None:
        try:
            width = os.get_terminal_size().columns
        except:
            width = 120
    
    lines = text.split('\n')
    centered_lines = []
    
    for line in lines:
        # Calcular espacios necesarios
        line_len = len(line)
        if line_len < width:
            padding = (width - line_len) // 2
            centered_lines.append(' ' * padding + line)
        else:
            centered_lines.append(line)
    
    return '\n'.join(centered_lines)

# ═══════════════════════════════════════════════════════════
# BANNER PRINCIPAL (se carga desde archivo)
# ═══════════════════════════════════════════════════════════
try:
    with open('c2 ascii.txt', 'r', encoding='utf-8') as f:
        MAIN_BANNER = f.read()
except FileNotFoundError:
    MAIN_BANNER = "[!] c2 ascii.txt no encontrado"

# ═══════════════════════════════════════════════════════════
# ASCII ARTS PARA ACCIONES ESPECÍFICAS
# ═══════════════════════════════════════════════════════════

ASCII_ARTS = {}

# Ejemplos alternativos (descomenta el que quieras):

# ═══════════════════════════════════════════════════════════
# Ejemplo 1: Mirai Original
# ═══════════════════════════════════════════════════════════
MIRAI_ORIGINAL = r"""
███╗   ███╗██╗██████╗  █████╗ ██╗
████╗ ████║██║██╔══██╗██╔══██╗██║
██╔████╔██║██║██████╔╝███████║██║
██║╚██╔╝██║██║██╔══██╗██╔══██║██║
██║ ╚═╝ ██║██║██║  ██║██║  ██║██║
╚═╝     ╚═╝╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝
    Botnet C2 Command Center
"""

# ═══════════════════════════════════════════════════════════
# Ejemplo 2: Cyberpunk
# ═══════════════════════════════════════════════════════════
CYBERPUNK = r"""
    ▄████████ ▀████    ▐████▀       ▄████████    ▄████████
   ███    ███   ███▌   ████▀       ███    ███   ███    ███
   ███    █▀     ███  ▐███         ███    █▀    ███    █▀
  ▄███▄▄▄        ▀███▄███▀        ▄███▄▄▄      ▄███▄▄▄
 ▀▀███▀▀▀        ████▀██▄        ▀▀███▀▀▀     ▀▀███▀▀▀
   ███    █▄    ▐███  ▀███         ███    █▄    ███    █▄
   ███    ███  ▄███     ███▄       ███    ███   ███    ███
   ██████████ ████       ███▄      ██████████   ██████████
        ╔═══════════════════════════════════╗
        ║   C2 Neural Network Interface   ║
        ╚═══════════════════════════════════╝
"""

# ═══════════════════════════════════════════════════════════
# Ejemplo 3: Matrix Style
# ═══════════════════════════════════════════════════════════
MATRIX = r"""
    ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
    ▓                                                ▓
    ▓  ███╗   ███╗ █████╗ ████████╗██████╗ ██╗██╗  ▓
    ▓  ████╗ ████║██╔══██╗╚══██╔══╝██╔══██╗██║╚██╗ ▓
    ▓  ██╔████╔██║███████║   ██║   ██████╔╝██║ ╚██╗▓
    ▓  ██║╚██╔╝██║██╔══██║   ██║   ██╔══██╗██║ ██╔╝▓
    ▓  ██║ ╚═╝ ██║██║  ██║   ██║   ██║  ██║██║██╔╝ ▓
    ▓  ╚═╝     ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚═╝  ▓
    ▓                                                ▓
    ▓         > Initializing Command Grid           ▓
    ▓         > Neural Link Established             ▓
    ▓                                                ▓
    ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓
"""

# ═══════════════════════════════════════════════════════════
# Ejemplo 4: Hacker Terminal
# ═══════════════════════════════════════════════════════════
HACKER_TERMINAL = r"""
┌─────────────────────────────────────────────────────────┐
│ root@c2-server:~# ./initialize_botnet                  │
│                                                         │
│  ██████╗ ██████╗     ██████╗  █████╗ ███╗   ██╗███████╗│
│ ██╔════╝██╔════╝     ██╔══██╗██╔══██╗████╗  ██║██╔════╝│
│ ██║     ╚████╗       ██████╔╝███████║██╔██╗ ██║█████╗  │
│ ██║      ╚══██╗      ██╔═══╝ ██╔══██║██║╚██╗██║██╔══╝  │
│ ╚██████╗██████║      ██║     ██║  ██║██║ ╚████║███████╗│
│  ╚═════╝╚═════╝      ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═══╝╚══════╝│
│                                                         │
│ [✓] System initialized                                 │
│ [✓] Network layer active                               │
│ [✓] Awaiting bot connections...                        │
└─────────────────────────────────────────────────────────┘
"""

# ═══════════════════════════════════════════════════════════
# Ejemplo 5: Simple y Limpio
# ═══════════════════════════════════════════════════════════
SIMPLE = r"""
═══════════════════════════════════════════════════
    
    ▄▀▀ ▄▀▄ █▄ ▄█ █▄ ▄█ ▄▀▄ █▄ █ █▀▄   ▄▀▀ ▄▀▄ █▄ █ ▀█▀ █▀▀ █▀▄
    ▀▄▄ ▀▄▀ █ ▀ █ █ ▀ █ ▀▄▀ █ ▀█ █▄▀   ▀▄▄ ▀▄▀ █ ▀█  █  █▄▄ █▀▄
    
        Advanced Bot Network - Control Interface
        
═══════════════════════════════════════════════════
"""


# ═══════════════════════════════════════════════════════════
# CARGAR ASCII ARTS DESDE CARPETA
# ═══════════════════════════════════════════════════════════
def load_ascii_from_folder(folder='ascii_arts'):
    """Carga todos los ASCII arts desde una carpeta."""
    if not os.path.exists(folder):
        return {}
    
    arts = {}
    for filename in os.listdir(folder):
        if filename.endswith('.txt'):
            action_name = filename.replace('.txt', '').lower()
            try:
                with open(os.path.join(folder, filename), 'r', encoding='utf-8') as f:
                    arts[action_name] = f.read()
            except:
                pass
    
    return arts

# Cargar artes automáticamente
ASCII_ARTS.update(load_ascii_from_folder())

# ═══════════════════════════════════════════════════════════
# FUNCIONES PARA USAR EN C2
# ═══════════════════════════════════════════════════════════
def get_banner():
    """Retorna el banner principal."""
    return MAIN_BANNER

def get_ascii(action_name):
    """Retorna ASCII art para acción específica."""
    return ASCII_ARTS.get(action_name.lower(), None)

def show_ascii(action_name, color='\033[95m'):
    """Muestra ASCII art centrado y coloreado."""
    art = get_ascii(action_name)
    if art:
        centered = center_ascii(art)
        print(color + centered + '\033[0m')
        return True
    return False


# ═══════════════════════════════════════════════════════════
# GENERADORES ONLINE RECOMENDADOS:
# ═══════════════════════════════════════════════════════════
# 1. https://patorjk.com/software/taag/
# 2. https://www.ascii-art-generator.org/
# 3. https://www.kammerl.de/ascii/AsciiSignature.php
# 4. https://fsymbols.com/generators/carty/
