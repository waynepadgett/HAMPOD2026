/**
 * @file radio_setters.c
 * @brief Radio parameter setter implementations
 * 
 * Part of Phase 3: Set Mode Implementation
 * 
 * Uses Hamlib to set various radio parameters.
 */

#include "radio_setters.h"
#include "hampod_core.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <hamlib/rig.h>

// ============================================================================
// External Hamlib State (from radio.c)
// ============================================================================

extern RIG* g_rig;
extern bool g_connected;
extern pthread_mutex_t g_rig_mutex;

// ============================================================================
// Mode List for Cycling
// ============================================================================

static const rmode_t mode_list[] = {
    RIG_MODE_USB,
    RIG_MODE_LSB,
    RIG_MODE_CW,
    RIG_MODE_AM,
    RIG_MODE_FM,
    RIG_MODE_RTTY
};
static const int mode_count = sizeof(mode_list) / sizeof(mode_list[0]);

// ============================================================================
// Power and Gain Levels
// ============================================================================

int radio_set_power(int level) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    // Clamp level to 0-100
    if (level < 0) level = 0;
    if (level > 100) level = 100;
    
    // Hamlib power is 0.0-1.0
    value_t val;
    val.f = level / 100.0f;
    
    int retcode = rig_set_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_RFPOWER, val);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_set_power: %s\n", rigerror(retcode));
        return -1;
    }
    
    DEBUG_PRINT("radio_set_power: Set to %d%%\n", level);
    return 0;
}

int radio_get_power(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    value_t val;
    int retcode = rig_get_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_RFPOWER, &val);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_power: %s\n", rigerror(retcode));
        return -1;
    }
    
    return (int)(val.f * 100.0f + 0.5f);
}

int radio_set_mic_gain(int level) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    if (level < 0) level = 0;
    if (level > 100) level = 100;
    
    value_t val;
    val.f = level / 100.0f;
    
    int retcode = rig_set_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_MICGAIN, val);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_set_mic_gain: %s\n", rigerror(retcode));
        return -1;
    }
    
    DEBUG_PRINT("radio_set_mic_gain: Set to %d%%\n", level);
    return 0;
}

int radio_get_mic_gain(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    value_t val;
    int retcode = rig_get_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_MICGAIN, &val);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_mic_gain: %s\n", rigerror(retcode));
        return -1;
    }
    
    return (int)(val.f * 100.0f + 0.5f);
}

int radio_set_compression(int level) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    // Compression level varies by radio (0-10 or 0-100)
    // Use 0-100 range and let Hamlib normalize
    if (level < 0) level = 0;
    if (level > 100) level = 100;
    
    value_t val;
    val.f = level / 100.0f;
    
    int retcode = rig_set_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_COMP, val);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_set_compression: %s\n", rigerror(retcode));
        return -1;
    }
    
    DEBUG_PRINT("radio_set_compression: Set to %d\n", level);
    return 0;
}

int radio_get_compression(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    value_t val;
    int retcode = rig_get_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_COMP, &val);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_compression: %s\n", rigerror(retcode));
        return -1;
    }
    
    return (int)(val.f * 100.0f + 0.5f);
}

int radio_set_compression_enabled(bool enabled) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    int retcode = rig_set_func(g_rig, RIG_VFO_CURR, RIG_FUNC_COMP, enabled ? 1 : 0);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_set_compression_enabled: %s\n", rigerror(retcode));
        return -1;
    }
    
    DEBUG_PRINT("radio_set_compression_enabled: %s\n", enabled ? "on" : "off");
    return 0;
}

bool radio_get_compression_enabled(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return false;
    }
    
    int status = 0;
    int retcode = rig_get_func(g_rig, RIG_VFO_CURR, RIG_FUNC_COMP, &status);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_compression_enabled: %s\n", rigerror(retcode));
        return false;
    }
    
    return status != 0;
}

// ============================================================================
// Noise Controls
// ============================================================================

int radio_set_nb(bool enabled, int level) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    int retcode;
    
    // Set on/off state
    retcode = rig_set_func(g_rig, RIG_VFO_CURR, RIG_FUNC_NB, enabled ? 1 : 0);
    if (retcode != RIG_OK) {
        pthread_mutex_unlock(&g_rig_mutex);
        DEBUG_PRINT("radio_set_nb (func): %s\n", rigerror(retcode));
        return -1;
    }
    
    // Set level if enabled
    if (enabled && level >= 0) {
        if (level > 10) level = 10;
        value_t val;
        val.f = level / 10.0f;
        retcode = rig_set_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_NB, val);
        if (retcode != RIG_OK) {
            pthread_mutex_unlock(&g_rig_mutex);
            DEBUG_PRINT("radio_set_nb (level): %s\n", rigerror(retcode));
            return -1;
        }
    }
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    DEBUG_PRINT("radio_set_nb: enabled=%d level=%d\n", enabled, level);
    return 0;
}

bool radio_get_nb_enabled(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return false;
    }
    
    int status = 0;
    int retcode = rig_get_func(g_rig, RIG_VFO_CURR, RIG_FUNC_NB, &status);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_nb_enabled: %s\n", rigerror(retcode));
        return false;
    }
    
    return status != 0;
}

