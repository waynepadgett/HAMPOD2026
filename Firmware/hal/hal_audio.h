#ifndef HAL_AUDIO_H
#define HAL_AUDIO_H

/**
 * @file hal_audio.h
 * @brief Hardware Abstraction Layer for audio output
 *
 * This HAL provides a unified interface for different audio output
 * implementations (USB audio, onboard audio, etc.) allowing the firmware to
 * remain hardware-agnostic.
 */

/**
 * @brief Initialize audio hardware
 *
 * Detects and configures the audio output device.
 * Must be called before any other HAL audio functions.
 *
 * @return 0 on success, negative error code on failure
 */
int hal_audio_init(void);

/**
 * @brief Set audio output device explicitly
 *
 * Allows manual configuration of the audio device if auto-detection
 * is insufficient or a specific device is required.
 *
 * @param device_name ALSA device name (e.g., "plughw:CARD=Device,DEV=0")
 * @return 0 on success, negative error code on failure
 */
int hal_audio_set_device(const char *device_name);

/**
 * @brief Get current audio device name
 *
 * Returns the ALSA device string currently configured for audio output.
 *
 * @return Pointer to static string containing device name
 */
const char *hal_audio_get_device(void);

/**
 * @brief Play an audio file
 *
 * Plays the specified WAV file through the configured audio device.
 * This function blocks until playback completes.
 *
 * @param filepath Absolute or relative path to WAV file
 * @return 0 on success, negative error code on failure
 */
int hal_audio_play_file(const char *filepath);

/**
 * @brief Cleanup audio resources
 *
 * Releases any allocated resources and resets audio configuration.
 * Should be called before program termination.
 */
void hal_audio_cleanup(void);

/**
 * @brief Get audio implementation name
 *
 * Returns a string describing the current audio implementation
 * (e.g., "USB Audio", "Onboard Audio")
 *
 * @return Pointer to static string containing implementation name
 */
const char *hal_audio_get_impl_name(void);

/* ============================================================================
 * Persistent Pipeline API (for low-latency audio)
 * ============================================================================
 */

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Write raw PCM samples to the audio pipeline
 *
 * Writes 16-bit signed samples directly to the persistent aplay process.
 * Sample rate and format must match pipeline configuration (16kHz mono).
 *
 * @param samples Pointer to 16-bit signed samples
 * @param num_samples Number of samples to write
 * @return 0 on success, -1 on failure
 */
int hal_audio_write_raw(const int16_t *samples, size_t num_samples);

/**
 * @brief Interrupt current audio playback
 *
 * Sets an interrupt flag that will cause streaming playback to stop
 * at the next chunk boundary (approximately 50ms).
 */
void hal_audio_interrupt(void);

/**
 * @brief Clear the interrupt flag
 *
 * Resets the interrupt flag to allow new audio playback.
 * Should be called at the start of a new audio/TTS operation.
 */
void hal_audio_clear_interrupt(void);

/**
 * @brief Check if audio is currently playing
 *
 * @return 1 if playing, 0 otherwise
 */
int hal_audio_is_playing(void);

/**
 * @brief Check if the audio pipeline is ready for streaming
 *
 * @return 1 if pipeline is ready, 0 otherwise
 */
int hal_audio_pipeline_ready(void);

/* ============================================================================
 * RAM-Cached Beep API (for low-latency beeps)
 * ============================================================================
 */

/**
 * @brief Beep type enumeration
 */
typedef enum {
  BEEP_KEYPRESS, /**< Short beep on key press */
  BEEP_HOLD,     /**< Lower-pitch beep for key hold */
  BEEP_ERROR     /**< Error/invalid key beep */
} BeepType;

/**
 * @brief Play a beep from RAM cache
 *
 * Plays a pre-loaded beep sound with minimal latency (no file I/O).
 * Beep sounds are loaded into RAM at hal_audio_init().
 *
 * @param type Type of beep to play
 * @return 0 on success, -1 on failure
 */
int hal_audio_play_beep(BeepType type);

/* ============================================================================
 * Device Info API (for volume control and change detection)
 * ============================================================================
 */

/**
 * @brief Get the selected ALSA card number
 *
 * Returns the card number of the detected audio device.
 * Used by Software2 for volume control via amixer.
 *
 * @return Card number (0-255), or -1 if not initialized
 */
int hal_audio_get_card_number(void);

/**
 * @brief Get the USB port path of the audio device
 *
 * Returns the sysfs path of the USB port the audio device is connected to.
 * Used for configuration change detection.
 *
 * @return Pointer to static string containing port path, or NULL if not USB
 */
const char *hal_audio_get_port_path(void);

#endif /* HAL_AUDIO_H */
