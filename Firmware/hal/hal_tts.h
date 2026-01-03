#ifndef HAL_TTS_H
#define HAL_TTS_H

/**
 * @file hal_tts.h
 * @brief Hardware Abstraction Layer for Text-to-Speech
 *
 * This HAL provides a unified interface for different TTS implementations
 * (Festival, Piper) allowing the firmware to remain engine-agnostic.
 */

/**
 * @brief Initialize TTS subsystem
 *
 * For Festival: Verifies text2wave is available.
 * For Piper: Checks for piper binary, model file, and starts persistent
 * pipeline.
 *
 * @return 0 on success, -1 on failure (prints guidance to stderr)
 */
int hal_tts_init(void);

/**
 * @brief Speak text
 *
 * Festival: Blocking - generates WAV and plays immediately.
 * Piper: Async - writes to persistent pipeline for immediate playback.
 *
 * @param text The text to speak
 * @param output_file Optional output file path (for caching). NULL for direct
 * playback.
 * @return 0 on success, -1 on failure
 */
int hal_tts_speak(const char *text, const char *output_file);

/**
 * @brief Interrupt current speech
 *
 * Immediately stops any ongoing speech.
 * For Piper: Closes and reopens the persistent pipeline.
 */
void hal_tts_interrupt(void);

/**
 * @brief Cleanup TTS resources
 *
 * For Piper: Closes the persistent pipeline.
 */
void hal_tts_cleanup(void);

/**
 * @brief Get TTS implementation name
 *
 * @return Pointer to static string (e.g., "Festival", "Piper (Persistent)")
 */
const char *hal_tts_get_impl_name(void);

#endif /* HAL_TTS_H */
