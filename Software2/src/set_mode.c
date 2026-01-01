/**
 * @file set_mode.c
 * @brief Set Mode implementation for adjusting radio parameters
 * 
 * Part of Phase 3: Set Mode Implementation
 */

#include "set_mode.h"
#include "radio_setters.h"
#include "radio_queries.h"
#include "speech.h"
#include "hampod_core.h"
#include "comm.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ============================================================================
// Module State
// ============================================================================

static SetModeState g_state = SET_MODE_OFF;
static SetModeParameter g_current_param = SET_PARAM_NONE;

// Value accumulator
#define MAX_VALUE_DIGITS 8
static char g_value_buffer[MAX_VALUE_DIGITS + 1];
static int g_value_len = 0;

// ============================================================================
// Internal Helpers
// ============================================================================

static void clear_value_buffer(void) {
    g_value_buffer[0] = '\0';
    g_value_len = 0;
}

static void add_digit(char digit) {
    if (g_value_len < MAX_VALUE_DIGITS) {
        g_value_buffer[g_value_len++] = digit;
        g_value_buffer[g_value_len] = '\0';
    }
}

static int get_value_as_int(void) {
    if (g_value_len == 0) {
        return -1;
    }
    return atoi(g_value_buffer);
}

static const char* param_name(SetModeParameter param) {
    switch (param) {
        case SET_PARAM_POWER:       return "Power";
        case SET_PARAM_MIC_GAIN:    return "Mic Gain";
        case SET_PARAM_COMPRESSION: return "Compression";
        case SET_PARAM_NB:          return "Noise Blanker";
        case SET_PARAM_NR:          return "Noise Reduction";
        case SET_PARAM_AGC:         return "A G C";
        case SET_PARAM_PREAMP:      return "Pre Amp";
        case SET_PARAM_ATTENUATION: return "Attenuation";
        case SET_PARAM_MODE:        return "Mode";
        default:                    return "Unknown";
    }
}

static void announce_current_value(SetModeParameter param) {
    char buffer[64];
    int value;
    
    switch (param) {
        case SET_PARAM_POWER:
            value = radio_get_power();
            if (value >= 0) {
                snprintf(buffer, sizeof(buffer), "Power %d percent", value);
            } else {
                snprintf(buffer, sizeof(buffer), "Power not available");
            }
            break;
            
        case SET_PARAM_MIC_GAIN:
            value = radio_get_mic_gain();
            if (value >= 0) {
                snprintf(buffer, sizeof(buffer), "Mic gain %d percent", value);
            } else {
                snprintf(buffer, sizeof(buffer), "Mic gain not available");
            }
            break;
            
        case SET_PARAM_COMPRESSION:
            value = radio_get_compression();
            if (value >= 0) {
                bool enabled = radio_get_compression_enabled();
                snprintf(buffer, sizeof(buffer), "Compression %s, level %d", 
                         enabled ? "on" : "off", value);
            } else {
                snprintf(buffer, sizeof(buffer), "Compression not available");
            }
            break;
            
        case SET_PARAM_NB:
            {
                bool enabled = radio_get_nb_enabled();
                int level = radio_get_nb_level();
                snprintf(buffer, sizeof(buffer), "Noise blanker %s, level %d", 
                         enabled ? "on" : "off", level >= 0 ? level : 0);
            }
            break;
            
        case SET_PARAM_NR:
            {
                bool enabled = radio_get_nr_enabled();
                int level = radio_get_nr_level();
                snprintf(buffer, sizeof(buffer), "Noise reduction %s, level %d", 
                         enabled ? "on" : "off", level >= 0 ? level : 0);
            }
            break;
            
        case SET_PARAM_AGC:
            snprintf(buffer, sizeof(buffer), "A G C %s", radio_get_agc_string());
            break;
            
        case SET_PARAM_PREAMP:
            value = radio_get_preamp();
            if (value == 0) {
                snprintf(buffer, sizeof(buffer), "Pre amp off");
            } else if (value == 1) {
                snprintf(buffer, sizeof(buffer), "Pre amp 1");
            } else if (value == 2) {
                snprintf(buffer, sizeof(buffer), "Pre amp 2");
            } else {
                snprintf(buffer, sizeof(buffer), "Pre amp not available");
            }
            break;
            
        case SET_PARAM_ATTENUATION:
            value = radio_get_attenuation();
            if (value == 0) {
                snprintf(buffer, sizeof(buffer), "Attenuation off");
            } else if (value > 0) {
                snprintf(buffer, sizeof(buffer), "Attenuation %d D B", value);
            } else {
                snprintf(buffer, sizeof(buffer), "Attenuation not available");
            }
            break;
            
        case SET_PARAM_MODE:
            {
                const char* mode = radio_get_mode_string();
                snprintf(buffer, sizeof(buffer), "Mode %s", mode);
            }
            break;
            
        default:
            snprintf(buffer, sizeof(buffer), "Select parameter");
            break;
    }
    
    speech_say_text(buffer);
}

