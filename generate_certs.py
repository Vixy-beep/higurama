"""Genera certificados SSL autofirmados para el C2."""
import os
import sys

def generate_certs_python():
    """Genera certificados usando cryptography."""
    try:
        from cryptography import x509
        from cryptography.x509.oid import NameOID
        from cryptography.hazmat.primitives import hashes
        from cryptography.hazmat.primitives.asymmetric import rsa
        from cryptography.hazmat.primitives import serialization
        import datetime
        
        print("[*] Generando certificados SSL...")
        
        # Generar clave privada
        private_key = rsa.generate_private_key(
            public_exponent=65537,
            key_size=2048,
        )
        
        # Crear certificado
        subject = issuer = x509.Name([
            x509.NameAttribute(NameOID.COUNTRY_NAME, u"US"),
            x509.NameAttribute(NameOID.STATE_OR_PROVINCE_NAME, u"State"),
            x509.NameAttribute(NameOID.LOCALITY_NAME, u"City"),
            x509.NameAttribute(NameOID.ORGANIZATION_NAME, u"Org"),
            x509.NameAttribute(NameOID.COMMON_NAME, u"localhost"),
        ])
        
        cert = x509.CertificateBuilder().subject_name(
            subject
        ).issuer_name(
            issuer
        ).public_key(
            private_key.public_key()
        ).serial_number(
            x509.random_serial_number()
        ).not_valid_before(
            datetime.datetime.utcnow()
        ).not_valid_after(
            datetime.datetime.utcnow() + datetime.timedelta(days=365)
        ).sign(private_key, hashes.SHA256())
        
        # Guardar clave privada
        with open("key.pem", "wb") as f:
            f.write(private_key.private_bytes(
                encoding=serialization.Encoding.PEM,
                format=serialization.PrivateFormat.TraditionalOpenSSL,
                encryption_algorithm=serialization.NoEncryption()
            ))
        
        # Guardar certificado
        with open("cert.pem", "wb") as f:
            f.write(cert.public_bytes(serialization.Encoding.PEM))
        
        print("[+] Certificados generados: cert.pem, key.pem")
        return True
        
    except ImportError:
        print("[!] Módulo 'cryptography' no encontrado")
        print("[*] Instalando cryptography...")
        import subprocess
        subprocess.check_call([sys.executable, "-m", "pip", "install", "cryptography"])
        print("[*] Reintentando generación de certificados...")
        return generate_certs_python()
    except Exception as e:
        print(f"[!] Error generando certificados: {e}")
        return False

if __name__ == "__main__":
    if os.path.exists("cert.pem") and os.path.exists("key.pem"):
        print("[!] Los certificados ya existen.")
        resp = input("¿Regenerar? (s/n): ").lower()
        if resp != 's':
            print("[*] Usando certificados existentes.")
            sys.exit(0)
    
    if generate_certs_python():
        print("\n[+] ¡Listo! Ahora puedes ejecutar c2_commander.py")
    else:
        print("\n[!] Fallo al generar certificados")
        sys.exit(1)
