# shodan_hunter.py - Módulo de caza automática con Shodan
import json
import time
import random

try:
    import shodan
    HAS_SHODAN = True
except ImportError:
    HAS_SHODAN = False
    print("[!] Shodan no instalado. Instalar con: pip install shodan")

# APIs de Shodan (conseguir en https://account.shodan.io/)
# NOTA: Usar cuentas gratuitas rotativas para evitar rate limits
SHODAN_API_KEYS = [
    'TU_API_KEY_1',  # Reemplazar con tu API key
    'TU_API_KEY_2',  # Agregar más para rotar
    'TU_API_KEY_3',
]

# Queries de búsqueda (objetivos vulnerables)
SHODAN_QUERIES = [
    # SSH con credenciales débiles
    'port:22 "SSH-2.0-OpenSSH" country:US,CA,GB,DE',
    'port:22 product:OpenSSH version:7.4',
    
    # FTP anónimo
    'port:21 "220" "FTP"',
    'port:21 anonymous',
    
    # Telnet abierto
    'port:23 telnet',
    
    # Routers expuestos
    'http.title:"Router" port:80',
    'http.html:"admin login" port:8080',
    
    # Cámaras IP
    'http.title:"IP Camera"',
    'Server: IP Webcam',
    
    # IoT devices
    'port:23 device:iot',
    'product:"MikroTik router"',
    
    # Bases de datos expuestas
    'port:27017 mongodb',
    'port:6379 redis',
    'port:5432 postgresql',
    
    # Servidores web vulnerables
    'apache 2.4.49',  # Path traversal
    'http.title:"Index of /"',
    
    # Docker expuesto
    'port:2375 docker',
    
    # Kubernetes
    'port:10250 kubelet',
]


class ShodanHunter:
    """Cazador automático de objetivos usando Shodan."""
    
    def __init__(self, api_keys=None):
        """Inicializa el cazador."""
        self.api_keys = api_keys or SHODAN_API_KEYS
        self.current_key_index = 0
        self.api = None
        self.targets_found = []
        
        if HAS_SHODAN and self.api_keys and self.api_keys[0] != 'TU_API_KEY_1':
            self._init_api()
    
    def _init_api(self):
        """Inicializa cliente Shodan con rotación de keys."""
        try:
            key = self.api_keys[self.current_key_index]
            self.api = shodan.Shodan(key)
            print(f"[+] Shodan API inicializada (key #{self.current_key_index + 1})")
        except Exception as e:
            print(f"[!] Error inicializando Shodan: {e}")
    
    def _rotate_key(self):
        """Rota a la siguiente API key si hay rate limit."""
        self.current_key_index = (self.current_key_index + 1) % len(self.api_keys)
        self._init_api()
        print(f"[*] Rotando a API key #{self.current_key_index + 1}")
    
    def search_targets(self, query, max_results=100):
        """Busca objetivos en Shodan."""
        if not self.api:
            print("[!] Shodan API no disponible")
            return []
        
        targets = []
        try:
            print(f"[*] Buscando: {query[:50]}...")
            results = self.api.search(query, limit=max_results)
            
            for result in results['matches']:
                target = {
                    'ip': result['ip_str'],
                    'port': result['port'],
                    'country': result.get('location', {}).get('country_code', 'Unknown'),
                    'org': result.get('org', 'Unknown'),
                    'os': result.get('os', 'Unknown'),
                    'product': result.get('product', 'Unknown'),
                    'version': result.get('version', 'Unknown'),
                    'data': result.get('data', '')[:500],  # Primeros 500 chars
                    'vulns': result.get('vulns', []),
                    'score': self._calculate_score(result)
                }
                targets.append(target)
            
            print(f"[+] Encontrados {len(targets)} objetivos")
            return sorted(targets, key=lambda x: x['score'], reverse=True)
            
        except shodan.APIError as e:
            if 'rate limit' in str(e).lower():
                print("[!] Rate limit alcanzado, rotando API key...")
                self._rotate_key()
                time.sleep(5)
                return self.search_targets(query, max_results)  # Reintentar
            else:
                print(f"[!] Error Shodan: {e}")
                return []
    
    def _calculate_score(self, result):
        """Calcula score de explotabilidad (0-100)."""
        score = 50  # Base
        
        # Bonus por vulnerabilidades conocidas
        if result.get('vulns'):
            score += len(result['vulns']) * 10
        
        # Bonus por servicios conocidos
        product = result.get('product', '').lower()
        if 'openssh' in product:
            score += 15
        elif 'ftp' in product:
            score += 20
        elif 'telnet' in product:
            score += 25
        
        # Bonus por credenciales débiles en banner
        data = result.get('data', '').lower()
        if any(cred in data for cred in ['admin', 'root', 'password', 'default']):
            score += 20
        
        # Penalty por países con mejor seguridad
        country = result.get('location', {}).get('country_code', '')
        if country in ['US', 'GB', 'DE', 'CA']:
            score -= 10
        
        return min(100, max(0, score))
    
    def hunt_continuous(self, callback=None, interval=300):
        """Caza continua de objetivos."""
        print("[*] Iniciando caza continua...")
        query_index = 0
        
        while True:
            try:
                # Rotar queries
                query = SHODAN_QUERIES[query_index % len(SHODAN_QUERIES)]
                query_index += 1
                
                targets = self.search_targets(query, max_results=50)
                
                if targets and callback:
                    # Enviar mejores objetivos al callback
                    top_targets = targets[:10]  # Top 10
                    callback(top_targets)
                
                # Esperar antes de siguiente búsqueda
                wait_time = interval + random.randint(-60, 60)
                print(f"[*] Esperando {wait_time}s antes de siguiente búsqueda...")
                time.sleep(wait_time)
                
            except KeyboardInterrupt:
                print("\n[!] Caza detenida")
                break
            except Exception as e:
                print(f"[!] Error en caza: {e}")
                time.sleep(60)
    
    def get_host_info(self, ip):
        """Obtiene información detallada de un host."""
        if not self.api:
            return None
        
        try:
            host = self.api.host(ip)
            return {
                'ip': host['ip_str'],
                'os': host.get('os'),
                'ports': host.get('ports', []),
                'vulns': host.get('vulns', []),
                'services': [
                    {
                        'port': s['port'],
                        'product': s.get('product', 'Unknown'),
                        'version': s.get('version', 'Unknown')
                    }
                    for s in host.get('data', [])
                ]
            }
        except shodan.APIError as e:
            print(f"[!] Error obteniendo info de {ip}: {e}")
            return None


