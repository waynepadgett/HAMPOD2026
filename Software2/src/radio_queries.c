/**
 * @file radio_queries.c
 * @brief Extended radio query functions implementation
 *
 * Part of Phase 2: Normal Mode Implementation
 */

#include "radio_queries.h"
#include "hampod_core.h"
#include "radio.h"

#include <hamlib/rig.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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
static const char *mode_to_string(rmode_t mode) {
  switch (mode) {
  case RIG_MODE_AM:
    return "AM";
  case RIG_MODE_CW:
    return "CW";
  case RIG_MODE_USB:
    return "USB";
  case RIG_MODE_LSB:
    return "LSB";
  case RIG_MODE_RTTY:
    return "RTTY";
  case RIG_MODE_FM:
    return "FM";
  case RIG_MODE_WFM:
    return "Wide FM";
  case RIG_MODE_CWR:
    return "CW Reverse";
  case RIG_MODE_RTTYR:
    return "RTTY Reverse";
  case RIG_MODE_AMS:
    return "AM Synchronous";
  case RIG_MODE_PKTLSB:
    return "Packet LSB";
  case RIG_MODE_PKTUSB:
    return "Packet USB";
  case RIG_MODE_PKTFM:
    return "Packet FM";
  case RIG_MODE_ECSSUSB:
    return "ECSS USB";
  case RIG_MODE_ECSSLSB:
    return "ECSS LSB";
  case RIG_MODE_FAX:
    return "FAX";
  case RIG_MODE_SAM:
    return "SAM";
  case RIG_MODE_SAL:
    return "SAL";
  case RIG_MODE_SAH:
    return "SAH";
  case RIG_MODE_DSB:
    return "DSB";
  case RIG_MODE_FMN:
    return "FM Narrow";
  case RIG_MODE_PKTAM:
    return "Packet AM";
  default:
    return "Unknown";
  }
}

const char *radio_get_mode_string(void) {
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
  case RADIO_VFO_A:
    hamlib_vfo = RIG_VFO_A;
    break;
  case RADIO_VFO_B:
    hamlib_vfo = RIG_VFO_B;
    break;
  case RADIO_VFO_CURRENT:
    hamlib_vfo = RIG_VFO_CURR;
    break;
  default:
    hamlib_vfo = RIG_VFO_CURR;
    break;
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

const char *radio_get_vfo_string(void) {
  RadioVfo vfo = radio_get_vfo();
  switch (vfo) {
  case RADIO_VFO_A:
    return "VFO A.";
  case RADIO_VFO_B:
    return "VFO B.";
  case RADIO_VFO_CURRENT:
    return "Current VFO";
  default:
    return "VFO";
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

const char *radio_get_smeter_string(char *buffer, int buf_size) {
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
    if (s_units < 1)
      s_units = 1;
    if (s_units > 9)
      s_units = 9;
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
  int retcode =
      rig_get_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_RFPOWER_METER, &val);

  pthread_mutex_unlock(&g_rig_mutex);

  if (retcode != RIG_OK) {
    DEBUG_PRINT("radio_get_power_meter: %s\n", rigerror(retcode));
    return -1.0;
  }

  // val.f is 0.0-1.0 normalized
  return val.f;
}

const char *radio_get_power_string(char *buffer, int buf_size) {
  double power = radio_get_power_meter();

  if (power < 0) {
    return "Error";
  }

  // Assume 100W max for now - this should be configurable
  int watts = (int)(power * 100.0);
  snprintf(buffer, buf_size, "%d watts", watts);

  return buffer;
}

// ============================================================================
// VOX Operations
// ============================================================================

/**
 * @brief Get VOX (Voice-Operated Transmit) status
 *
 * @return 1 if VOX is on, 0 if off, -1 on error
 */
int radio_get_vox_status(void) {
  pthread_mutex_lock(&g_rig_mutex);

  if (!g_connected || !g_rig) {
    pthread_mutex_unlock(&g_rig_mutex);
    return -1;
  }

  int status = 0;
  int retcode = rig_get_func(g_rig, RIG_VFO_CURR, RIG_FUNC_VOX, &status);

  pthread_mutex_unlock(&g_rig_mutex);

  if (retcode != RIG_OK) {
    DEBUG_PRINT("radio_get_vox_status: %s\n", rigerror(retcode));
    return -1;
  }

  return status ? 1 : 0;
}

// get radio break in status
int radio_get_break_in_status(void) {
    pthread_mutex_lock(&g_rig_mutex);

    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }

    int semi = 0;
    int full = 0;

    int ret_semi = rig_get_func(g_rig, RIG_VFO_CURR, RIG_FUNC_SBKIN, &semi);
    int ret_full = rig_get_func(g_rig, RIG_VFO_CURR, RIG_FUNC_FBKIN, &full);

    pthread_mutex_unlock(&g_rig_mutex);

    if (ret_semi != RIG_OK && ret_full != RIG_OK) {
        DEBUG_PRINT("radio_get_break_in_status: break-in unavailable\n");
        return -1;
    }

    if (full) return 2;   // full break-in
    if (semi) return 1;   // semi break-in
    return 0;             // off
}

// toggle memory scan
int radio_toggle_memory_scan(void)
{
    return -1;
}

// get tunning step status
int radio_get_tuning_step(void)
{
    pthread_mutex_lock(&g_rig_mutex);

    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }

    shortfreq_t ts;
    int ret = rig_get_ts(g_rig, RIG_VFO_CURR, &ts);

    pthread_mutex_unlock(&g_rig_mutex);

    if (ret != RIG_OK) {
        DEBUG_PRINT("radio_get_tuning_step: %s\n", rigerror(ret));
        return -1;
    }

    return ts;   // tuning step in Hz
}