int radio_get_nb_level(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    value_t val;
    int retcode = rig_get_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_NB, &val);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_nb_level: %s\n", rigerror(retcode));
        return -1;
    }
    
    return (int)(val.f * 10.0f + 0.5f);
}

int radio_set_nr(bool enabled, int level) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    int retcode;
    
    // Set on/off state
    retcode = rig_set_func(g_rig, RIG_VFO_CURR, RIG_FUNC_NR, enabled ? 1 : 0);
    if (retcode != RIG_OK) {
        pthread_mutex_unlock(&g_rig_mutex);
        DEBUG_PRINT("radio_set_nr (func): %s\n", rigerror(retcode));
        return -1;
    }
    
    // Set level if enabled
    if (enabled && level >= 0) {
        if (level > 10) level = 10;
        value_t val;
        val.f = level / 10.0f;
        retcode = rig_set_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_NR, val);
        if (retcode != RIG_OK) {
            pthread_mutex_unlock(&g_rig_mutex);
            DEBUG_PRINT("radio_set_nr (level): %s\n", rigerror(retcode));
            return -1;
        }
    }
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    DEBUG_PRINT("radio_set_nr: enabled=%d level=%d\n", enabled, level);
    return 0;
}

bool radio_get_nr_enabled(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return false;
    }
    
    int status = 0;
    int retcode = rig_get_func(g_rig, RIG_VFO_CURR, RIG_FUNC_NR, &status);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_nr_enabled: %s\n", rigerror(retcode));
        return false;
    }
    
    return status != 0;
}

int radio_get_nr_level(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    value_t val;
    int retcode = rig_get_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_NR, &val);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_nr_level: %s\n", rigerror(retcode));
        return -1;
    }
    
    return (int)(val.f * 10.0f + 0.5f);
}

// ============================================================================
// AGC (Automatic Gain Control)
// ============================================================================

int radio_set_agc_speed(AgcSpeed speed) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    value_t val;
    // Hamlib uses integer AGC constants
    switch (speed) {
        case AGC_OFF:    val.i = RIG_AGC_OFF; break;
        case AGC_FAST:   val.i = RIG_AGC_FAST; break;
        case AGC_MEDIUM: val.i = RIG_AGC_MEDIUM; break;
        case AGC_SLOW:   val.i = RIG_AGC_SLOW; break;
        default:         val.i = RIG_AGC_AUTO; break;
    }
    
    int retcode = rig_set_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_AGC, val);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_set_agc_speed: %s\n", rigerror(retcode));
        return -1;
    }
    
    DEBUG_PRINT("radio_set_agc_speed: %d\n", speed);
    return 0;
}

AgcSpeed radio_get_agc_speed(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return AGC_OFF;
    }
    
    value_t val;
    int retcode = rig_get_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_AGC, &val);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_agc_speed: %s\n", rigerror(retcode));
        return AGC_OFF;
    }
    
    switch (val.i) {
        case RIG_AGC_OFF:    return AGC_OFF;
        case RIG_AGC_FAST:   return AGC_FAST;
        case RIG_AGC_MEDIUM: return AGC_MEDIUM;
        case RIG_AGC_SLOW:   return AGC_SLOW;
        default:             return AGC_MEDIUM;  // Default for AUTO etc.
    }
}

const char* radio_get_agc_string(void) {
    AgcSpeed speed = radio_get_agc_speed();
    switch (speed) {
        case AGC_OFF:    return "Off";
        case AGC_FAST:   return "Fast";
        case AGC_MEDIUM: return "Medium";
        case AGC_SLOW:   return "Slow";
        default:         return "Unknown";
    }
}

// ============================================================================
// Preamp and Attenuation
// ============================================================================

int radio_set_preamp(int state) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    // Hamlib preamp is typically 0=off, 10=preamp1, 20=preamp2 (dB values)
    // But many radios just use 0, 1, 2 indices
    value_t val;
    val.i = state * 10;  // Convert 0/1/2 to 0/10/20
    
    int retcode = rig_set_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_PREAMP, val);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_set_preamp: %s\n", rigerror(retcode));
        return -1;
    }
    
    DEBUG_PRINT("radio_set_preamp: %d\n", state);
    return 0;
}

int radio_get_preamp(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    value_t val;
    int retcode = rig_get_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_PREAMP, &val);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_preamp: %s\n", rigerror(retcode));
        return -1;
    }
    
    // Convert dB back to 0/1/2
    return val.i / 10;
}

int radio_set_attenuation(int db) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    value_t val;
    val.i = db;
    
    int retcode = rig_set_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_ATT, val);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_set_attenuation: %s\n", rigerror(retcode));
        return -1;
    }
    
    DEBUG_PRINT("radio_set_attenuation: %d dB\n", db);
    return 0;
}

int radio_get_attenuation(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    value_t val;
    int retcode = rig_get_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_ATT, &val);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_attenuation: %s\n", rigerror(retcode));
        return -1;
    }
    
    return val.i;
}

// ============================================================================
// Mode Control
// ============================================================================

