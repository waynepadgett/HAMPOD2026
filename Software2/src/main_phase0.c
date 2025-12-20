/**
 * main_phase0.c - Phase Zero Integration Test
 * 
 * Demonstrates all Software2 modules working together:
 * - Comm (Firmware pipe communication)
 * - Speech (non-blocking TTS queue)
 * - Keypad (key press/hold detection)
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "hampod_core.h"
#include "comm.h"
#include "speech.h"
#include "keypad.h"

static volatile int running = 1;

void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\nShutting down...\n");
        running = 0;
    }
}

/**
 * Step 5: Handle keypad events.
 * Called by keypad module when a key is pressed or held.
 */
void on_key_event(const KeyPressEvent* event) {
    if (event->key == '-') {
        return;  /* No key pressed, ignore */
    }
    
    printf("[Keypad] Key: '%c', Hold: %s\n", 
           event->key, 
           event->isHold ? "YES" : "NO");
    
    /* Steps 7-8: Speak the key */
    char speak_buffer[64];
    
    if (event->isHold) {
        /* Long press (held) */
        snprintf(speak_buffer, sizeof(speak_buffer), "You held %c", event->key);
        speech_say_text(speak_buffer);
    } else {
        /* Regular press */
        snprintf(speak_buffer, sizeof(speak_buffer), "You pressed %c", event->key);
        speech_say_text(speak_buffer);
    }
}

int main() {
    printf("=== HAMPOD2026 Phase Zero Integration Test ===\n");
    signal(SIGINT, signal_handler);
    
    /* Step 3: Initialize communication with Firmware */
    printf("Initializing communication...\n");
    if (comm_init() != HAMPOD_OK) {
        LOG_ERROR("Failed to connect to Firmware. Is firmware.elf running?");
        return 1;
    }
    printf("Connected to Firmware.\n");
    
    printf("Waiting for Firmware ready signal...\n");
    if (comm_wait_ready() != HAMPOD_OK) {
        LOG_ERROR("Firmware did not send ready signal");
        comm_close();
        return 1;
    }
    printf("Firmware ready!\n");
    
    /* Step 4: Initialize speech system */
    printf("Initializing speech system...\n");
    if (speech_init() != HAMPOD_OK) {
        LOG_ERROR("Failed to initialize speech system");
        comm_close();
        return 1;
    }
    printf("Speech system ready.\n");
    
    /* Announce startup */
    speech_say_text("Phase zero integration test ready. Press any key.");
    
    /* Step 6: Initialize keypad system */
    printf("Initializing keypad system...\n");
    if (keypad_init() != HAMPOD_OK) {
        LOG_ERROR("Failed to initialize keypad system");
        speech_shutdown();
        comm_close();
        return 1;
    }
    keypad_register_callback(on_key_event);
    printf("Keypad system ready.\n");
    
    /* Step 9: All systems ready */
    printf("\n=== All systems GO! ===\n");
    printf("Press keys on the keypad to hear them spoken.\n");
    printf("Hold a key for 1+ second to hear 'held' message.\n");
    printf("Press Ctrl+C to exit.\n\n");
    
    while (running) {
        usleep(100000);  // 100ms loop
    }
    
    /* Cleanup - Step 9: Orderly shutdown */
    printf("\nShutdown sequence:\n");
    printf("  - Stopping keypad...\n");
    keypad_shutdown();
    printf("  - Speaking goodbye...\n");
    speech_say_text("Goodbye");
    speech_wait_complete();  /* Wait for final speech before shutdown */
    printf("  - Stopping speech...\n");
    speech_shutdown();
    printf("  - Closing communication...\n");
    comm_close();
    printf("  - Done!\n");
    
    printf("Goodbye!\n");
    return 0;
}
