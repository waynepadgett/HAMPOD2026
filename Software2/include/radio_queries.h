/**
 * @file radio_queries.h
 * @brief Extended radio query functions using Hamlib
 * 
 * Provides mode, VFO, and meter query capabilities on top of the
 * basic radio module (radio.h) which handles frequency operations.
 * 
 * Part of Phase 2: Normal Mode Implementation
 */

#ifndef RADIO_QUERIES_H
#define RADIO_QUERIES_H

#include <stdbool.h>

// ============================================================================
// Mode Operations
// ============================================================================

/**
 * @brief Get current operating mode as human-readable string
 * 
 * @return Mode string ("USB", "LSB", "CW", "FM", "AM", etc.), or "UNKNOWN"
 */
const char* radio_get_mode_string(void);

/**
 * @brief Get current operating mode as Hamlib rmode_t value
 * 
 * @return Hamlib mode value, or 0 on error
 */
int radio_get_mode_raw(void);

// ============================================================================
// VFO Operations
// ============================================================================

/**
 * @brief VFO selection values
 */
typedef enum {
    RADIO_VFO_A = 0,
    RADIO_VFO_B = 1,
    RADIO_VFO_CURRENT = 2
} RadioVfo;

/**
 * @brief Get current VFO selection
 * 
 * @return RADIO_VFO_A, RADIO_VFO_B, or RADIO_VFO_CURRENT
 */
RadioVfo radio_get_vfo(void);

/**
 * @brief Switch to a different VFO
 * 
 * @param vfo VFO to select (RADIO_VFO_A, RADIO_VFO_B)
 * @return 0 on success, -1 on error
 */
int radio_set_vfo(RadioVfo vfo);

/**
 * @brief Get VFO selection as human-readable string
 * 
 * @return "VFO A", "VFO B", or "Current VFO"
 */
const char* radio_get_vfo_string(void);

// ============================================================================
// Meter Operations
// ============================================================================

/**
 * @brief Get S-meter reading
 * 
 * The S-meter measures received signal strength.
 * 
 * @return S-meter value in dB (typically -54 to +30), or -999.0 on error
 */
double radio_get_smeter(void);

/**
 * @brief Get S-meter reading as human-readable string
 * 
 * Converts dB value to S-units (e.g., "S9", "S9+20dB")
 * 
 * @param buffer Buffer to write string into
 * @param buf_size Size of buffer
 * @return Pointer to buffer, or "ERROR" on failure
 */
const char* radio_get_smeter_string(char* buffer, int buf_size);

/**
 * @brief Get RF power meter reading
 * 
 * Reads the transmit power level during transmission.
 * 
 * @return Power level 0.0-1.0 (normalized), or -1.0 on error
 */
double radio_get_power_meter(void);

/**
 * @brief Get power meter reading as human-readable string
 * 
 * @param buffer Buffer to write string into
 * @param buf_size Size of buffer
 * @return Pointer to buffer (e.g., "75 watts"), or "ERROR"
 */
const char* radio_get_power_string(char* buffer, int buf_size);

#endif // RADIO_QUERIES_H
