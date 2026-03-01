/**
 * @file config_mode.h
 * @brief Configuration Mode API for adjusting HAMPOD system parameters
 *
 * Config Mode allows modifying system-level settings (Volume, Speech Speed,
 * Keypad Layout, etc.)
 */

#ifndef CONFIG_MODE_H
#define CONFIG_MODE_H

#include <stdbool.h>

// ============================================================================
// State Definitions
// ============================================================================

typedef enum {
  CONFIG_MODE_OFF,     // Not in Config Mode (Normal Mode active)
  CONFIG_MODE_BROWSING // In Config Mode, navigating system params
} ConfigModeState;

typedef enum {
  CONFIG_PARAM_VOLUME,
  CONFIG_PARAM_SPEECH_SPEED,
  CONFIG_PARAM_LAYOUT,
  CONFIG_PARAM_SHUTDOWN,
  CONFIG_PARAM_COUNT // Must be last
} ConfigModeParameter;

// ============================================================================
// Initialization
// ============================================================================

/**
 * @brief Initialize Config Mode system
 *
 * Call once at startup.
 */
void config_mode_init(void);

// ============================================================================
// State Queries
// ============================================================================

/**
 * @brief Check if Config Mode is active
 * @return true if in Config Mode (any state except CONFIG_MODE_OFF)
 */
bool config_mode_is_active(void);

/**
 * @brief Get current Config Mode state
 * @return Current ConfigModeState
 */
ConfigModeState config_mode_get_state(void);

/**
 * @brief Get current parameter being edited
 * @return Current ConfigModeParameter
 */
ConfigModeParameter config_mode_get_parameter(void);

// ============================================================================
// Key Handling
// ============================================================================

/**
 * @brief Handle a keypress when Config Mode is active
 *
 * @param key The key character ('0'-'9', '#', '*', 'A'-'D')
 * @param is_hold true if key was held (>500ms)
 * @return true if key was consumed by Config Mode
 */
bool config_mode_handle_key(char key, bool is_hold);

// ============================================================================
// Mode Entry/Exit
// ============================================================================

/**
 * @brief Enter Config Mode
 *
 * Transitions from CONFIG_MODE_OFF to CONFIG_MODE_BROWSING.
 * Announces "Configuration Mode".
 */
void config_mode_enter(void);

/**
 * @brief Exit Config Mode and save changes
 *
 * Announces "Configuration saved".
 */
void config_mode_exit_save(void);

/**
 * @brief Exit Config Mode and discard changes
 *
 * Undoes all changes made during this session.
 * Announces "Configuration cancelled".
 */
void config_mode_exit_discard(void);

#endif // CONFIG_MODE_H
