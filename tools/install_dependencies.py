"""Instala las dependencias necesarias para el botnet."""
import subprocess
import sys
import platform

def install_package(package):
    """Instala un paquete usando pip."""
    try:
        print(f"[*] Instalando {package}...")
        subprocess.check_call([sys.executable, "-m", "pip", "install", package])
        print(f"[+] {package} instalado correctamente")
        return True
    except Exception as e:
        print(f"[!] Error instalando {package}: {e}")
        return False

if __name__ == "__main__":
    packages = ["paramiko", "netaddr", "cryptography", "scapy"]
    
    print("="*60)
    print("  LUCKY STAR BOTNET - INSTALACIÓN DE DEPENDENCIAS")
    print("="*60)
    print(f"[*] Python: {sys.executable}")
    print(f"[*] Versión: {sys.version}")
    print(f"[*] OS: {platform.system()}")
    print()
    
    # Advertencia especial para Windows
    if platform.system() == 'Windows':
        print("\033[93m[!] IMPORTANTE PARA WINDOWS:\033[0m")
        print("    Scapy requiere Npcap para funcionar correctamente")
        print("    Descarga: https://npcap.com/#download")
        print("    Instala Npcap ANTES de continuar")
        print()
        resp = input("¿Ya instalaste Npcap? (s/n): ").lower()
        if resp != 's':
            print("\n[!] Instala Npcap primero y vuelve a ejecutar este script")
            sys.exit(1)
    
    success = []
    failed = []
    
    for pkg in packages:
        if install_package(pkg):
            success.append(pkg)
        else:
            failed.append(pkg)
    
    print("\n" + "="*60)
    print(f"[+] Instalados correctamente: {len(success)}/{len(packages)}")
    if failed:
        print(f"[!] Fallaron: {', '.join(failed)}")
    else:
        print("[+] ¡Todas las dependencias instaladas!")
    print("="*60)
    
    # Verificar funcionalidad de Scapy
    if 'scapy' in success:
        print("\n[*] Verificando Scapy...")
        try:
            from scapy.all import ARP
            print("[+] Scapy funciona correctamente")
        except Exception as e:
            print(f"[!] Scapy instalado pero no funciona: {e}")
            if platform.system() == 'Windows':
                print("[!] Asegúrate de haber instalado Npcap")
