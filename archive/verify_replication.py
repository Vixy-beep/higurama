"""Verifica que el bot se replicó correctamente en los contenedores."""
import subprocess
import time

containers = ['vulnerable_ssh_1', 'vulnerable_ssh_2', 'vulnerable_ssh_3']

print("="*70)
print("       VERIFICACIÓN DE REPLICACIÓN EN CONTENEDORES")
print("="*70 + "\n")

for container in containers:
    print(f"\n>>> Verificando {container}:")
    
    # Ver si el archivo existe
    result = subprocess.run(
        f'docker exec {container} ls -lh /tmp/bot_soldier.py',
        shell=True, capture_output=True, text=True
    )
    
    if result.returncode == 0:
        print(f"  ✓ Archivo bot_soldier.py existe:")
        print(f"    {result.stdout.strip()}")
        
        # Ver si el proceso está corriendo
        result2 = subprocess.run(
            f'docker exec {container} ps aux | grep "[b]ot_soldier.py"',
            shell=True, capture_output=True, text=True
        )
        
        if result2.stdout.strip():
            print(f"  ✓ Bot está corriendo:")
            print(f"    {result2.stdout.strip()}")
        else:
            print(f"  ✗ Bot NO está corriendo")
            
            # Ver los logs
            result3 = subprocess.run(
                f'docker exec {container} cat /tmp/bot.log',
                shell=True, capture_output=True, text=True
            )
            if result3.stdout:
                print(f"  Logs del bot:")
                print(f"    {result3.stdout[:500]}")
    else:
        print(f"  ✗ Archivo bot_soldier.py NO existe")

print("\n" + "="*70)
input("\nPresiona Enter para continuar...")