static void apply_value(void) {
    int value = get_value_as_int();
    char buffer[64];
    int result = -1;
    
    switch (g_current_param) {
        case SET_PARAM_POWER:
            if (value >= 0 && value <= 100) {
                result = radio_set_power(value);
                if (result == 0) {
                    snprintf(buffer, sizeof(buffer), "Power set to %d", value);
                }
            }
            break;
            
        case SET_PARAM_MIC_GAIN:
            if (value >= 0 && value <= 100) {
                result = radio_set_mic_gain(value);
                if (result == 0) {
                    snprintf(buffer, sizeof(buffer), "Mic gain set to %d", value);
                }
            }
            break;
            
        case SET_PARAM_COMPRESSION:
            if (value >= 0 && value <= 100) {
                result = radio_set_compression(value);
                if (result == 0) {
                    snprintf(buffer, sizeof(buffer), "Compression set to %d", value);
                }
            }
            break;
            
        case SET_PARAM_NB:
            if (value >= 0 && value <= 10) {
                result = radio_set_nb(true, value);
                if (result == 0) {
                    snprintf(buffer, sizeof(buffer), "Noise blanker level %d", value);
                }
            }
            break;
            
        case SET_PARAM_NR:
            if (value >= 0 && value <= 10) {
                result = radio_set_nr(true, value);
                if (result == 0) {
                    snprintf(buffer, sizeof(buffer), "Noise reduction level %d", value);
                }
            }
            break;
            
        case SET_PARAM_PREAMP:
            if (value >= 0 && value <= 2) {
                result = radio_set_preamp(value);
                if (result == 0) {
                    if (value == 0) {
                        snprintf(buffer, sizeof(buffer), "Pre amp off");
                    } else {
                        snprintf(buffer, sizeof(buffer), "Pre amp %d", value);
                    }
                }
            }
            break;
            
        case SET_PARAM_ATTENUATION:
            result = radio_set_attenuation(value);
            if (result == 0) {
                if (value == 0) {
                    snprintf(buffer, sizeof(buffer), "Attenuation off");
                } else {
                    snprintf(buffer, sizeof(buffer), "Attenuation %d D B", value);
                }
            }
            break;
            
        default:
            break;
    }
    
    if (result == 0) {
        speech_say_text(buffer);
    } else {
        if (config_get_key_beep_enabled()) {
            comm_play_beep(COMM_BEEP_ERROR);
        }
        speech_say_text("Failed");
    }
    
    // Return to idle state after applying
    clear_value_buffer();
    g_current_param = SET_PARAM_NONE;
    g_state = SET_MODE_IDLE;
}

// ============================================================================
// Parameter Selection Handlers
// ============================================================================

static bool select_parameter(SetModeParameter param) {
    g_current_param = param;
    g_state = SET_MODE_EDITING;
    clear_value_buffer();
    
    DEBUG_PRINT("set_mode: Selected %s\n", param_name(param));
    announce_current_value(param);
    
    return true;
}

// ============================================================================
// Toggle Handlers (for NB, NR, Compression)
// ============================================================================

static void toggle_nb(bool enable) {
    int level = radio_get_nb_level();
    if (level < 0) level = 5;  // Default level
    
    if (radio_set_nb(enable, level) == 0) {
        speech_say_text(enable ? "Noise blanker on" : "Noise blanker off");
    } else {
        if (config_get_key_beep_enabled()) {
            comm_play_beep(COMM_BEEP_ERROR);
        }
        speech_say_text("Failed");
    }
}

