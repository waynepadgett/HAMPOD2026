/**
 * @file main.c
 * @brief HAMPOD2026 Software2 Main Entry Point
 * 
 * Integrates all modules for frequency, normal, and set mode operation:
 * - Config module for settings
 * - Comm module for Firmware communication
 * - Speech module for audio feedback
 * - Keypad module for input
 * - Radio module for Hamlib control
 * - Frequency mode for keypad frequency entry
 * - Normal mode for radio queries and status
 * - Set mode for adjusting radio parameters
 * 
 * Part of Phase 1/2/3: Frequency Mode, Normal Mode, Set Mode
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>

#include "hampod_core.h"
#include "config.h"
#include "comm.h"
#include "speech.h"
#include "keypad.h"
#include "radio.h"
#include "frequency_mode.h"
#include "normal_mode.h"
#include "set_mode.h"

// ============================================================================
// Signal Handling
// ============================================================================

static volatile bool g_running = true;

static void signal_handler(int sig) {
    (void)sig;
    printf("\nShutting down...\n");
    g_running = false;
}

// ============================================================================
// Keypad Callback
// ============================================================================

// Shift state for Set Mode (toggled by [A] key)
static bool g_shift_active = false;

static void on_keypress(const KeyPressEvent* kp) {
    DEBUG_PRINT("main: Key '%c' hold=%d shift=%d\n", kp->key, kp->isHold, kp->shiftAmount);
    
    // Capture shift state before processing (shift is one-shot: auto-clears after use)
    bool was_shifted = g_shift_active;
    
    // Route to set mode first (if active, it takes priority for ALL keys including [A])
    if (set_mode_is_active()) {
        if (set_mode_handle_key(kp->key, kp->isHold, was_shifted)) {
            // Auto-clear shift after a shifted key is consumed
            if (was_shifted) {
                g_shift_active = false;
            }
            return;
        }
    }
    
    // Handle [A] key for shift toggle (only when NOT in set mode)
    if (kp->key == 'A' && !kp->isHold) {
        g_shift_active = !g_shift_active;
        speech_say_text(g_shift_active ? "Shift" : "Shift off");
        return;
    }
    
    // [B] key enters Set Mode when not active
    if (kp->key == 'B' && !kp->isHold && !set_mode_is_active()) {
        set_mode_enter();
        // Clear shift when entering Set Mode
        if (was_shifted) {
            g_shift_active = false;
        }
        return;
    }
    
    // Route to frequency mode
    if (frequency_mode_handle_key(kp->key, kp->isHold)) {
        // Auto-clear shift after a key is consumed
        if (was_shifted) {
            g_shift_active = false;
        }
        return;
    }
    
    // Route to normal mode (pass shift state)
    if (normal_mode_handle_key(kp->key, kp->isHold, was_shifted)) {
        // Auto-clear shift after a shifted key is consumed
        if (was_shifted) {
            g_shift_active = false;
        }
        return;
    }
    
    // Key not consumed by any mode - announce it
    char text[32];
    if (kp->isHold) {
        snprintf(text, sizeof(text), "Held %c", kp->key);
    } else {
        snprintf(text, sizeof(text), "Pressed %c", kp->key);
    }
    speech_say_text(text);
    
    // Even unhandled keys clear shift
    if (was_shifted) {
        g_shift_active = false;
    }
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char *argv[]) {
    printf("=== HAMPOD2026 Frequency/Normal Mode ===\n\n");
    
    // Set up signal handler for clean shutdown
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Check for skip-radio mode (for testing without hardware)
    bool skip_radio = false;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == 'n') {
            skip_radio = true;
            printf("Running without radio (--no-radio mode)\n\n");
        }
    }
    
    // Initialize config
    printf("Initializing config...\n");
    if (config_init(NULL) != 0) {
        printf("WARNING: Config init failed, using defaults\n");
    }
    
    // Apply volume setting to USB audio (typically card 2)
    int volume = config_get_volume();
    printf("Setting volume to %d%%...\n", volume);
    char vol_cmd[256];
    // Try card 2 (USB2.0 Device) first, then card 4 (USB Audio CODEC)
    snprintf(vol_cmd, sizeof(vol_cmd), 
        "amixer -c 2 -q sset PCM %d%% 2>/dev/null || "
        "amixer -c 4 -q sset PCM %d%% 2>/dev/null || "
        "amixer -q sset PCM %d%% 2>/dev/null", 
        volume, volume, volume);
    system(vol_cmd);
    
    // Initialize comm (Firmware pipes)
    printf("Connecting to Firmware...\n");
    if (comm_init() != 0) {
        fprintf(stderr, "ERROR: Could not connect to Firmware\n");
        fprintf(stderr, "Is firmware.elf running?\n");
        config_cleanup();
        return 1;
    }
    
    // Wait for Firmware ready
    printf("Waiting for Firmware ready signal...\n");
    if (comm_wait_ready() != 0) {
        fprintf(stderr, "ERROR: Firmware not ready\n");
        comm_close();
        config_cleanup();
        return 1;
    }
    
    // Initialize speech
    printf("Initializing speech...\n");
    if (speech_init() != 0) {
        fprintf(stderr, "ERROR: Speech init failed\n");
        comm_close();
        config_cleanup();
        return 1;
    }
    
    // Initialize keypad
    printf("Initializing keypad...\n");
    if (keypad_init() != 0) {
        fprintf(stderr, "ERROR: Keypad init failed\n");
        speech_shutdown();
        comm_close();
        config_cleanup();
        return 1;
    }
    keypad_register_callback(on_keypress);
    
    // Initialize radio (optional)
    if (!skip_radio) {
        printf("Connecting to radio...\n");
        if (radio_init() != 0) {
            printf("WARNING: Could not connect to radio\n");
            printf("Frequency mode will work but cannot set radio\n");
        } else {
            printf("Radio connected!\n");
            
            // Start polling for VFO dial changes
            if (radio_start_polling(frequency_mode_on_radio_change) == 0) {
                printf("Radio polling started (1-second debounce)\n");
            }
        }
    }
    
    // Initialize frequency mode
    frequency_mode_init();
    
    // Initialize normal mode
    normal_mode_init();
    
    // Initialize set mode
    set_mode_init();
    
    // Announce startup
    printf("\nStartup complete. Normal mode active.\n");
    printf("Press [#] to enter frequency mode.\n");
    printf("Press Ctrl+C to exit.\n\n");
    speech_say_text("Ready");
    
    // Main loop - just keep running while keypad thread handles input
    while (g_running) {
        // Sleep to avoid busy-waiting
        struct timespec ts = {0, 100000000};  // 100ms
        nanosleep(&ts, NULL);
    }
    
    // Cleanup
    printf("\nCleaning up...\n");
    
    if (radio_is_connected()) {
        radio_stop_polling();
        radio_cleanup();
    }
    
    keypad_shutdown();
    speech_shutdown();
    comm_close();
    config_cleanup();
    
    printf("Goodbye!\n");
    return 0;
}
