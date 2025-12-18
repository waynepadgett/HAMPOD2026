/**
 * test_comm_read.c - Test Pipe Communication (Reading)
 * 
 * Verification for Phase 0 Step 1.1: Comm Reader
 * 
 * This test:
 * 1. Connects to Firmware via pipes
 * 2. Polls for keypad input
 * 3. Prints each key pressed
 * 
 * Prerequisites:
 * - Firmware must be running: cd ../Firmware && ./firmware.elf
 * 
 * Usage:
 *   make tests
 *   ./bin/test_comm_read
 *   (press keys on USB keypad to see them printed)
 *   (Ctrl+C to exit)
 * 
 * Expected output:
 *   [INFO] Waiting for keypad input... (press keys, Ctrl+C to exit)
 *   [INFO] Key pressed: 5
 *   [INFO] Key pressed: A
 *   ...
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "hampod_core.h"
#include "comm.h"

static volatile int running = 1;

void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

int main() {
    printf("=== Phase 0 Step 1.1: Comm Reader Test ===\n\n");
    
    // Set up Ctrl+C handler
    signal(SIGINT, signal_handler);
    
    // Initialize communication
    if (comm_init() != HAMPOD_OK) {
        LOG_ERROR("Failed to connect to Firmware");
        LOG_ERROR("Make sure Firmware is running: cd ../Firmware && ./firmware.elf");
        return 1;
    }
    
    // Wait for Firmware ready signal
    if (comm_wait_ready() != HAMPOD_OK) {
        LOG_ERROR("Firmware not ready");
        comm_close();
        return 1;
    }
    
    LOG_INFO("Connected to Firmware!");
    LOG_INFO("Waiting for keypad input... (press keys, Ctrl+C to exit)");
    printf("\n");
    
    int key_count = 0;
    
    while (running) {
        char key;
        
        if (comm_read_keypad(&key) != HAMPOD_OK) {
            LOG_ERROR("Failed to read keypad");
            break;
        }
        
        // Skip "no key" events
        if (key == '-' || key == 0xFF) {
            usleep(50000);  // 50ms polling interval
            continue;
        }
        
        key_count++;
        LOG_INFO("Key pressed: %c (0x%02X) [count=%d]", key, (unsigned char)key, key_count);
    }
    
    printf("\n");
    LOG_INFO("Exiting... (received %d key presses)", key_count);
    
    comm_close();
    
    return 0;
}