static void toggle_nr(bool enable) {
    int level = radio_get_nr_level();
    if (level < 0) level = 5;  // Default level
    
    if (radio_set_nr(enable, level) == 0) {
        speech_say_text(enable ? "Noise reduction on" : "Noise reduction off");
    } else {
        if (config_get_key_beep_enabled()) {
            comm_play_beep(COMM_BEEP_ERROR);
        }
        speech_say_text("Failed");
    }
}

static void toggle_compression(bool enable) {
    if (radio_set_compression_enabled(enable) == 0) {
        speech_say_text(enable ? "Compression on" : "Compression off");
    } else {
        if (config_get_key_beep_enabled()) {
            comm_play_beep(COMM_BEEP_ERROR);
        }
        speech_say_text("Failed");
    }
}

// ============================================================================
// AGC Handlers
// ============================================================================

static void set_agc(AgcSpeed speed) {
    if (radio_set_agc_speed(speed) == 0) {
        const char* names[] = {"Off", "Fast", "Medium", "Slow"};
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "A G C %s", names[speed]);
        speech_say_text(buffer);
    } else {
        if (config_get_key_beep_enabled()) {
            comm_play_beep(COMM_BEEP_ERROR);
        }
        speech_say_text("Failed");
    }
}

// ============================================================================
// Initialization
// ============================================================================

void set_mode_init(void) {
    g_state = SET_MODE_OFF;
    g_current_param = SET_PARAM_NONE;
    clear_value_buffer();
    DEBUG_PRINT("set_mode_init: Initialized\n");
}

// ============================================================================
// State Queries
// ============================================================================

bool set_mode_is_active(void) {
    return g_state != SET_MODE_OFF;
}

SetModeState set_mode_get_state(void) {
    return g_state;
}

SetModeParameter set_mode_get_parameter(void) {
    return g_current_param;
}

// ============================================================================
// Mode Entry/Exit
// ============================================================================

void set_mode_enter(void) {
    if (g_state == SET_MODE_OFF) {
        g_state = SET_MODE_IDLE;
        g_current_param = SET_PARAM_NONE;
        clear_value_buffer();
        speech_say_text("Set");
        DEBUG_PRINT("set_mode_enter: Entered Set Mode\n");
    }
}

void set_mode_exit(void) {
    g_state = SET_MODE_OFF;
    g_current_param = SET_PARAM_NONE;
    clear_value_buffer();
    speech_say_text("Set Off");
    DEBUG_PRINT("set_mode_exit: Exited Set Mode\n");
}

void set_mode_cancel_edit(void) {
    if (g_state == SET_MODE_EDITING) {
        g_current_param = SET_PARAM_NONE;
        clear_value_buffer();
        g_state = SET_MODE_IDLE;
        speech_say_text("Cancelled");
        DEBUG_PRINT("set_mode_cancel_edit: Cancelled\n");
    }
}

const char* set_mode_get_value_buffer(void) {
    return g_value_buffer;
}

void set_mode_clear_value(void) {
    clear_value_buffer();
}

// ============================================================================
// Key Handling
// ============================================================================

