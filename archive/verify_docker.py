"""Verifica que los contenedores Docker están listos para ser atacados."""
import socket
import time

DOCKER_PORTS = [2201, 2202, 2203]

def check_port(port):
    """Verifica si un puerto está abierto."""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(2)
        result = sock.connect_ex(('127.0.0.1', port))
        sock.close()
        return result == 0
    except:
        return False

def check_ssh_banner(port):
    """Intenta obtener el banner SSH."""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(3)
        sock.connect(('127.0.0.1', port))
        banner = sock.recv(1024).decode('utf-8', errors='ignore')
        sock.close()
        return banner.strip()
    except:
        return None

print("\033[96m" + "="*60)
print("       VERIFICACIÓN DE CONTENEDORES DOCKER")
print("="*60 + "\033[0m\n")

all_ready = True

for port in DOCKER_PORTS:
    print(f"[*] Verificando localhost:{port}...", end=" ")
    
    if check_port(port):
        print("\033[92m✓ ABIERTO\033[0m")
        
        banner = check_ssh_banner(port)
        if banner:
            print(f"    Banner: \033[93m{banner[:50]}...\033[0m")
        else:
            print("    \033[91m[!] No se pudo obtener banner SSH\033[0m")
    else:
        print("\033[91m✗ CERRADO\033[0m")
        all_ready = False

print()

if all_ready:
    print("\033[92m[+] ¡Todos los contenedores están listos!\033[0m")
    print("\033[93m[*] Ahora puedes ejecutar:\033[0m")
    print("    1. python c2_commander.py")
    print("    2. python bot_soldier.py")
    print("    3. En C2: recon 1")
    print("    4. En C2: list 1")
    print("    5. En C2: exploit 1 1")
else:
    print("\033[91m[!] Algunos contenedores no están listos\033[0m")
    print("\033[93m[*] Ejecuta: docker-compose -f docker-vulnerable-ssh.yml up -d\033[0m")
    print("\033[93m[*] Espera 15 segundos y vuelve a verificar\033[0m")

print()
input("Presiona Enter para continuar...")
