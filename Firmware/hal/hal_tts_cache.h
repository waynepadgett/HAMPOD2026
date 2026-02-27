#ifndef HAL_TTS_CACHE_H
#define HAL_TTS_CACHE_H

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Initialize the TTS cache
 * @return 0 on success, -1 on failure
 */
int hal_tts_cache_init(void);

/**
 * @brief Look up a phrase in the cache
 * @param text The phrase to look up
 * @param samples Pointer to store the loaded PCM samples (dynamically
 * allocated)
 * @param num_samples Pointer to store the number of samples
 * @return 0 on success (hit), -1 on miss/error
 */
int hal_tts_cache_lookup(const char *text, int16_t **samples,
                         size_t *num_samples);

/**
 * @brief Release a cache entry previously returned by lookup
 * @param samples The pointer returned by lookup
 */
void hal_tts_cache_release(int16_t *samples);

/**
 * @brief Store a phrase in the cache
 * @param text The phrase to store
 * @param samples The PCM data
 * @param num_samples The number of samples
 * @return 0 on success, -1 on failure
 */
int hal_tts_cache_store(const char *text, const int16_t *samples,
                        size_t num_samples);

/**
 * @brief Clean up cache resources
 */
void hal_tts_cache_cleanup(void);

/**
 * @brief Clear the entire cache
 * @return 0 on success, -1 on failure
 */
int hal_tts_cache_clear(void);

#endif /* HAL_TTS_CACHE_H */
