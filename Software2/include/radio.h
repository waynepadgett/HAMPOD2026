/**
 * @file radio.h
 * @brief Radio control module using Hamlib
 *
 * Provides a clean abstraction over Hamlib for radio control:
 * - Thread-safe get/set frequency
 * - Radio polling with configurable callback
 * - Uses config module for radio model/device/baud settings
 *
 * Part of Phase 1: Frequency Mode Implementation
 */

#ifndef RADIO_H
#define RADIO_H

#include <stdbool.h>

// ============================================================================
// Initialization & Cleanup
// ============================================================================

/**
 * @brief Initialize connection to radio
 *
 * Uses settings from config module:
 * - config_get_radio_model()
 * - config_get_radio_device()
 * - config_get_radio_baud()
 *
 * @return 0 on success, -1 on error
 */
int radio_init(bool debug_mode);

/**
 * @brief Close connection to radio
 */
void radio_cleanup(void);

/**
 * @brief Check if radio is connected
 * @return true if connected, false otherwise
 */
bool radio_is_connected(void);

// ============================================================================
// Frequency Operations
// ============================================================================

/**
 * @brief Get current frequency from radio
 *
 * Thread-safe. Reads from current VFO.
 *
 * @return Frequency in Hz, or -1.0 on error
 */
double radio_get_frequency(void);

/**
 * @brief Set frequency on radio
 *
 * Thread-safe. Sets on current VFO.
 *
 * @param freq_hz Frequency in Hz (e.g., 14250000.0 for 14.250 MHz)
 * @return 0 on success, -1 on error
 */
int radio_set_frequency(double freq_hz);

// ============================================================================
// Radio Polling
// ============================================================================

/**
 * @brief Callback type for frequency change notifications
 *
 * @param new_freq The new frequency in Hz
 */
typedef void (*radio_freq_change_callback)(double new_freq);

/**
 * @brief Start polling radio for frequency changes
 *
 * Creates a background thread that polls the radio every 100ms.
 * When frequency changes and remains stable for 1 second, the
 * callback is invoked with the new frequency.
 *
 * @param on_change Callback function for frequency changes
 * @return 0 on success, -1 on error
 */
int radio_start_polling(radio_freq_change_callback on_change);

/**
 * @brief Stop polling radio
 */
void radio_stop_polling(void);

/**
 * @brief Check if polling is active
 * @return true if polling thread is running
 */
bool radio_is_polling(void);

// ============================================================================
// Auto-Reconnect
// ============================================================================

/**
 * @brief Callback when radio connects (or reconnects)
 */
typedef void (*radio_connect_callback)(void);

/**
 * @brief Callback when radio disconnects
 */
typedef void (*radio_disconnect_callback)(void);

/**
 * @brief Start auto-reconnect monitoring
 *
 * Creates a background thread that:
 * - If radio is not connected: checks for USB device every few seconds
 *   and attempts radio_init() when the device appears.
 * - If radio is connected: monitors for disconnect (repeated query failures).
 *
 * @param on_connect  Called when radio connects/reconnects
 * @param on_disconnect Called when radio disconnects
 * @return 0 on success, -1 on error
 */
int radio_start_reconnect(radio_connect_callback on_connect,
                          radio_disconnect_callback on_disconnect);

/**
 * @brief Stop auto-reconnect monitoring
 */
void radio_stop_reconnect(void);

#endif // RADIO_H
