/**
 * @file set_mode.h
 * @brief Set Mode API for adjusting radio parameters
 * 
 * Part of Phase 3: Set Mode Implementation
 * 
 * Set Mode is an overlay state that modifies how parameter keys behave.
 * When active, keys that normally query parameters instead allow modification.
 */

#ifndef SET_MODE_H
#define SET_MODE_H

#include <stdbool.h>

// ============================================================================
// State Definitions
// ============================================================================

typedef enum {
    SET_MODE_OFF,       // Not in Set Mode (Normal Mode active)
    SET_MODE_IDLE,      // In Set Mode, awaiting parameter selection
    SET_MODE_EDITING,   // Editing a specific parameter value
    SET_MODE_CONFIRM    // Value entered, awaiting confirmation
} SetModeState;

typedef enum {
    SET_PARAM_NONE,
    SET_PARAM_POWER,
    SET_PARAM_MIC_GAIN,
    SET_PARAM_COMPRESSION,
    SET_PARAM_NB,
    SET_PARAM_NR,
    SET_PARAM_AGC,
    SET_PARAM_PREAMP,
    SET_PARAM_ATTENUATION,
    SET_PARAM_MODE,
    SET_PARAM_TUNING_STEP,
    SET_PARAM_VOX,
    SET_PARAM_FILTER_NUMBER,
    SET_PARAM_KEYER_SPEED  
} SetModeParameter;

// ============================================================================
// Initialization
// ============================================================================

/**
 * @brief Initialize Set Mode system
 * 
 * Call once at startup.
 */
void set_mode_init(void);

// ============================================================================
// State Queries
// ============================================================================

/**
 * @brief Check if Set Mode is active
 * @return true if in Set Mode (any state except SET_MODE_OFF)
 */
bool set_mode_is_active(void);

/**
 * @brief Get current Set Mode state
 * @return Current SetModeState
 */
SetModeState set_mode_get_state(void);

/**
 * @brief Get current parameter being edited
 * @return Current SetModeParameter, or SET_PARAM_NONE
 */
SetModeParameter set_mode_get_parameter(void);

// ============================================================================
// Key Handling
// ============================================================================

/**
 * @brief Handle a keypress when Set Mode is active
 * 
 * @param key The key character ('0'-'9', '#', '*', 'A'-'D')
 * @param is_hold true if key was held (>500ms)
 * @param is_shifted true if shift mode is active ([A] was pressed)
 * @return true if key was consumed by Set Mode
 */
bool set_mode_handle_key(char key, bool is_hold, bool is_shifted);

// ============================================================================
// Mode Entry/Exit
// ============================================================================

/**
 * @brief Enter Set Mode
 * 
 * Transitions from SET_MODE_OFF to SET_MODE_IDLE.
 * Announces "Set Mode".
 */
void set_mode_enter(void);

/**
 * @brief Exit Set Mode
 * 
 * Clears any pending value and returns to SET_MODE_OFF.
 * Announces "Set Mode Off".
 */
void set_mode_exit(void);

/**
 * @brief Cancel current parameter editing but stay in Set Mode
 * 
 * Returns to SET_MODE_IDLE without applying changes.
 */
void set_mode_cancel_edit(void);

// ============================================================================
// Value Entry
// ============================================================================

/**
 * @brief Get the currently accumulated value string
 * @return Pointer to value buffer (read-only)
 */
const char* set_mode_get_value_buffer(void);

/**
 * @brief Clear the value buffer
 */
void set_mode_clear_value(void);


#endif // SET_MODE_H
