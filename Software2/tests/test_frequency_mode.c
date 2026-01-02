/**
 * @file test_frequency_mode.c
 * @brief Unit tests for frequency mode state machine
 *
 * Tests the state machine logic without requiring hardware.
 * Uses mocked radio and speech modules.
 *
 * Part of Phase 1: Frequency Mode Implementation
 */

#include "frequency_mode.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Test Framework
// ============================================================================

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name)                                                         \
  do {                                                                         \
    printf("  Running %s... ", #name);                                         \
    tests_run++;                                                               \
    test_##name();                                                             \
    tests_passed++;                                                            \
    printf("OK\n");                                                            \
  } while (0)

#define ASSERT_EQ(a, b)                                                        \
  do {                                                                         \
    if ((a) != (b)) {                                                          \
      printf("FAILED: %s != %s\n", #a, #b);                                    \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

#define ASSERT_TRUE(x) ASSERT_EQ((x), 1)
#define ASSERT_FALSE(x) ASSERT_EQ((x), 0)

// ============================================================================
// Mock Infrastructure
// ============================================================================

// Mock speech - just track what was said
static char last_speech[256];
static int speech_call_count = 0;

void speech_say_text(const char *text) {
  strncpy(last_speech, text, sizeof(last_speech) - 1);
  speech_call_count++;
  printf("[SPEECH] %s\n", text);
}

int speech_init(void) { return 0; }
void speech_cleanup(void) {}

// Mock radio - track set frequency
static double last_set_frequency = 0;
static int radio_set_count = 0;

int radio_set_frequency(double freq_hz) {
  last_set_frequency = freq_hz;
  radio_set_count++;
  printf("[RADIO] Set frequency to %.3f MHz\n", freq_hz / 1000000.0);
  return 0;
}

int radio_init(void) { return 0; }
void radio_cleanup(void) {}
double radio_get_frequency(void) { return 14250000.0; }

// Mock config
int config_init(const char *p) {
  (void)p;
  return 0;
}
void config_cleanup(void) {}
int config_get_radio_model(void) { return 3073; }
const char *config_get_radio_device(void) { return "/dev/ttyUSB0"; }
int config_get_radio_baud(void) { return 19200; }
bool config_get_key_beep_enabled(void) { return false; }

// Mock comm_play_beep
void comm_play_beep(const char *beep_type) { (void)beep_type; }

// Mock radio_queries (for radio_set_vfo used in submit_frequency)
#include "radio_queries.h"
int radio_set_vfo(RadioVfo vfo) {
  (void)vfo;
  printf("[RADIO] Set VFO to %d\n", vfo);
  return 0;
}

// Mock normal_mode (for normal_mode_get_verbosity used in on_radio_change)
bool normal_mode_get_verbosity(void) { return true; }

// ============================================================================
// Tests
// ============================================================================

TEST(initial_state) {
  frequency_mode_init();
  ASSERT_EQ(frequency_mode_get_state(), FREQ_MODE_IDLE);
  ASSERT_FALSE(frequency_mode_is_active());
}

TEST(enter_mode_with_hash) {
  frequency_mode_init();

  // Press # to enter mode
  bool consumed = frequency_mode_handle_key('#', false);

  ASSERT_TRUE(consumed);
  ASSERT_EQ(frequency_mode_get_state(), FREQ_MODE_SELECT_VFO);
  ASSERT_TRUE(frequency_mode_is_active());
}

TEST(cycle_vfo) {
  frequency_mode_init();

  // Enter mode
  frequency_mode_handle_key('#', false);
  ASSERT_EQ(frequency_mode_get_state(), FREQ_MODE_SELECT_VFO);

  // Press # again to cycle VFO
  frequency_mode_handle_key('#', false);
  // Should still be in SELECT_VFO state, just cycled
  ASSERT_EQ(frequency_mode_get_state(), FREQ_MODE_SELECT_VFO);

  // Press # two more times to cycle through all
  frequency_mode_handle_key('#', false);
  frequency_mode_handle_key('#', false);
  ASSERT_EQ(frequency_mode_get_state(), FREQ_MODE_SELECT_VFO);
}

TEST(enter_digits) {
  frequency_mode_init();
  speech_call_count = 0;

  // Enter mode
  frequency_mode_handle_key('#', false);

  // Enter digit
  frequency_mode_handle_key('1', false);
  ASSERT_EQ(frequency_mode_get_state(), FREQ_MODE_ENTERING);

  // Enter more digits
  frequency_mode_handle_key('4', false);
  ASSERT_EQ(frequency_mode_get_state(), FREQ_MODE_ENTERING);

  // Should have announced each digit
  ASSERT_TRUE(speech_call_count >= 3); // VFO + 2 digits
}

TEST(decimal_point) {
  frequency_mode_init();

  // Enter mode and digits
  frequency_mode_handle_key('#', false);
  frequency_mode_handle_key('1', false);
  frequency_mode_handle_key('4', false);

  // Insert decimal
  frequency_mode_handle_key('*', false);
  ASSERT_EQ(frequency_mode_get_state(), FREQ_MODE_ENTERING);

  // More digits after decimal
  frequency_mode_handle_key('2', false);
  frequency_mode_handle_key('5', false);
  frequency_mode_handle_key('0', false);

  ASSERT_EQ(frequency_mode_get_state(), FREQ_MODE_ENTERING);
}

TEST(submit_frequency) {
  frequency_mode_init();
  radio_set_count = 0;

  // Enter 14.250
  frequency_mode_handle_key('#', false);
  frequency_mode_handle_key('1', false);
  frequency_mode_handle_key('4', false);
  frequency_mode_handle_key('*', false);
  frequency_mode_handle_key('2', false);
  frequency_mode_handle_key('5', false);
  frequency_mode_handle_key('0', false);

  // Submit with #
  frequency_mode_handle_key('#', false);

  // Should be back to idle
  ASSERT_EQ(frequency_mode_get_state(), FREQ_MODE_IDLE);
  ASSERT_FALSE(frequency_mode_is_active());

  // Radio should have been called
  ASSERT_EQ(radio_set_count, 1);

  // Frequency should be 14.250 MHz = 14250000 Hz
  double expected = 14250000.0;
  double diff = last_set_frequency - expected;
  if (diff < 0)
    diff = -diff;
  ASSERT_TRUE(diff < 1000); // Allow 1 kHz tolerance
}

TEST(cancel_with_double_star) {
  frequency_mode_init();

  // Enter some digits with decimal
  frequency_mode_handle_key('#', false);
  frequency_mode_handle_key('1', false);
  frequency_mode_handle_key('4', false);
  frequency_mode_handle_key('*', false);
  frequency_mode_handle_key('2', false);

  // Second * cancels
  frequency_mode_handle_key('*', false);

  ASSERT_EQ(frequency_mode_get_state(), FREQ_MODE_IDLE);
  ASSERT_FALSE(frequency_mode_is_active());
}

TEST(cancel_with_d) {
  frequency_mode_init();

  // Enter mode
  frequency_mode_handle_key('#', false);
  frequency_mode_handle_key('1', false);

  // Cancel with D
  frequency_mode_handle_key('D', false);

  ASSERT_EQ(frequency_mode_get_state(), FREQ_MODE_IDLE);
  ASSERT_FALSE(frequency_mode_is_active());
}

TEST(cancel_from_vfo_select) {
  frequency_mode_init();

  // Enter mode
  frequency_mode_handle_key('#', false);
  ASSERT_EQ(frequency_mode_get_state(), FREQ_MODE_SELECT_VFO);

  // Cancel with *
  frequency_mode_handle_key('*', false);

  ASSERT_EQ(frequency_mode_get_state(), FREQ_MODE_IDLE);
}

TEST(key_not_consumed_when_idle) {
  frequency_mode_init();

  // Non-# keys should not be consumed when idle
  bool consumed = frequency_mode_handle_key('5', false);
  ASSERT_FALSE(consumed);

  consumed = frequency_mode_handle_key('*', false);
  ASSERT_FALSE(consumed);

  ASSERT_EQ(frequency_mode_get_state(), FREQ_MODE_IDLE);
}

TEST(force_cancel) {
  frequency_mode_init();

  // Enter mode and start entering
  frequency_mode_handle_key('#', false);
  frequency_mode_handle_key('1', false);
  frequency_mode_handle_key('4', false);

  ASSERT_TRUE(frequency_mode_is_active());

  // Force cancel
  frequency_mode_cancel();

  ASSERT_EQ(frequency_mode_get_state(), FREQ_MODE_IDLE);
  ASSERT_FALSE(frequency_mode_is_active());
}

// ============================================================================
// Main
// ============================================================================

int main(void) {
  printf("=== Frequency Mode Unit Tests ===\n\n");

  RUN_TEST(initial_state);
  RUN_TEST(enter_mode_with_hash);
  RUN_TEST(cycle_vfo);
  RUN_TEST(enter_digits);
  RUN_TEST(decimal_point);
  RUN_TEST(submit_frequency);
  RUN_TEST(cancel_with_double_star);
  RUN_TEST(cancel_with_d);
  RUN_TEST(cancel_from_vfo_select);
  RUN_TEST(key_not_consumed_when_idle);
  RUN_TEST(force_cancel);

  printf("\n=== Results: %d/%d tests passed ===\n", tests_passed, tests_run);

  return (tests_passed == tests_run) ? 0 : 1;
}