# Alternativas gratuitas a Shodan
class CensysHunter:
    """Cazador usando Censys (alternativa a Shodan)."""
    
    def __init__(self, api_id=None, api_secret=None):
        try:
            from censys.search import CensysHosts
            self.api = CensysHosts(api_id, api_secret)
            print("[+] Censys API inicializada")
        except ImportError:
            print("[!] Censys no instalado. pip install censys")
            self.api = None
    
    def search_targets(self, query, max_results=100):
        """Busca objetivos en Censys."""
        if not self.api:
            return []
        
        targets = []
        try:
            for page in self.api.search(query, per_page=100, pages=1):
                for host in page:
                    target = {
                        'ip': host['ip'],
                        'port': host.get('services', [{}])[0].get('port', 0),
                        'services': host.get('services', [])
                    }
                    targets.append(target)
            
            return targets
        except Exception as e:
            print(f"[!] Error Censys: {e}")
            return []


class ZoomEyeHunter:
    """Cazador usando ZoomEye (popular en Asia)."""
    
    def __init__(self, api_key=None):
        self.api_key = api_key
        self.base_url = 'https://api.zoomeye.org'
    
    def search_targets(self, query, max_results=100):
        """Busca objetivos en ZoomEye."""
        import requests
        
        if not self.api_key:
            return []
        
        headers = {'API-KEY': self.api_key}
        url = f"{self.base_url}/host/search"
        params = {'query': query, 'page': 1}
        
        try:
            response = requests.get(url, headers=headers, params=params, timeout=10)
            data = response.json()
            
            targets = []
            for match in data.get('matches', []):
                target = {
                    'ip': match.get('ip'),
                    'port': match.get('portinfo', {}).get('port'),
                    'country': match.get('geoinfo', {}).get('country', {}).get('code')
                }
                targets.append(target)
            
            return targets
        except Exception as e:
            print(f"[!] Error ZoomEye: {e}")
            return []


def demo_usage():
    """Demostración de uso."""
    print("=" * 60)
    print("  SHODAN HUNTER - Demo")
    print("=" * 60)
    
    hunter = ShodanHunter()
    
    # Búsqueda simple
    targets = hunter.search_targets('port:22 openssh', max_results=10)
    
    print("\n[*] Top 5 objetivos:")
    for i, target in enumerate(targets[:5], 1):
        print(f"\n{i}. {target['ip']}:{target['port']}")
        print(f"   Score: {target['score']}/100")
        print(f"   País: {target['country']}")
        print(f"   Producto: {target['product']} {target['version']}")
        if target['vulns']:
            print(f"   Vulns: {', '.join(target['vulns'])}")


if __name__ == "__main__":
    demo_usage()
