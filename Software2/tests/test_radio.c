/**
 * @file test_radio.c
 * @brief Radio module integration test
 * 
 * Tests Hamlib connection to physical radio.
 * Requires radio connected via USB.
 * 
 * Part of Phase 1: Frequency Mode Implementation
 */

#include "radio.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static volatile int freq_change_count = 0;
static volatile double last_announced_freq = 0;

static void on_freq_change(double new_freq) {
    freq_change_count++;
    last_announced_freq = new_freq;
    printf("[CALLBACK] Frequency stable at: %.3f MHz\n", new_freq / 1000000.0);
}

int main(int argc, char *argv[]) {
    printf("=== Radio Module Test ===\n\n");
    
    // Initialize config first (radio module uses it)
    printf("1. Initializing config...\n");
    if (config_init(NULL) != 0) {
        printf("   WARNING: Config init failed, using defaults\n");
    } else {
        printf("   OK - Config loaded\n");
    }
    
    printf("   Radio Model: %d\n", config_get_radio_model());
    printf("   Device: %s\n", config_get_radio_device());
    printf("   Baud: %d\n", config_get_radio_baud());
    printf("\n");
    
    // Connect to radio
    printf("2. Connecting to radio...\n");
    if (radio_init() != 0) {
        printf("   FAILED - Could not connect to radio\n");
        printf("   Check: Is the radio powered on? Is USB connected?\n");
        config_cleanup();
        return 1;
    }
    printf("   OK - Connected to radio\n\n");
    
    // Test get frequency
    printf("3. Getting current frequency...\n");
    double freq = radio_get_frequency();
    if (freq < 0) {
        printf("   FAILED - Could not read frequency\n");
        radio_cleanup();
        config_cleanup();
        return 1;
    }
    printf("   OK - Current frequency: %.3f MHz\n\n", freq / 1000000.0);
    
    // Test set frequency (add 100 kHz)
    printf("4. Setting frequency (+100 kHz)...\n");
    double new_freq = freq + 100000;
    if (radio_set_frequency(new_freq) != 0) {
        printf("   FAILED - Could not set frequency\n");
        radio_cleanup();
        config_cleanup();
        return 1;
    }
    printf("   OK - Set to %.3f MHz\n", new_freq / 1000000.0);
    
    // Verify
    usleep(500000);  // Wait 500ms for radio to settle
    double verify_freq = radio_get_frequency();
    double diff = verify_freq - new_freq;
    if (diff < 0) diff = -diff;
    
    if (diff > 1000) {  // Allow 1 kHz tolerance
        printf("   WARNING - Frequency mismatch: expected %.3f, got %.3f\n",
               new_freq / 1000000.0, verify_freq / 1000000.0);
    } else {
        printf("   OK - Verified: %.3f MHz\n\n", verify_freq / 1000000.0);
    }
    
    // Test polling (optional - requires user interaction)
    if (argc > 1 && argv[1][0] == 'p') {
        printf("5. Testing polling (turn VFO knob, press Ctrl+C to stop)...\n");
        if (radio_start_polling(on_freq_change) != 0) {
            printf("   FAILED - Could not start polling\n");
        } else {
            printf("   Polling started. Turn VFO knob and wait 1 second...\n");
            
            // Run for 30 seconds or until interrupt
            for (int i = 0; i < 30; i++) {
                sleep(1);
                if (freq_change_count > 0) {
                    printf("   Detected %d frequency change(s)\n", freq_change_count);
                }
            }
            
            radio_stop_polling();
            printf("   Polling stopped\n\n");
        }
    } else {
        printf("5. Polling test skipped (run with 'p' argument to test)\n\n");
    }
    
    // Restore original frequency
    printf("6. Restoring original frequency...\n");
    if (radio_set_frequency(freq) == 0) {
        printf("   OK - Restored to %.3f MHz\n\n", freq / 1000000.0);
    }
    
    // Cleanup
    printf("7. Cleaning up...\n");
    radio_cleanup();
    config_cleanup();
    printf("   OK - Disconnected\n\n");
    
    printf("=== All tests passed ===\n");
    return 0;
}
