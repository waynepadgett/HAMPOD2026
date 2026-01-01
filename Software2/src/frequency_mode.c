/**
 * @file frequency_mode.c
 * @brief Frequency Entry Mode implementation
 * 
 * Part of Phase 1: Frequency Mode Implementation
 */

#include "frequency_mode.h"
#include "radio.h"
#include "radio_queries.h"
#include "speech.h"
#include "normal_mode.h"
#include "hampod_core.h"
#include "comm.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// ============================================================================
// Module State
// ============================================================================

static FreqModeState g_state = FREQ_MODE_IDLE;
static VfoSelection g_selected_vfo = VFO_CURRENT;

// Frequency accumulator
#define MAX_FREQ_DIGITS 12
static char g_freq_buffer[MAX_FREQ_DIGITS + 1];
static int g_freq_len = 0;
static bool g_has_decimal = false;

// Inactivity timeout (seconds)
#define FREQ_MODE_TIMEOUT_SEC 10
static time_t g_last_activity = 0;

// Suppress polling announcement after user sets frequency
static bool g_suppress_next_poll = false;

// ============================================================================
// Internal Helpers
// ============================================================================

static const char* vfo_name(VfoSelection vfo) {
    switch (vfo) {
        case VFO_A:       return "VFO A";
        case VFO_B:       return "VFO B";
        case VFO_CURRENT: return "Current VFO";
        default:          return "VFO";
    }
}

static void clear_freq_buffer(void) {
    memset(g_freq_buffer, 0, sizeof(g_freq_buffer));
    g_freq_len = 0;
    g_has_decimal = false;
}

static void announce_digit(char digit) {
    char text[16];
    snprintf(text, sizeof(text), "%c", digit);
    speech_say_text(text);
}

