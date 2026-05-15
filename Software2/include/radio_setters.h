/**
 * @file radio_setters.h
 * @brief Radio parameter setter functions for Set Mode
 * 
 * Part of Phase 3: Set Mode Implementation
 * 
 * These functions wrap Hamlib calls to set various radio parameters
 * like power, mic gain, compression, noise blanker/reduction, AGC,
 * preamp, and attenuation.
 */

#ifndef RADIO_SETTERS_H
#define RADIO_SETTERS_H

#include <stdbool.h>

// ============================================================================
// Power and Gain Levels
// ============================================================================

/**
 * @brief Set transmit power level
 * @param level Power level 0-100 (percentage)
 * @return 0 on success, -1 on error
 */
int radio_set_power(int level);

/**
 * @brief Get current power level
 * @return Power level 0-100, or -1 on error
 */
int radio_get_power(void);

/**
 * @brief Set microphone gain level
 * @param level Mic gain 0-100 (percentage)
 * @return 0 on success, -1 on error
 */
int radio_set_mic_gain(int level);

/**
 * @brief Get current mic gain level
 * @return Mic gain 0-100, or -1 on error
 */
int radio_get_mic_gain(void);

/**
 * @brief Set compression level
 * @param level Compression 0-100 or 0-10 depending on radio
 * @return 0 on success, -1 on error
 */
int radio_set_compression(int level);

/**
 * @brief Get current compression level
 * @return Compression level, or -1 on error
 */
int radio_get_compression(void);

/**
 * @brief Enable/disable compression
 * @param enabled true to enable, false to disable
 * @return 0 on success, -1 on error
 */
int radio_set_compression_enabled(bool enabled);

/**
 * @brief Check if compression is enabled
 * @return true if enabled, false if disabled
 */
bool radio_get_compression_enabled(void);

// ============================================================================
// Noise Controls
// ============================================================================

/**
 * @brief Set Noise Blanker state and level
 * @param enabled true to enable, false to disable
 * @param level NB level 0-10 (only used when enabled)
 * @return 0 on success, -1 on error
 */
int radio_set_nb(bool enabled, int level);

/**
 * @brief Get Noise Blanker state
 * @return true if enabled, false if disabled
 */
bool radio_get_nb_enabled(void);

/**
 * @brief Get Noise Blanker level
 * @return NB level 0-10, or -1 on error
 */
int radio_get_nb_level(void);

/**
 * @brief Set Noise Reduction state and level
 * @param enabled true to enable, false to disable
 * @param level NR level 0-10 (only used when enabled)
 * @return 0 on success, -1 on error
 */
int radio_set_nr(bool enabled, int level);

/**
 * @brief Get Noise Reduction state
 * @return true if enabled, false if disabled
 */
bool radio_get_nr_enabled(void);

/**
 * @brief Get Noise Reduction level
 * @return NR level 0-10, or -1 on error
 */
int radio_get_nr_level(void);

// ============================================================================
// AGC (Automatic Gain Control)
// ============================================================================

typedef enum {
    AGC_OFF = 0,
    AGC_FAST = 1,
    AGC_MEDIUM = 2,
    AGC_SLOW = 3
} AgcSpeed;

/**
 * @brief Set AGC speed
 * @param speed AGC_OFF, AGC_FAST, AGC_MEDIUM, or AGC_SLOW
 * @return 0 on success, -1 on error
 */
int radio_set_agc_speed(AgcSpeed speed);

/**
 * @brief Get current AGC speed
 * @return AgcSpeed value
 */
AgcSpeed radio_get_agc_speed(void);

/**
 * @brief Get AGC speed as human-readable string
 * @return "Off", "Fast", "Medium", or "Slow"
 */
const char* radio_get_agc_string(void);

// ============================================================================
// Preamp and Attenuation
// ============================================================================

/**
 * @brief Set preamp state
 * @param state 0=off, 1=preamp1, 2=preamp2
 * @return 0 on success, -1 on error
 */
int radio_set_preamp(int state);

/**
 * @brief Get current preamp state
 * @return 0=off, 1=preamp1, 2=preamp2, -1 on error
 */
int radio_get_preamp(void);

/**
 * @brief Set attenuation level
 * @param db Attenuation in dB (0=off, typical values: 6, 12, 18, 20)
 * @return 0 on success, -1 on error
 */
int radio_set_attenuation(int db);

/**
 * @brief Get current attenuation level
 * @return Attenuation in dB, or -1 on error
 */
int radio_get_attenuation(void);

// ============================================================================
// Mode Control
// ============================================================================

/**
 * @brief Cycle to next available operating mode
 * @return 0 on success, -1 on error
 */
int radio_cycle_mode(void);

/**
 * @brief Set specific operating mode
 * @param mode_index Index into mode list (0=AM, 1=CW, 2=USB, 3=LSB, 4=FM, 5=RTTY)
 * @return 0 on success, -1 on error
 */
int radio_set_mode_by_index(int mode_index);

int radio_set_tuning_step(int step_hz);

int radio_set_vox_status(bool enable);
// int radio_get_filter_number(void);
int radio_set_filter_number(int filter_num);

int radio_set_keyer_speed(int wpm);

int radio_set_mode_by_number(int mode_num);
#endif // RADIO_SETTERS_H
