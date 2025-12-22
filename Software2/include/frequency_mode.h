/**
 * @file frequency_mode.h
 * @brief Frequency Entry Mode for HAMPOD
 * 
 * Implements keypad frequency entry per ICOMReader manual:
 * - [#] to enter mode and cycle VFO selection (A/B/Current)
 * - Digits 0-9 to accumulate frequency digits
 * - [*] for decimal point, second [*] cancels
 * - [#] after digits submits frequency
 * - [D] clears and exits mode
 * 
 * Part of Phase 1: Frequency Mode Implementation
 */

#ifndef FREQUENCY_MODE_H
#define FREQUENCY_MODE_H

#include <stdbool.h>

// ============================================================================
// Mode States
// ============================================================================

typedef enum {
    FREQ_MODE_IDLE,        // Not in frequency mode
    FREQ_MODE_SELECT_VFO,  // [#] pressed, waiting for VFO selection or digits
    FREQ_MODE_ENTERING,    // Accumulating digits
} FreqModeState;

typedef enum {
    VFO_CURRENT,
    VFO_A,
    VFO_B
} VfoSelection;

// ============================================================================
// Initialization
// ============================================================================

/**
 * @brief Initialize frequency mode system
 * 
 * Call once at startup to set up internal state.
 */
void frequency_mode_init(void);

// ============================================================================
// Key Handling
// ============================================================================

/**
 * @brief Handle a keypress in frequency mode
 * 
 * Main entry point for key events. Routes to appropriate handlers
 * based on current state.
 * 
 * @param key The key character ('0'-'9', '#', '*', 'A'-'D')
 * @param is_hold true if key was held (>500ms)
 * @return true if key was consumed by frequency mode, false if not in mode
 */
bool frequency_mode_handle_key(char key, bool is_hold);

/**
 * @brief Check if frequency mode is active
 * @return true if in frequency entry mode
 */
bool frequency_mode_is_active(void);

/**
 * @brief Get current mode state
 * @return Current FreqModeState
 */
FreqModeState frequency_mode_get_state(void);

/**
 * @brief Force exit from frequency mode
 * 
 * Clears accumulated input and returns to IDLE state.
 */
void frequency_mode_cancel(void);

// ============================================================================
// Radio Polling Integration
// ============================================================================

/**
 * @brief Callback for radio frequency changes
 * 
 * Should be registered with radio_start_polling() to receive
 * frequency change notifications from VFO dial movement.
 * 
 * @param new_freq The new frequency in Hz
 */
void frequency_mode_on_radio_change(double new_freq);

#endif // FREQUENCY_MODE_H