// get squelch setting
int radio_get_squelch_level(void)
{
    pthread_mutex_lock(&g_rig_mutex);

    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }

    value_t val;
    int ret = rig_get_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_SQL, &val);

    pthread_mutex_unlock(&g_rig_mutex);

    if (ret != RIG_OK) {
        DEBUG_PRINT("radio_get_squelch_level: %s\n", rigerror(ret));
        return -1;
    }

    // normalized value (0.0 – 1.0)
    return (int)(val.f * 100);
}

// toggle split mode
int radio_toggle_split_mode(void)
{
    pthread_mutex_lock(&g_rig_mutex);

    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }

    split_t split;
    vfo_t tx_vfo;

    int ret = rig_get_split_vfo(g_rig, RIG_VFO_CURR, &split, &tx_vfo);

    if (ret != RIG_OK) {
        pthread_mutex_unlock(&g_rig_mutex);
        DEBUG_PRINT("radio_toggle_split_mode get: %s\n", rigerror(ret));
        return -1;
    }

    split_t new_split = (split == RIG_SPLIT_ON) ? RIG_SPLIT_OFF : RIG_SPLIT_ON;

    ret = rig_set_split_vfo(g_rig, RIG_VFO_CURR, new_split, RIG_VFO_B);

    pthread_mutex_unlock(&g_rig_mutex);

    if (ret != RIG_OK) {
        DEBUG_PRINT("radio_toggle_split_mode set: %s\n", rigerror(ret));
        return -1;
    }

    return (new_split == RIG_SPLIT_ON) ? 1 : 0;
}

// exchange vfo A and B
int radio_exchange_vfo(void)
{
    pthread_mutex_lock(&g_rig_mutex);

    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }

    int ret = rig_vfo_op(g_rig, RIG_VFO_CURR, RIG_OP_XCHG);

    pthread_mutex_unlock(&g_rig_mutex);

    if (ret != RIG_OK) {
        DEBUG_PRINT("radio_exchange_vfo: %s\n", rigerror(ret));
        return -1;
    }

    return 1;
}