static void announce_frequency(double freq_hz) {
    // Convert Hz to MHz and format for speech
    // Need 5 decimal places for 10 Hz resolution (e.g., 14.25000)
    double freq_mhz = freq_hz / 1000000.0;
    
    // Format as "14 point 2 5 0 0 0 megahertz" (spell out decimals)
    char text[128];
    int mhz_part = (int)freq_mhz;
    
    // Get 5 decimal places (100 Hz resolution = 5 digits after decimal)
    // e.g., 14.250000 MHz -> decimals = 25000
    int decimals = (int)((freq_mhz - mhz_part) * 100000 + 0.5);
    
    if (decimals == 0) {
        snprintf(text, sizeof(text), "%d megahertz", mhz_part);
    } else {
        // Spell out each decimal digit for clarity
        // e.g., 14.25000 -> "14 point 2 5 0 0 0 megahertz"
        char decimal_str[16];
        snprintf(decimal_str, sizeof(decimal_str), "%05d", decimals);
        
        // Build spoken string with spaces between digits
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

static double parse_frequency(void) {
    // Parse the accumulated buffer as a frequency in MHz
    // Returns frequency in Hz
    
    if (g_freq_len == 0) {
        return -1.0;
    }
    
    double freq_mhz;
    
    // If no decimal point was entered and we have 4-5 digits,
    // interpret as MHz with decimal before last 3 digits
    // e.g., "14025" -> 14.025, "7074" -> 7.074
    if (!g_has_decimal && g_freq_len >= 4 && g_freq_len <= 5) {
        // Insert decimal point before last 3 digits
        char formatted[MAX_FREQ_DIGITS + 2];
        int decimal_pos = g_freq_len - 3;  // Position to insert decimal
        strncpy(formatted, g_freq_buffer, decimal_pos);
        formatted[decimal_pos] = '.';
        strcpy(formatted + decimal_pos + 1, g_freq_buffer + decimal_pos);
        freq_mhz = atof(formatted);
        DEBUG_PRINT("parse_frequency: Auto-decimal '%s' -> '%s' = %.3f MHz\n", 
                    g_freq_buffer, formatted, freq_mhz);
    } else {
        freq_mhz = atof(g_freq_buffer);
    }
    
    // Validate reasonable range (100 kHz to 500 MHz)
    if (freq_mhz < 0.1 || freq_mhz > 500.0) {
        return -1.0;
    }
    
    return freq_mhz * 1000000.0;  // Convert to Hz
}

static void submit_frequency(void) {
    double freq_hz = parse_frequency();
    
    if (freq_hz < 0) {
        if (config_get_key_beep_enabled()) {
            comm_play_beep(COMM_BEEP_ERROR);
        }
        speech_say_text("Invalid frequency");
        clear_freq_buffer();
        g_state = FREQ_MODE_IDLE;
        return;
    }
    
    DEBUG_PRINT("submit_frequency: %.3f MHz to %s\n", freq_hz / 1000000.0, vfo_name(g_selected_vfo));
    
    // Suppress the polling announcement for this frequency change
    g_suppress_next_poll = true;
    
    // Switch to selected VFO first (if not current)
    if (g_selected_vfo != VFO_CURRENT) {
        RadioVfo target_vfo = (g_selected_vfo == VFO_A) ? RADIO_VFO_A : RADIO_VFO_B;
        if (radio_set_vfo(target_vfo) != 0) {
            if (config_get_key_beep_enabled()) {
                comm_play_beep(COMM_BEEP_ERROR);
            }
            speech_say_text("VFO switch failed");
            clear_freq_buffer();
            g_state = FREQ_MODE_IDLE;
            return;
        }
    }
    
    // Set frequency on radio
    if (radio_set_frequency(freq_hz) == 0) {
        // Just announce the frequency (no separate "Frequency set" message)
        announce_frequency(freq_hz);
    } else {
        if (config_get_key_beep_enabled()) {
            comm_play_beep(COMM_BEEP_ERROR);
        }
        speech_say_text("Failed to set frequency");
    }
    
    clear_freq_buffer();
    g_state = FREQ_MODE_IDLE;
}

// ============================================================================
// Initialization
// ============================================================================

void frequency_mode_init(void) {
    g_state = FREQ_MODE_IDLE;
    g_selected_vfo = VFO_CURRENT;
    clear_freq_buffer();
    DEBUG_PRINT("frequency_mode_init: Initialized\n");
}

// ============================================================================
// Key Handling
// ============================================================================

bool frequency_mode_handle_key(char key, bool is_hold) {
    (void)is_hold;  // Hold not used differently in this mode for now
    
    DEBUG_PRINT("frequency_mode_handle_key: key='%c' state=%d\n", key, g_state);
    
    // Check for timeout if in frequency mode
    if (g_state != FREQ_MODE_IDLE) {
        time_t now = time(NULL);
        if (now - g_last_activity > FREQ_MODE_TIMEOUT_SEC) {
            DEBUG_PRINT("frequency_mode: Timeout - cancelling\n");
            clear_freq_buffer();
            g_state = FREQ_MODE_IDLE;
            speech_say_text("Timeout");
            return true;  // Consume key, announce timeout
        }
        g_last_activity = now;  // Update activity timestamp
    }
    
    switch (g_state) {
        case FREQ_MODE_IDLE:
            // Not in frequency mode - check for entry key
            if (key == '#') {
                // Enter frequency mode
                g_state = FREQ_MODE_SELECT_VFO;
                g_last_activity = time(NULL);  // Start timeout clock
                speech_say_text("Frequency Mode");
                return true;
            }
            return false;  // Key not consumed
            
        case FREQ_MODE_SELECT_VFO:
            if (key == '#') {
                // Cycle VFO selection
                g_selected_vfo = (g_selected_vfo + 1) % 3;
                speech_say_text(vfo_name(g_selected_vfo));
                return true;
            }
            if (isdigit(key)) {
                // Start entering digits
                g_state = FREQ_MODE_ENTERING;
                g_freq_buffer[g_freq_len++] = key;
                g_freq_buffer[g_freq_len] = '\0';
                announce_digit(key);
                return true;
            }
            if (key == '*') {
                // Cancel
                clear_freq_buffer();
                g_state = FREQ_MODE_IDLE;
                speech_say_text("Cancelled");
                return true;
            }
            if (key == 'D') {
                // Clear/exit
                clear_freq_buffer();
                g_state = FREQ_MODE_IDLE;
                speech_say_text("Cancelled");
                return true;
            }
            return true;  // Consume but ignore other keys
            
        case FREQ_MODE_ENTERING:
            if (isdigit(key)) {
                // Accumulate digit
                if (g_freq_len < MAX_FREQ_DIGITS) {
                    g_freq_buffer[g_freq_len++] = key;
                    g_freq_buffer[g_freq_len] = '\0';
                    announce_digit(key);
                }
                return true;
            }
            if (key == '*') {
                if (!g_has_decimal) {
                    // Insert decimal point
                    if (g_freq_len < MAX_FREQ_DIGITS) {
                        g_freq_buffer[g_freq_len++] = '.';
                        g_freq_buffer[g_freq_len] = '\0';
                        g_has_decimal = true;
                        speech_say_text("point");
                    }
                } else {
                    // Second * cancels
                    clear_freq_buffer();
                    g_state = FREQ_MODE_IDLE;
                    speech_say_text("Cancelled");
                }
                return true;
            }
            if (key == '#') {
                // Submit frequency
                submit_frequency();
                return true;
            }
            if (key == 'D') {
                // Clear/exit
                clear_freq_buffer();
                g_state = FREQ_MODE_IDLE;
                speech_say_text("Cancelled");
                return true;
            }
            return true;  // Consume but ignore other keys
    }
    
    return false;
}

bool frequency_mode_is_active(void) {
    return g_state != FREQ_MODE_IDLE;
}

FreqModeState frequency_mode_get_state(void) {
    return g_state;
}

void frequency_mode_cancel(void) {
    if (g_state != FREQ_MODE_IDLE) {
        clear_freq_buffer();
        g_state = FREQ_MODE_IDLE;
        DEBUG_PRINT("frequency_mode_cancel: Cancelled\n");
    }
}

// ============================================================================
// Radio Polling Integration
// ============================================================================

void frequency_mode_on_radio_change(double new_freq) {
    // Announce frequency change from VFO dial
    // Only announce if NOT actively entering a frequency
    // Also suppress if we just set the frequency ourselves
    // And respect verbosity setting
    if (g_state == FREQ_MODE_IDLE) {
        if (g_suppress_next_poll) {
            DEBUG_PRINT("frequency_mode_on_radio_change: Suppressed (we just set it)\n");
            g_suppress_next_poll = false;
            return;
        }
        
        // Check if announcements are enabled (from normal_mode)
        if (!normal_mode_get_verbosity()) {
            DEBUG_PRINT("frequency_mode_on_radio_change: Suppressed (verbosity off)\n");
            return;
        }
        
        DEBUG_PRINT("frequency_mode_on_radio_change: %.3f MHz\n", new_freq / 1000000.0);
        announce_frequency(new_freq);
    }
}

void frequency_mode_suppress_next_poll(void) {
    g_suppress_next_poll = true;
    DEBUG_PRINT("frequency_mode_suppress_next_poll: armed\n");
}
