// test_animations.c - Test ASCII animations thread safety
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "ascii_art.h"

// Simula múltiples bots conectándose simultáneamente
void *simulate_bot_connection(void *arg) {
    int bot_id = *(int *)arg;
    
    printf("\n[Thread %d] Bot connecting...\n", bot_id);
    usleep(100000 * bot_id);  // Stagger connections slightly
    
    // Intentar mostrar animación de replicación
    show_replication_animation();
    
    printf("[Thread %d] Bot connection complete\n", bot_id);
    return NULL;
}

int main() {
    printf("====================================\n");
    printf("   ASCII ANIMATION THREAD TEST\n");
    printf("====================================\n\n");
    
    // Test 1: Lucky Star splash (single thread)
    printf("TEST 1: Lucky Star Splash Animation\n");
    printf("------------------------------------\n");
    show_splash_animation();
    sleep(1);
    
    // Test 2: Multiple simultaneous replications (thread safety test)
    printf("\n\nTEST 2: Simultaneous Bot Connections (Thread Safety)\n");
    printf("-----------------------------------------------------\n");
    printf("Spawning 5 bot connections simultaneously...\n");
    printf("Expected: Only 1-2 animations show (others skipped)\n\n");
    
    pthread_t threads[5];
    int bot_ids[5] = {1, 2, 3, 4, 5};
    
    // Launch all threads at once
    for (int i = 0; i < 5; i++) {
        pthread_create(&threads[i], NULL, simulate_bot_connection, &bot_ids[i]);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < 5; i++) {
        pthread_join(threads[i], NULL);
    }
    
    sleep(1);
    
    // Test 3: Other animations
    printf("\n\nTEST 3: Other Animations\n");
    printf("------------------------\n");
    
    printf("Testing C2 splash...\n");
    show_c2_splash();
    
    printf("Testing spinner...\n");
    show_spinner("Scanning network", 1000);
    
    printf("Testing progress bar...\n");
    for (int i = 0; i <= 100; i += 10) {
        show_progress("Exploiting", i, 100);
        usleep(100000);
    }
    printf("\n");
    
    printf("Testing victory animation...\n");
    show_victory();
    
    printf("\n\n====================================\n");
    printf("   ALL TESTS COMPLETE!\n");
    printf("====================================\n");
    printf("\nVerify:\n");
    printf("  ✓ Lucky Star animation played smoothly\n");
    printf("  ✓ Only 1-2 replication animations showed (others skipped)\n");
    printf("  ✓ No garbled/corrupted output\n");
    printf("  ✓ All other animations worked\n\n");
    
    return 0;
}
