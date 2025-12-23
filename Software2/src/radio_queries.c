/**
 * @file radio_queries.c
 * @brief Extended radio query functions implementation
 * 
 * Part of Phase 2: Normal Mode Implementation
 */

#include "radio_queries.h"
#include "radio.h"
#include "hampod_core.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <hamlib/rig.h>

// ============================================================================
// External Access to Radio Handle
// ============================================================================

// Defined in radio.c - we need access for extended queries
extern RIG *g_rig;
extern bool g_connected;
extern pthread_mutex_t g_rig_mutex;

// ============================================================================
// Mode Operations
// ============================================================================

/**
 * @brief Mode to string conversion table
 */
static const char* mode_to_string(rmode_t mode) {
    switch (mode) {
        case RIG_MODE_AM:      return "AM";
        case RIG_MODE_CW:      return "CW";
        case RIG_MODE_USB:     return "USB";
        case RIG_MODE_LSB:     return "LSB";
        case RIG_MODE_RTTY:    return "RTTY";
        case RIG_MODE_FM:      return "FM";
        case RIG_MODE_WFM:     return "Wide FM";
        case RIG_MODE_CWR:     return "CW Reverse";
        case RIG_MODE_RTTYR:   return "RTTY Reverse";
        case RIG_MODE_AMS:     return "AM Synchronous";
        case RIG_MODE_PKTLSB:  return "Packet LSB";
        case RIG_MODE_PKTUSB:  return "Packet USB";
        case RIG_MODE_PKTFM:   return "Packet FM";
        case RIG_MODE_ECSSUSB: return "ECSS USB";
        case RIG_MODE_ECSSLSB: return "ECSS LSB";
        case RIG_MODE_FAX:     return "FAX";
        case RIG_MODE_SAM:     return "SAM";
        case RIG_MODE_SAL:     return "SAL";
        case RIG_MODE_SAH:     return "SAH";
        case RIG_MODE_DSB:     return "DSB";
        case RIG_MODE_FMN:     return "FM Narrow";
        case RIG_MODE_PKTAM:   return "Packet AM";
        default:               return "Unknown";
    }
}

const char* radio_get_mode_string(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return "Not connected";
    }
    
    rmode_t mode;
    pbwidth_t width;
    int retcode = rig_get_mode(g_rig, RIG_VFO_CURR, &mode, &width);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_mode_string: %s\n", rigerror(retcode));
        return "Error";
    }
    
    return mode_to_string(mode);
}

int radio_get_mode_raw(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return 0;
    }
    
    rmode_t mode;
    pbwidth_t width;
    int retcode = rig_get_mode(g_rig, RIG_VFO_CURR, &mode, &width);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        return 0;
    }
    
    return (int)mode;
}

// ============================================================================
// VFO Operations
// ============================================================================

RadioVfo radio_get_vfo(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return RADIO_VFO_CURRENT;
    }
    
    vfo_t vfo;
    int retcode = rig_get_vfo(g_rig, &vfo);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_vfo: %s\n", rigerror(retcode));
        return RADIO_VFO_CURRENT;
    }
    
    if (vfo == RIG_VFO_A || vfo == RIG_VFO_MAIN) {
        return RADIO_VFO_A;
    } else if (vfo == RIG_VFO_B || vfo == RIG_VFO_SUB) {
        return RADIO_VFO_B;
    }
    
    return RADIO_VFO_CURRENT;
}

int radio_set_vfo(RadioVfo vfo) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    vfo_t hamlib_vfo;
    switch (vfo) {
        case RADIO_VFO_A:       hamlib_vfo = RIG_VFO_A; break;
        case RADIO_VFO_B:       hamlib_vfo = RIG_VFO_B; break;
        case RADIO_VFO_CURRENT: hamlib_vfo = RIG_VFO_CURR; break;
        default:                hamlib_vfo = RIG_VFO_CURR; break;
    }
    
    int retcode = rig_set_vfo(g_rig, hamlib_vfo);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_set_vfo: %s\n", rigerror(retcode));
        return -1;
    }
    
    DEBUG_PRINT("radio_set_vfo: Set to %d\n", vfo);
    return 0;
}

const char* radio_get_vfo_string(void) {
    RadioVfo vfo = radio_get_vfo();
    switch (vfo) {
        case RADIO_VFO_A:       return "VFO A";
        case RADIO_VFO_B:       return "VFO B";
        case RADIO_VFO_CURRENT: return "Current VFO";
        default:                return "VFO";
    }
}

// ============================================================================
// Meter Operations
// ============================================================================

double radio_get_smeter(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -999.0;
    }
    
    value_t val;
    int retcode = rig_get_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_STRENGTH, &val);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_smeter: %s\n", rigerror(retcode));
        return -999.0;
    }
    
    // val.i is signal strength in dB (S9 = 0dB reference in most radios)
    return (double)val.i;
}

const char* radio_get_smeter_string(char* buffer, int buf_size) {
    double db = radio_get_smeter();
    
    if (db <= -998.0) {
        return "Error";
    }
    
    // Convert dB to S-units
    // S9 is typically 0dB reference, each S-unit is 6dB
    // S1=-48dB, S2=-42dB, ..., S9=0dB, S9+10=+10dB
    if (db < -48) {
        snprintf(buffer, buf_size, "S0");
    } else if (db < 0) {
        int s_units = (int)((db + 54) / 6);
        if (s_units < 1) s_units = 1;
        if (s_units > 9) s_units = 9;
        snprintf(buffer, buf_size, "S%d", s_units);
    } else {
        // Over S9
        snprintf(buffer, buf_size, "S9 plus %d dB", (int)db);
    }
    
    return buffer;
}

double radio_get_power_meter(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1.0;
    }
    
    value_t val;
    int retcode = rig_get_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_RFPOWER_METER, &val);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_power_meter: %s\n", rigerror(retcode));
        return -1.0;
    }
    
    // val.f is 0.0-1.0 normalized
    return val.f;
}

const char* radio_get_power_string(char* buffer, int buf_size) {
    double power = radio_get_power_meter();
    
    if (power < 0) {
        return "Error";
    }
    
    // Assume 100W max for now - this should be configurable
    int watts = (int)(power * 100.0);
    snprintf(buffer, buf_size, "%d watts", watts);
    
    return buffer;
}
