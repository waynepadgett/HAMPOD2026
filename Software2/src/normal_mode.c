/**
 * @file normal_mode.c
 * @brief Normal Mode implementation
 * 
 * Part of Phase 2: Normal Mode Implementation
 */

#include "normal_mode.h"
#include "radio.h"
#include "radio_queries.h"
#include "speech.h"
#include "frequency_mode.h"
#include "hampod_core.h"

#include <stdio.h>
#include <string.h>

// ============================================================================
// Module State
// ============================================================================

static bool g_verbosity_enabled = true;  // Auto-announcements on by default

// ============================================================================
// Internal Helpers
// ============================================================================

/**
 * @brief Announce the current frequency as speech
 */
static void announce_frequency(void) {
    double freq_hz = radio_get_frequency();
    if (freq_hz < 0) {
        speech_say_text("Frequency not available");
        return;
    }
    
    // Convert Hz to MHz and format
    double freq_mhz = freq_hz / 1000000.0;
    int mhz_part = (int)freq_mhz;
    
    // Get 5 decimal places for 10 Hz resolution
    int decimals = (int)((freq_mhz - mhz_part) * 100000 + 0.5);
    
    char text[128];
    if (decimals == 0) {
        snprintf(text, sizeof(text), "%d megahertz", mhz_part);
    } else {
        // Spell out each decimal digit for clarity
        char decimal_str[16];
        snprintf(decimal_str, sizeof(decimal_str), "%05d", decimals);
        
        char spoken_decimals[32] = "";
        for (int i = 0; i < 5; i++) {
            char digit[4];
            snprintf(digit, sizeof(digit), "%c ", decimal_str[i]);
            strcat(spoken_decimals, digit);
        }
        
        snprintf(text, sizeof(text), "%d point %s megahertz", mhz_part, spoken_decimals);
    }
    
    speech_say_text(text);
}

/**
 * @brief Announce S-meter reading
 */
static void announce_smeter(void) {
    char buffer[32];
    const char* reading = radio_get_smeter_string(buffer, sizeof(buffer));
    speech_say_text(reading);
}

/**
 * @brief Announce power meter reading
 */
static void announce_power_meter(void) {
    char buffer[32];
    const char* reading = radio_get_power_string(buffer, sizeof(buffer));
    speech_say_text(reading);
}

// ============================================================================
// Initialization
// ============================================================================

void normal_mode_init(void) {
    g_verbosity_enabled = true;
    DEBUG_PRINT("normal_mode_init: Initialized\n");
}

// ============================================================================
// Key Handling
// ============================================================================

bool normal_mode_handle_key(char key, bool is_hold) {
    DEBUG_PRINT("normal_mode_handle_key: key='%c' hold=%d\n", key, is_hold);
    
    // [1] - VFO selection
    if (key == '1') {
        if (!is_hold) {
            // Select VFO A
            if (radio_set_vfo(RADIO_VFO_A) == 0) {
                speech_say_text("VFO A");
                announce_frequency();
            } else {
                speech_say_text("VFO A not available");
            }
        } else {
            // Select VFO B
            if (radio_set_vfo(RADIO_VFO_B) == 0) {
                speech_say_text("VFO B");
                announce_frequency();
            } else {
                speech_say_text("VFO B not available");
            }
        }
        return true;
    }
    
    // [2] - Announce current frequency
    if (key == '2' && !is_hold) {
        announce_frequency();
        return true;
    }
    
    // [0] - Announce current mode
    if (key == '0' && !is_hold) {
        const char* mode = radio_get_mode_string();
        speech_say_text(mode);
        return true;
    }
    
    // [*] - S-meter (press) / Power meter (hold)
    if (key == '*') {
        if (!is_hold) {
            announce_smeter();
        } else {
            announce_power_meter();
        }
        return true;
    }
    
    // [C] - Toggle verbosity (press) / Config mode entry (hold, not implemented)
    if (key == 'C' && !is_hold) {
        g_verbosity_enabled = !g_verbosity_enabled;
        if (g_verbosity_enabled) {
            speech_say_text("Announcements on");
        } else {
            speech_say_text("Announcements off");
        }
        return true;
    }
    
    // Key not handled by normal mode
    return false;
}

// ============================================================================
// Verbosity Control
// ============================================================================

void normal_mode_set_verbosity(bool enabled) {
    g_verbosity_enabled = enabled;
    DEBUG_PRINT("normal_mode_set_verbosity: %s\n", enabled ? "on" : "off");
}

bool normal_mode_get_verbosity(void) {
    return g_verbosity_enabled;
}

// ============================================================================
// Radio Change Callbacks
// ============================================================================

void normal_mode_on_mode_change(const char* new_mode) {
    if (!g_verbosity_enabled) {
        return;
    }
    
    // Don't announce if frequency mode is active
    if (frequency_mode_is_active()) {
        return;
    }
    
    DEBUG_PRINT("normal_mode_on_mode_change: %s\n", new_mode);
    speech_say_text(new_mode);
}

void normal_mode_on_vfo_change(int new_vfo) {
    if (!g_verbosity_enabled) {
        return;
    }
    
    // Don't announce if frequency mode is active
    if (frequency_mode_is_active()) {
        return;
    }
    
    DEBUG_PRINT("normal_mode_on_vfo_change: %d\n", new_vfo);
    const char* vfo_name = radio_get_vfo_string();
    speech_say_text(vfo_name);
}
