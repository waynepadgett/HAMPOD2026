/**
 * @file config.h
 * @brief Configuration management for HAMPOD Software2
 * 
 * Provides:
 * - Load/save settings to INI file (auto-save on changes)
 * - 10-deep undo history
 * - Thread-safe getter/setter API
 */

#ifndef HAMPOD_CONFIG_H
#define HAMPOD_CONFIG_H

#include <stdbool.h>

// Undo system depth
#define CONFIG_UNDO_DEPTH 10

// Default values
#define CONFIG_DEFAULT_RADIO_MODEL    3073  // IC-7300
#define CONFIG_DEFAULT_RADIO_DEVICE   "/dev/ttyUSB0"
#define CONFIG_DEFAULT_RADIO_BAUD     19200
#define CONFIG_DEFAULT_VOLUME         80
#define CONFIG_DEFAULT_SPEECH_SPEED   1.0f
#define CONFIG_DEFAULT_KEY_BEEP       true

// Default config file path (relative to Software2 directory)
#define CONFIG_DEFAULT_PATH           "config/hampod.conf"

/**
 * @brief Main configuration structure
 */
typedef struct {
    // Radio settings
    int radio_model;
    char radio_device[64];
    int radio_baud;
    
    // Audio settings
    int output_volume;      // 0-100
    float speech_speed;     // 0.5-2.0
    bool key_beep_enabled;
} HampodConfig;

// ============================================================================
// Core Functions
// ============================================================================

/**
 * @brief Initialize configuration system
 * @param config_path Path to config file (NULL for default)
 * @return 0 on success, -1 on error
 */
int config_init(const char* config_path);

/**
 * @brief Save current configuration to file
 * @return 0 on success, -1 on error
 */
int config_save(void);

/**
 * @brief Cleanup configuration system
 */
void config_cleanup(void);

// ============================================================================
// Undo Functions
// ============================================================================

/**
 * @brief Undo last configuration change
 * @return 0 on success, -1 if no undo available
 */
int config_undo(void);

/**
 * @brief Get number of available undo steps
 * @return Number of undos available (0-10)
 */
int config_get_undo_count(void);

// ============================================================================
// Radio Getters
// ============================================================================

int config_get_radio_model(void);
const char* config_get_radio_device(void);
int config_get_radio_baud(void);

// ============================================================================
// Audio Getters
// ============================================================================

int config_get_volume(void);
float config_get_speech_speed(void);
bool config_get_key_beep_enabled(void);

// ============================================================================
// Radio Setters (auto-save after each)
// ============================================================================

void config_set_radio_model(int model);
void config_set_radio_device(const char* device);
void config_set_radio_baud(int baud);

// ============================================================================
// Audio Setters (auto-save after each)
// ============================================================================

void config_set_volume(int volume);
void config_set_speech_speed(float speed);
void config_set_key_beep_enabled(bool enabled);

#endif // HAMPOD_CONFIG_H
