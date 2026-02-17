/**
 * test_keypad_events.c - Test Keypad Input with Hold Detection
 * 
 * Verification for Phase 0 Step 3.1: Keypad Events
 * 
 * This test:
 * 1. Initializes comm and keypad systems
 * 2. Registers a callback to receive key events
 * 3. Prints each key press/hold event
 * 
 * Prerequisites:
 * - Firmware must be running: cd ../Firmware && ./firmware.elf
 * 
 * Usage:
 *   make tests
 *   ./bin/test_keypad_events
 *   (press keys on USB keypad)
 *   (hold a key for >500ms to trigger a hold event)
 *   (Ctrl+C to exit)
 * 
 * Expected output:
 *   [KEY] Press: 5
 *   [KEY] Hold: 3
 *   ...
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#include "hampod_core.h"
#include "comm.h"
#include "keypad.h"
#include "speech.h"

static volatile int running = 1;
static int press_count = 0;
static int hold_count = 0;

void signal_handler(int sig) {
    (void)sig;
    running = 0;
}

void on_key_event(const KeyPressEvent* event) {
    if (event->isHold) {
        hold_count++;
        printf("[KEY] Hold: %c (total holds: %d)\n", event->key, hold_count);
        
        // NOTE: Speech disabled during keypad testing
        // Both use the same pipe and responses get interleaved
        // TODO: Fix with proper request/response matching
        // char msg[64];
        // snprintf(msg, sizeof(msg), "Held %c", event->key);
        // speech_say_text(msg);
        
    } else {
        press_count++;
        printf("[KEY] Press: %c (total presses: %d)\n", event->key, press_count);
        
        // NOTE: Speech disabled during keypad testing
        // char msg[64];
        // snprintf(msg, sizeof(msg), "Pressed %c", event->key);
        // speech_say_text(msg);
    }
}

int main() {
    printf("=== Phase 0 Step 3.1: Keypad Events Test ===\n\n");
    
    // Set up Ctrl+C handler
    signal(SIGINT, signal_handler);
    
    // Initialize communication
    LOG_INFO("Initializing communication...");
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
    
    // NOTE: Speech system NOT initialized in this test
    // Keypad and speech both use same pipe, responses get interleaved
    // Integration test (Step 0.9) will properly coordinate them
    
    // Initialize keypad system
    LOG_INFO("Initializing keypad system...");
    keypad_register_callback(on_key_event);
    
    if (keypad_init() != HAMPOD_OK) {
        LOG_ERROR("Failed to initialize keypad system");
        comm_close();
        return 1;
    }
    
    printf("\n");
    LOG_INFO("=== Ready for input ===");
    LOG_INFO("Press keys on the USB keypad");
    LOG_INFO("Hold a key for >500ms to trigger a hold event");
    LOG_INFO("Press Ctrl+C to exit");
    printf("\n");
    
    // Main loop - just wait for Ctrl+C
    while (running) {
        sleep(1);
    }
    
    printf("\n");
    LOG_INFO("=== Test Summary ===");
    LOG_INFO("Total key presses: %d", press_count);
    LOG_INFO("Total key holds: %d", hold_count);
    
    // Shutdown
    keypad_shutdown();
    comm_close();
    
    return 0;
}
