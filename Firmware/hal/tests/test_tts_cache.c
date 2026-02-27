#include "../hal_tts_cache.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  printf("--- Running TTS Cache phase 1 tests ---\n");

  // Test 1: init
  if (hal_tts_cache_init() != 0) {
    fprintf(stderr, "Failed to init cache\n");
    return 1;
  }

  // Clear cache to start fresh
  hal_tts_cache_clear();

  const char *test_phrase = "hello world";
  int16_t *samples = NULL;
  size_t num_samples = 0;

  // Test 2: lookup on empty cache
  if (hal_tts_cache_lookup(test_phrase, &samples, &num_samples) == 0) {
    fprintf(stderr, "Lookup succeeded on empty cache!\n");
    return 1;
  }
  printf("Test 2 (miss): passed\n");

  // Test 3: store mock data
  int16_t mock_samples[100];
  for (int i = 0; i < 100; i++)
    mock_samples[i] = i * 10;

  if (hal_tts_cache_store(test_phrase, mock_samples, 100) != 0) {
    fprintf(stderr, "Store failed\n");
    return 1;
  }
  printf("Test 3 (store): passed\n");

  // Test 4: lookup hit
  if (hal_tts_cache_lookup(test_phrase, &samples, &num_samples) != 0) {
    fprintf(stderr, "Lookup failed after store\n");
    return 1;
  }

  if (num_samples != 100) {
    fprintf(stderr, "Lookup returned wrong size: %zu\n", num_samples);
    return 1;
  }

  for (int i = 0; i < 100; i++) {
    if (samples[i] != mock_samples[i]) {
      fprintf(stderr, "Data mismatch at %d\n", i);
      return 1;
    }
  }
  printf("Test 4 (hit & verify): passed\n");

  hal_tts_cache_release(samples);

  // Test 5: clear
  hal_tts_cache_clear();
  if (hal_tts_cache_lookup(test_phrase, &samples, &num_samples) == 0) {
    fprintf(stderr, "Lookup succeeded after clear!\n");
    return 1;
  }
  printf("Test 5 (clear): passed\n");

  printf("All Phase 1 unit tests passed\n");
  return 0;
}