bool set_mode_handle_key(char key, bool is_hold, bool is_shifted) {
    DEBUG_PRINT("set_mode_handle_key: key='%c' hold=%d shift=%d state=%d\n", 
                key, is_hold, is_shifted, g_state);
    
    // [B] - Toggle Set Mode or toggle OFF for toggle parameters
    if (key == 'B' && !is_hold && !is_shifted) {
        if (g_state == SET_MODE_OFF) {
            set_mode_enter();
            return true;
        } else if (g_state == SET_MODE_IDLE) {
            set_mode_exit();
            return true;
        } else if (g_state == SET_MODE_EDITING) {
            // For toggle parameters (NB, NR, Compression), [B] means "disable"
            switch (g_current_param) {
                case SET_PARAM_NB:
                    toggle_nb(false);
                    return true;
                case SET_PARAM_NR:
                    toggle_nr(false);
                    return true;
                case SET_PARAM_COMPRESSION:
                    toggle_compression(false);
                    return true;
                default:
                    // For other parameters, exit Set Mode
                    set_mode_exit();
                    return true;
            }
        }
    }
    
    // [D] - Cancel/Exit
    if (key == 'D' && !is_hold) {
        if (g_state == SET_MODE_EDITING) {
            set_mode_cancel_edit();
            return true;
        } else if (g_state == SET_MODE_IDLE) {
            set_mode_exit();
            return true;
        }
    }
    
    // If not active, don't consume keys
    if (g_state == SET_MODE_OFF) {
        return false;
    }
    
    // =========================================================================
    // SET_MODE_IDLE: Parameter Selection Keys
    // =========================================================================
    
    if (g_state == SET_MODE_IDLE) {
        // [9] Hold - Power Level
        if (key == '9' && is_hold && !is_shifted) {
            return select_parameter(SET_PARAM_POWER);
        }
        
        // [8] Hold - Mic Gain
        if (key == '8' && is_hold && !is_shifted) {
            return select_parameter(SET_PARAM_MIC_GAIN);
        }
        
        // [Shift]+[9] - Compression
        if (key == '9' && !is_hold && is_shifted) {
            return select_parameter(SET_PARAM_COMPRESSION);
        }
        
        // [7] - Noise Blanker
        if (key == '7' && !is_hold && !is_shifted) {
            return select_parameter(SET_PARAM_NB);
        }
        
        // [8] - Noise Reduction
        if (key == '8' && !is_hold && !is_shifted) {
            return select_parameter(SET_PARAM_NR);
        }
        
        // [4] Hold - AGC
        if (key == '4' && is_hold && !is_shifted) {
            return select_parameter(SET_PARAM_AGC);
        }
        
        // [4] - PreAmp
        if (key == '4' && !is_hold && !is_shifted) {
            return select_parameter(SET_PARAM_PREAMP);
        }
        
        // [Shift]+[4] - Attenuation
        if (key == '4' && !is_hold && is_shifted) {
            return select_parameter(SET_PARAM_ATTENUATION);
        }
        
        // [0] - Mode
        if (key == '0' && !is_hold && !is_shifted) {
            return select_parameter(SET_PARAM_MODE);
        }
        
        // Consume but ignore other keys in idle state
        return true;
    }
    
    // =========================================================================
    // SET_MODE_EDITING: Value Entry and Control Keys
    // =========================================================================
    
    if (g_state == SET_MODE_EDITING) {
        // Digits - accumulate value
        if (isdigit(key)) {
            add_digit(key);
            // Announce the digit
            char digit_text[2] = {key, '\0'};
            speech_say_text(digit_text);
            return true;
        }
        
        // [#] - Confirm and apply
        if (key == '#' && !is_hold) {
            if (g_value_len > 0) {
                apply_value();
            } else {
                // No value entered, just exit editing
                g_current_param = SET_PARAM_NONE;
                g_state = SET_MODE_IDLE;
            }
            return true;
        }
        
        // [*] - Clear value
        if (key == '*' && !is_hold) {
            clear_value_buffer();
            speech_say_text("Cleared");
            return true;
        }
        
        // [A] - Enable (for toggle parameters)
        if (key == 'A' && !is_hold) {
            switch (g_current_param) {
                case SET_PARAM_NB:
                    toggle_nb(true);
                    break;
                case SET_PARAM_NR:
                    toggle_nr(true);
                    break;
                case SET_PARAM_COMPRESSION:
                    toggle_compression(true);
                    break;
                default:
                    break;
            }
            // Stay in editing state
            return true;
        }
        
        // Note: [B] key is handled at the top of the function for toggle parameters
        
        // AGC-specific: [1] Hold = Fast, [2] Hold = Medium, [3] Hold = Slow
        if (g_current_param == SET_PARAM_AGC && is_hold) {
            if (key == '1') {
                set_agc(AGC_FAST);
                return true;
            } else if (key == '2') {
                set_agc(AGC_MEDIUM);
                return true;
            } else if (key == '3') {
                set_agc(AGC_SLOW);
                return true;
            }
        }
        
        // Mode-specific: [0] cycles mode
        if (g_current_param == SET_PARAM_MODE && key == '0' && !is_hold) {
            if (radio_cycle_mode() == 0) {
                const char* mode = radio_get_mode_string();
                speech_say_text(mode);
            } else {
                if (config_get_key_beep_enabled()) {
                    comm_play_beep(COMM_BEEP_ERROR);
                }
                speech_say_text("Failed");
            }
            return true;
        }
        
        // Consume other keys
        return true;
    }
    
    return false;
}
