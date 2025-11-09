"""Prueba exhaustiva de los 3 contenedores Docker."""
import socket
import time

def test_port(port):
    """Verifica si un puerto está abierto."""
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(3)
        result = sock.connect_ex(('127.0.0.1', port))
        sock.close()
        return result == 0
    except:
        return False

def test_ssh_auth(port, credentials):
    """Prueba credenciales SSH."""
    try:
        import paramiko
        
        for user, pwd in credentials:
            try:
                client = paramiko.SSHClient()
                client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
                client.connect('127.0.0.1', port=port, username=user, password=pwd, timeout=5)
                
                stdin, stdout, stderr = client.exec_command('whoami')
                result = stdout.read().decode('utf-8').strip()
                
                client.close()
                return True, user, pwd, result
            except paramiko.AuthenticationException:
                continue
            except Exception as e:
                continue
        
        return False, None, None, None
    except ImportError:
        print("  [!] Paramiko no instalado")
        return False, None, None, None

if __name__ == "__main__":
    print("\033[96m" + "="*70)
    print("       PRUEBA EXHAUSTIVA DE CONTENEDORES DOCKER")
    print("="*70 + "\033[0m\n")
    
    containers = [
        {'port': 2201, 'name': 'Container 1', 'creds': [('admin', 'admin')]},
        {'port': 2202, 'name': 'Container 2', 'creds': [('test', 'test')]},
        {'port': 2203, 'name': 'Container 3', 'creds': [('root', 'root'), ('user', 'password')]},
    ]
    
    all_ok = True
    
    for container in containers:
        port = container['port']
        name = container['name']
        creds = container['creds']
        
        print(f"\n\033[94m>>> {name} (127.0.0.1:{port})\033[0m")
        
        # Test 1: Puerto abierto
        if test_port(port):
            print(f"  \033[92m✓ Puerto ABIERTO\033[0m")
            
            # Test 2: Banner SSH
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.settimeout(3)
                sock.connect(('127.0.0.1', port))
                banner = sock.recv(1024).decode('utf-8', errors='ignore').strip()
                sock.close()
                print(f"  \033[93m✓ Banner: {banner}\033[0m")
            except:
                print(f"  \033[91m✗ No se pudo obtener banner\033[0m")
            
            # Test 3: Autenticación
            success, user, pwd, whoami = test_ssh_auth(port, creds)
            if success:
                print(f"  \033[92m✓ Auth exitosa: {user}:{pwd} -> {whoami}\033[0m")
            else:
                print(f"  \033[91m✗ Auth fallida con todas las credenciales\033[0m")
                all_ok = False
        else:
            print(f"  \033[91m✗ Puerto CERRADO\033[0m")
            print(f"  \033[93m  Revisa: docker logs vulnerable_ssh_{port-2200}\033[0m")
            all_ok = False
    
    print("\n" + "="*70)
    if all_ok:
        print("\033[92m✓ TODOS LOS CONTENEDORES ESTÁN LISTOS\033[0m")
        print("\n\033[96mAhora puedes ejecutar:\033[0m")
        print("  1. python c2_commander.py")
        print("  2. python bot_soldier.py")
        print("  3. En C2: recon 1")
        print("  4. En C2: list 1")
        print("  5. En C2: exploit 1 1")
    else:
        print("\033[91m✗ HAY PROBLEMAS CON ALGUNOS CONTENEDORES\033[0m")
        print("\n\033[93mSolución:\033[0m")
        print("  1. Ejecuta: fix_docker.bat")
        print("  2. Espera 20 segundos")
        print("  3. Vuelve a ejecutar este script")
    print("="*70 + "\n")
    
    input("Presiona Enter para continuar...")
