/**
 * @file normal_mode.h
 * @brief Normal Mode - primary operating mode for radio queries
 * 
 * Normal Mode is the default operating mode where the user can:
 * - Query radio status (frequency, mode, VFO, meters)
 * - Receive automatic announcements of changes
 * - Toggle verbosity settings
 * 
 * Part of Phase 2: Normal Mode Implementation
 */

#ifndef NORMAL_MODE_H
#define NORMAL_MODE_H

#include <stdbool.h>

// ============================================================================
// Initialization
// ============================================================================

/**
 * @brief Initialize Normal Mode
 * 
 * Sets up internal state. Should be called once at startup.
 */
void normal_mode_init(void);

// ============================================================================
// Key Handling
// ============================================================================

/**
 * @brief Handle a key press in Normal Mode
 * 
 * Processes query keys ([0]-[9], [*], [#], [A]-[D]) and performs
 * appropriate radio queries or mode changes.
 * 
 * @param key The key character ('0'-'9', 'A'-'D', '*', '#')
 * @param is_hold true if this is a held key (long press)
 * @return true if the key was consumed, false to pass to next handler
 */
bool normal_mode_handle_key(char key, bool is_hold);

// ============================================================================
// Verbosity Control
// ============================================================================

/**
 * @brief Set automatic announcement verbosity
 * 
 * When enabled, frequency/mode changes are automatically announced.
 * When disabled, only manual queries produce announcements.
 * 
 * @param enabled true to enable automatic announcements
 */
void normal_mode_set_verbosity(bool enabled);

/**
 * @brief Get current verbosity setting
 * 
 * @return true if automatic announcements are enabled
 */
bool normal_mode_get_verbosity(void);

// ============================================================================
// Radio Change Callbacks
// ============================================================================

/**
 * @brief Called when radio mode changes
 * 
 * This should be registered with the polling system.
 * Announces the new mode if verbosity is enabled.
 * 
 * @param new_mode The new mode string (e.g., "USB")
 */
void normal_mode_on_mode_change(const char* new_mode);

/**
 * @brief Called when VFO changes
 * 
 * @param new_vfo The new VFO (0=A, 1=B, 2=Current)
 */
void normal_mode_on_vfo_change(int new_vfo);

#endif // NORMAL_MODE_H
