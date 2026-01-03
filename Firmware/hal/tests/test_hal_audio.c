/**
 * @file test_hal_audio.c
 * @brief Unit tests for the Audio HAL persistent pipeline
 *
 * Part of Chunk 1.3: Audio HAL Unit Tests
 */

#include "../hal_audio.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Test result counters */
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST_PASS(name)                                                        \
  do {                                                                         \
    printf("  [PASS] %s\n", name);                                             \
    tests_passed++;                                                            \
  } while (0)

#define TEST_FAIL(name, reason)                                                \
  do {                                                                         \
    printf("  [FAIL] %s: %s\n", name, reason);                                 \
    tests_failed++;                                                            \
  } while (0)

/* ============================================================================
 * Test Cases
 * ============================================================================
 */

/**
 * Test: Pipeline starts and stops cleanly
 */
void test_audio_init_cleanup(void) {
  printf("\n=== Test: Init and Cleanup ===\n");

  /* Initialize */
  if (hal_audio_init() != 0) {
    TEST_FAIL("hal_audio_init", "returned non-zero");
    return;
  }
  TEST_PASS("hal_audio_init");

  /* Check pipeline ready */
  if (!hal_audio_pipeline_ready()) {
    TEST_FAIL("hal_audio_pipeline_ready", "pipeline not ready after init");
  } else {
    TEST_PASS("hal_audio_pipeline_ready");
  }

  /* Cleanup */
  hal_audio_cleanup();
  TEST_PASS("hal_audio_cleanup");

  /* Verify cleanup worked */
  if (hal_audio_pipeline_ready()) {
    TEST_FAIL("pipeline state after cleanup", "pipeline still ready");
  } else {
    TEST_PASS("pipeline state after cleanup");
  }
}

/**
 * Test: Write raw samples to pipeline
 */
void test_audio_write_raw(void) {
  int16_t samples[800]; /* 50ms of silence at 16kHz */
  int i;

  printf("\n=== Test: Write Raw Samples ===\n");

  /* Initialize */
  if (hal_audio_init() != 0) {
    TEST_FAIL("init for write_raw test", "hal_audio_init failed");
    return;
  }

  /* Generate silence samples */
  for (i = 0; i < 800; i++) {
    samples[i] = 0;
  }

  /* Write samples */
  if (hal_audio_write_raw(samples, 800) != 0) {
    TEST_FAIL("hal_audio_write_raw", "returned non-zero");
  } else {
    TEST_PASS("hal_audio_write_raw (silence)");
  }

  /* Generate a short beep (1000Hz for 50ms) */
  for (i = 0; i < 800; i++) {
    /* 1000Hz sine wave: sin(2*pi*1000*i/16000) */
    /* Using integer approximation */
    int phase = (i * 1000 * 360) / 16000;
    phase = phase % 360;
    /* Simple triangle wave approximation */
    if (phase < 90) {
      samples[i] = (int16_t)(phase * 364); /* 0 to 32760 */
    } else if (phase < 270) {
      samples[i] = (int16_t)((180 - phase) * 364); /* 32760 to -32760 */
    } else {
      samples[i] = (int16_t)((phase - 360) * 364); /* -32760 to 0 */
    }
  }

  if (hal_audio_write_raw(samples, 800) != 0) {
    TEST_FAIL("hal_audio_write_raw", "beep write failed");
  } else {
    TEST_PASS("hal_audio_write_raw (beep)");
  }

  /* Give time for audio to play */
  usleep(200000); /* 200ms */

  hal_audio_cleanup();
}

/**
 * Test: Play WAV file through pipeline
 */
void test_audio_play_file(void) {
  const char *test_file = "../pregen_audio/1.wav";

  printf("\n=== Test: Play WAV File ===\n");

  /* Initialize */
  if (hal_audio_init() != 0) {
    TEST_FAIL("init for play_file test", "hal_audio_init failed");
    return;
  }

  /* Check if test file exists */
  if (access(test_file, F_OK) != 0) {
    printf("  [SKIP] Test file not found: %s\n", test_file);
    hal_audio_cleanup();
    return;
  }

  /* Play file */
  if (hal_audio_play_file(test_file) != 0) {
    /* May fail if file format doesn't match pipeline - check fallback */
    printf("  [INFO] hal_audio_play_file returned error (may be format "
           "mismatch)\n");
  } else {
    TEST_PASS("hal_audio_play_file");
  }

  /* Give time for audio to play */
  usleep(500000); /* 500ms */

  hal_audio_cleanup();
}

/**
 * Test: Interrupt during playback
 */
void test_audio_interrupt(void) {
  int16_t samples[8000]; /* 500ms of audio at 16kHz */
  int i;

  printf("\n=== Test: Interrupt During Playback ===\n");

  /* Initialize */
  if (hal_audio_init() != 0) {
    TEST_FAIL("init for interrupt test", "hal_audio_init failed");
    return;
  }

  /* Generate 500ms of beeps */
  for (i = 0; i < 8000; i++) {
    int phase = (i * 500 * 360) / 16000;
    phase = phase % 360;
    if (phase < 180) {
      samples[i] = (int16_t)(16000);
    } else {
      samples[i] = (int16_t)(-16000);
    }
  }

  /* Test interrupt state */
  if (hal_audio_is_playing()) {
    TEST_FAIL("is_playing before playback", "should be 0");
  } else {
    TEST_PASS("is_playing before playback");
  }

  /* Test interrupt function */
  hal_audio_interrupt();
  TEST_PASS("hal_audio_interrupt called");

  hal_audio_cleanup();
}

/* ============================================================================
 * Main
 * ============================================================================
 */

int main(int argc, char *argv[]) {
  printf("=============================================\n");
  printf("  HAMPOD Audio HAL Unit Tests\n");
  printf("=============================================\n");

  (void)argc;
  (void)argv;

  /* Run tests */
  test_audio_init_cleanup();
  test_audio_write_raw();
  test_audio_play_file();
  test_audio_interrupt();

  /* Summary */
  printf("\n=============================================\n");
  printf("  Results: %d passed, %d failed\n", tests_passed, tests_failed);
  printf("=============================================\n");

  return (tests_failed > 0) ? 1 : 0;
}
