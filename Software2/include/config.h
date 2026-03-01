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
#define CONFIG_DEFAULT_RADIO_MODEL 3073 // IC-7300
#define CONFIG_DEFAULT_RADIO_DEVICE "/dev/ttyUSB0"
#define CONFIG_DEFAULT_RADIO_BAUD 19200
#define CONFIG_DEFAULT_VOLUME 25
#define CONFIG_DEFAULT_SPEECH_SPEED 1.0f
#define CONFIG_DEFAULT_KEY_BEEP true

// Default config file path (relative to Software2 directory)
#define CONFIG_DEFAULT_PATH "config/hampod.conf"

#define MAX_RADIOS 10

/**
 * @brief Individual radio settings
 */
typedef struct {
  bool enabled;       // Only one radio active at a time
  char name[64];      // Friendly name (e.g. "IC-7300")
  int model;          // Hamlib model ID
  char device[64];    // Path (e.g. /dev/ttyUSB0)
  int baud;           // Baud rate
  char port[128];     // Physical USB port path
  int detected_model; // Model ID detected at runtime
} RadioSettings;

/**
 * @brief Audio device and playback settings
 */
typedef struct {
  char preferred_device[64]; // "USB2.0 Device" etc
  char device_name[128];     // Actual name detected
  char port[128];            // Physical USB port path
  int card_number;           // ALSA card number
  int volume;                // 0-100
  float speech_speed;        // 0.5-2.0
  bool key_beep_enabled;
} AudioSettings;

/**
 * @brief Keypad device tracking
 */
typedef struct {
  char port[128];        // Physical USB port path
  char device_name[128]; // Actual name detected
  char layout[16];       // "calculator" (default) or "phone"
} KeypadSettings;

/**
 * @brief Main configuration structure
 */
typedef struct {
  RadioSettings radios[MAX_RADIOS];
  AudioSettings audio;
  KeypadSettings keypad;
} HampodConfig;

// ============================================================================
// Core Functions
// ============================================================================

/**
 * @brief Initialize configuration system
 * @param config_path Path to config file (NULL for default)
 * @return 0 on success, -1 on error
 */
int config_init(const char *config_path);

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
// Radio Management
// ============================================================================

/**
 * @brief Get the index of the currently enabled radio
 * @return Index (0 to MAX_RADIOS-1) or -1 if none enabled
 */
int config_get_active_radio_index(void);

/**
 * @brief Get settings for a specific radio
 * @param index Index (0 to MAX_RADIOS-1)
 * @return Pointer to internal RadioSettings (read-only)
 */
const RadioSettings *config_get_radio(int index);

/**
 * @brief Set a radio as enabled
 * Note: Enforcing only one radio enabled at a time
 * @param index Index to enable
 */
void config_set_radio_enabled(int index, bool enabled);

// ============================================================================
// Radio Getters (Act on the currently active radio)
// ============================================================================

int config_get_radio_model(void);
const char *config_get_radio_device(void);
int config_get_radio_baud(void);
const char *config_get_radio_name(void);
const char *config_get_radio_port(void);
int config_get_radio_detected_model(void);

// ============================================================================
// Audio Getters
// ============================================================================

int config_get_volume(void);
float config_get_speech_speed(void);
bool config_get_key_beep_enabled(void);
const char *config_get_audio_preferred_device(void);
const char *config_get_audio_device_name(void);
const char *config_get_audio_port(void);
int config_get_audio_card_number(void);

// ============================================================================
// Keypad Getters
// ============================================================================

const char *config_get_keypad_port(void);
const char *config_get_keypad_device_name(void);
const char *config_get_keypad_layout(void);

// ============================================================================
// Radio Setters (Act on the currently active radio, auto-save after each)
// ============================================================================

void config_set_radio_model(int model);
void config_set_radio_device(const char *device);
void config_set_radio_baud(int baud);
void config_set_radio_name(const char *name);
void config_set_radio_port(const char *port);
void config_set_radio_detected_model(int model);

// ============================================================================
// Audio Setters (auto-save after each)
// ============================================================================

void config_set_volume(int volume);
void config_set_speech_speed(float speed);
void config_set_key_beep_enabled(bool enabled);
void config_set_audio_device_name(const char *name);
void config_set_audio_port(const char *port);
void config_set_audio_card_number(int card);

// ============================================================================
// Keypad Setters (auto-save after each)
// ============================================================================

void config_set_keypad_port(const char *port);
void config_set_keypad_device_name(const char *name);
void config_set_keypad_layout(const char *layout);

#endif // HAMPOD_CONFIG_H