int radio_cycle_mode(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    // Get current mode
    rmode_t current_mode;
    pbwidth_t current_width;
    int retcode = rig_get_mode(g_rig, RIG_VFO_CURR, &current_mode, &current_width);
    
    if (retcode != RIG_OK) {
        pthread_mutex_unlock(&g_rig_mutex);
        DEBUG_PRINT("radio_cycle_mode (get): %s\n", rigerror(retcode));
        return -1;
    }
    
    // Find current mode in list
    int current_index = -1;
    for (int i = 0; i < mode_count; i++) {
        if (mode_list[i] == current_mode) {
            current_index = i;
            break;
        }
    }
    
    // Try next modes until one works
    for (int i = 0; i < mode_count; i++) {
        int next_index = (current_index + 1 + i) % mode_count;
        rmode_t next_mode = mode_list[next_index];
        pbwidth_t width = rig_passband_normal(g_rig, next_mode);
        
        retcode = rig_set_mode(g_rig, RIG_VFO_CURR, next_mode, width);
        if (retcode == RIG_OK) {
            pthread_mutex_unlock(&g_rig_mutex);
            DEBUG_PRINT("radio_cycle_mode: Set to %s\n", rig_strrmode(next_mode));
            return 0;
        }
    }
    
    pthread_mutex_unlock(&g_rig_mutex);
    DEBUG_PRINT("radio_cycle_mode: No mode available\n");
    return -1;
}

int radio_set_mode_by_index(int mode_index) {
    if (mode_index < 0 || mode_index >= mode_count) {
        return -1;
    }
    
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    rmode_t mode = mode_list[mode_index];
    pbwidth_t width = rig_passband_normal(g_rig, mode);
    
    int retcode = rig_set_mode(g_rig, RIG_VFO_CURR, mode, width);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_set_mode_by_index: %s\n", rigerror(retcode));
        return -1;
    }
    
    DEBUG_PRINT("radio_set_mode_by_index: Set to %s\n", rig_strrmode(mode));
    return 0;
}


int radio_set_tuning_step(int step_hz) {
    pthread_mutex_lock(&g_rig_mutex);

    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }

    if (step_hz <= 0) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }

    int retcode = rig_set_ts(g_rig, RIG_VFO_CURR, (shortfreq_t)step_hz);

    pthread_mutex_unlock(&g_rig_mutex);

    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_set_tuning_step: %s\n", rigerror(retcode));
        return -1;
    }

    DEBUG_PRINT("radio_set_tuning_step: Set to %d Hz\n", step_hz);
    return 0;
}

int radio_set_vox_status(bool enable)
{
    pthread_mutex_lock(&g_rig_mutex);

    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        DEBUG_PRINT("radio_set_vox_status: not connected\n");
        return -1;
    }

    int retcode = rig_set_func(
        g_rig,
        RIG_VFO_CURR,
        RIG_FUNC_VOX,
        enable ? 1 : 0
    );

    pthread_mutex_unlock(&g_rig_mutex);

    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_set_vox_status: %s\n", rigerror(retcode));
        return -1;
    }

    DEBUG_PRINT("radio_set_vox_status: %s\n", enable ? "ON" : "OFF");
    return 0;
}

int radio_set_filter_number(int filter_num) {
    pthread_mutex_lock(&g_rig_mutex);

    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -999;
    }

    if (filter_num < 1 || filter_num > 3) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -999;
    }

    rmode_t mode;
    pbwidth_t current_width;
    int retcode = rig_get_mode(g_rig, RIG_VFO_CURR, &mode, &current_width);
    if (retcode != RIG_OK) {
        pthread_mutex_unlock(&g_rig_mutex);
        DEBUG_PRINT("radio_set_filter_number get_mode: %s\n", rigerror(retcode));
        return -999;
    }

    pbwidth_t new_width;

    // Approximate mapping: Filter 1 = wide, Filter 2 = medium, Filter 3 = narrow
    switch (filter_num) {
        case 1:
            new_width = 2400;   // wide
            break;
        case 2:
            new_width = 1800;   // medium
            break;
        case 3:
            new_width = 500;    // narrow
            break;
        default:
            pthread_mutex_unlock(&g_rig_mutex);
            return -999;
    }

    retcode = rig_set_mode(g_rig, RIG_VFO_CURR, mode, new_width);

    pthread_mutex_unlock(&g_rig_mutex);

    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_set_filter_number set_mode: %s\n", rigerror(retcode));
        return -999;
    }

    return 0;
}

int radio_set_keyer_speed(int wpm) {
    pthread_mutex_lock(&g_rig_mutex);

    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -999;
    }

    // Optional: basic sanity range (IC-7300 is typically ~6–48 WPM)
    if (wpm < 5 || wpm > 60) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -999;
    }

    value_t val;
    memset(&val, 0, sizeof(val));
    val.i = wpm;

    int retcode = rig_set_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_KEYSPD, val);

    pthread_mutex_unlock(&g_rig_mutex);

    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_set_keyer_speed: %s\n", rigerror(retcode));
        return -999;
    }

    return 0;
}

