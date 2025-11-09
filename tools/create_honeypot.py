"""
Crea un servidor SSH honeypot vulnerable para pruebas de replicación.
Este sistema SIMULA tener credenciales débiles sin comprometer seguridad real.
"""
import socket
import threading
import time

# Credenciales que aceptará el honeypot
FAKE_USERS = {
    'admin': 'admin',
    'root': 'root',
    'test': 'test',
    'user': 'password'
}

def handle_ssh_connection(client_socket, addr):
    """Simula un servidor SSH vulnerable."""
    try:
        # Enviar banner SSH falso
        banner = b"SSH-2.0-OpenSSH_7.4\r\n"
        client_socket.sendall(banner)
        
        print(f"\033[93m[+] Conexión desde {addr}\033[0m")
        
        # Mantener conexión abierta
        time.sleep(30)
        
    except Exception as e:
        print(f"[!] Error: {e}")
    finally:
        client_socket.close()

def start_fake_ssh_server(port=22):
    """Inicia servidor SSH falso."""
    try:
        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        server.bind(('0.0.0.0', port))
        server.listen(5)
        
        print(f"\033[92m[*] Honeypot SSH escuchando en puerto {port}\033[0m")
        print(f"\033[93m[*] Credenciales aceptadas:\033[0m")
        for user, pwd in FAKE_USERS.items():
            print(f"    - {user}:{pwd}")
        print()
        
        while True:
            client, addr = server.accept()
            threading.Thread(target=handle_ssh_connection, 
                           args=(client, addr), 
                           daemon=True).start()
            
    except PermissionError:
        print("\033[91m[!] Error: Puerto 22 requiere privilegios de root/admin\033[0m")
        print("\033[93m[*] Usa: sudo python create_honeypot.py\033[0m")
        print("\033[93m[*] O cambia el puerto a 2222\033[0m")
    except Exception as e:
        print(f"\033[91m[!] Error: {e}\033[0m")

if __name__ == "__main__":
    import sys
    
    port = 2222  # Puerto no privilegiado por defecto
    if len(sys.argv) > 1:
        port = int(sys.argv[1])
    
    print("\033[96m" + "="*60)
    print("       SSH HONEYPOT - Sistema Vulnerable de Prueba")
    print("="*60 + "\033[0m\n")
    
    start_fake_ssh_server(port)
