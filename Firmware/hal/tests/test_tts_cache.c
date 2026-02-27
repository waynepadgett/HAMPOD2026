#include "../hal_audio.h"
#include "../hal_tts.h"
#include "../hal_tts_cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

long long time_ms(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int main(void) {
  printf("--- Running TTS Cache latency and integration tests ---\n");

  // Clear cache to start fresh
  hal_tts_cache_clear();

  if (hal_audio_init() != 0) {
    fprintf(stderr, "Failed to init audio\n");
    return 1;
  }

  if (hal_tts_init() != 0) {
    fprintf(stderr, "Failed to init TTS\n");
    return 1;
  }

  const char *phrase = "hello world";

  printf("Test: Speak (cold)\n");
  hal_audio_play_beep(BEEP_KEYPRESS);
  long long start = time_ms();
  hal_tts_speak(phrase, NULL);
  long long cold_duration = time_ms() - start;
  printf("Cold speaking completed in: %lld ms\n", cold_duration);

  if (cold_duration < 100) {
    fprintf(
        stderr,
        "Warning: cold speak was unexpectedly fast... cache not cleared?\n");
  }

  // Wait for Piper to finish generating and capturing
  usleep(500000); // 500ms should be enough

  printf("Test: Speak (warm)\n");
  hal_audio_play_beep(BEEP_KEYPRESS);
  start = time_ms();
  hal_tts_speak(phrase, NULL);
  long long warm_duration = time_ms() - start;
  printf("Warm speak completed in: %lld ms\n", warm_duration);

  if (warm_duration >= 2000) {
    fprintf(stderr, "FAILED: Warm speak was too slow!\n");
    return 1;
  }

  printf("Test: Clear cache\n");
  hal_tts_cache_clear();

  start = time_ms();
  hal_tts_speak(phrase, NULL);
  long long cold_again_duration = time_ms() - start;
  printf("Cold (again) speak took: %lld ms\n", cold_again_duration);

  if (cold_again_duration < 100) {
    fprintf(stderr, "FAILED: Cache clear didn't work, speak was too fast\n");
    return 1;
  }

  hal_tts_cleanup();
  hal_audio_cleanup();

  printf("All Phase 1 integration tests passed\n");
  return 0;
}