// read filter width
int radio_get_filter_width(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -999;
    }
    
    rmode_t mode;
    pbwidth_t width;
    int retcode = rig_get_mode(g_rig, RIG_VFO_CURR, &mode, &width);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_filter_width: %s\n", rigerror(retcode));
        return -999;
    }
    
    // width is the filter width in Hz
    return (int)width;
}
// audio peaker filter
int radio_get_apf_status(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -999;
    }
    
    int status = 0;
    int retcode = rig_get_func(g_rig, RIG_VFO_CURR, RIG_FUNC_APF, &status);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_apf_status: %s\n", rigerror(retcode));
        return -999;
    }
    
    return status;   // 0 = off, nonzero = on
}
// get filter number
int radio_get_filter_number(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -999;
    }
    
    rmode_t mode;
    pbwidth_t width;
    int retcode = rig_get_mode(g_rig, RIG_VFO_CURR, &mode, &width);
    
    pthread_mutex_unlock(&g_rig_mutex);
    
    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_filter_number: %s\n", rigerror(retcode));
        return -999;
    }

    // If unknown / normal
    if (width <= 0) {
        return 1;  // assume default filter
    }

    // Map width → filter number (approximate)
    if (width > 2000) {
        return 1;  // wide
    } else if (width > 1000) {
        return 2;  // medium
    } else {
        return 3;  // narrow
    }
}

int radio_get_tuner_status(void) {
    pthread_mutex_lock(&g_rig_mutex);

    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -999;
    }

    int status = 0;
    int retcode = rig_get_func(g_rig, RIG_VFO_CURR, RIG_FUNC_TUNER, &status);

    pthread_mutex_unlock(&g_rig_mutex);

    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_tuner_status: %s\n", rigerror(retcode));
        return -999;
    }

    return status ? 1 : 0;
}

int radio_get_antenna(void) {
    pthread_mutex_lock(&g_rig_mutex);

    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -999;
    }

    ant_t ant_curr = 0;
    ant_t ant_tx = 0;
    ant_t ant_rx = 0;
    value_t option = {0};

    int retcode = rig_get_ant(g_rig, RIG_VFO_CURR, 0,
                              &option, &ant_curr, &ant_tx, &ant_rx);

    pthread_mutex_unlock(&g_rig_mutex);

    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_antenna: %s\n", rigerror(retcode));
        return -999;
    }

    return (ant_curr > 0) ? (int)ant_curr : -999;
}

float radio_get_swr(void) {
    pthread_mutex_lock(&g_rig_mutex);

    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -999.0f;
    }

    value_t val;
    int retcode = rig_get_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_SWR, &val);

    pthread_mutex_unlock(&g_rig_mutex);

    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_swr: %s\n", rigerror(retcode));
        return -999.0f;
    }

    return val.f;
}

int radio_toggle_data_mode(void) {
    pthread_mutex_lock(&g_rig_mutex);

    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -999;
    }

    rmode_t mode;
    pbwidth_t width;

    int retcode = rig_get_mode(g_rig, RIG_VFO_CURR, &mode, &width);
    if (retcode != RIG_OK) {
        pthread_mutex_unlock(&g_rig_mutex);
        DEBUG_PRINT("radio_toggle_data_mode get: %s\n", rigerror(retcode));
        return -999;
    }

    rmode_t new_mode = mode;

    // Toggle logic
    switch (mode) {
        case RIG_MODE_USB:
            new_mode = RIG_MODE_PKTUSB;
            break;
        case RIG_MODE_LSB:
            new_mode = RIG_MODE_PKTLSB;
            break;
        case RIG_MODE_PKTUSB:
            new_mode = RIG_MODE_USB;
            break;
        case RIG_MODE_PKTLSB:
            new_mode = RIG_MODE_LSB;
            break;
        default:
            // Not supported for this mode
            pthread_mutex_unlock(&g_rig_mutex);
            return -999;
    }

    retcode = rig_set_mode(g_rig, RIG_VFO_CURR, new_mode, width);

    pthread_mutex_unlock(&g_rig_mutex);

    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_toggle_data_mode set: %s\n", rigerror(retcode));
        return -999;
    }

    return 0;
}

int radio_get_keyer_speed(void) {
    pthread_mutex_lock(&g_rig_mutex);

    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -999;
    }

    value_t val;
    memset(&val, 0, sizeof(val));

    int retcode = rig_get_level(g_rig, RIG_VFO_CURR, RIG_LEVEL_KEYSPD, &val);

    pthread_mutex_unlock(&g_rig_mutex);

    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_keyer_speed: %s\n", rigerror(retcode));
        return -999;
    }

    return val.i;   // WPM
}