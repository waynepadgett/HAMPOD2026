/**
 * keypad.c - Keypad Input Module Implementation
 *
 * Implements keypad polling with hold detection using a background thread.
 *
 * Hold Detection Algorithm:
 * The Firmware reports a key once when pressed, then reports '-' continuously.
 * We detect holds by measuring the time between key press and release:
 *
 * 1. When a key is first detected, record the time
 * 2. While key is being reported, check if held > threshold -> fire hold event
 * 3. When key is released (we see '-'), fire press/hold based on duration
 *
 * Part of Phase 0: Core Infrastructure (Step 3.1)
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "comm.h"
#include "config.h"
#include "keypad.h"

// ============================================================================
// Configuration Defaults
// ============================================================================

#define DEFAULT_HOLD_THRESHOLD_MS 500
#define DEFAULT_POLL_INTERVAL_MS 50

// ============================================================================
// Module State
// ============================================================================

static pthread_t keypad_thread;
static volatile bool running = false;
static KeypadCallback user_callback = NULL;

// Configuration
static int hold_threshold_ms = DEFAULT_HOLD_THRESHOLD_MS;
static int poll_interval_ms = DEFAULT_POLL_INTERVAL_MS;

// Hold detection state
static char last_key = '-';            // Last key seen (or '-' for none)
static struct timespec key_press_time; // When the key was first pressed
static bool hold_event_fired = false;  // Have we already fired a hold event?

// ============================================================================
// Private Helper Functions
// ============================================================================

// Get current time in milliseconds
static long get_time_ms(void) {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return now.tv_sec * 1000 + now.tv_nsec / 1000000;
}

// Calculate elapsed time since key_press_time in milliseconds
static long elapsed_since_press(void) {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  return (now.tv_sec - key_press_time.tv_sec) * 1000 +
         (now.tv_nsec - key_press_time.tv_nsec) / 1000000;
}

// Fire a key event to the registered callback
static void fire_event(char key, bool is_hold) {
  if (user_callback == NULL) {
    return;
  }

  // Play beep feedback if enabled
  if (config_get_key_beep_enabled()) {
    if (is_hold) {
      // Lower-pitch beep for hold events
      comm_play_beep(COMM_BEEP_HOLD);
    } else {
      // Standard beep for press events
      comm_play_beep(COMM_BEEP_KEYPRESS);
    }
  }

  KeyPressEvent event = {.key = key,
                         .shiftAmount = 0, // Reserved for future use
                         .isHold = is_hold};

  LOG_DEBUG("Firing key event: key='%c', isHold=%s", key,
            is_hold ? "YES" : "NO");

  user_callback(&event);
}

// ============================================================================
// Keypad Thread
// ============================================================================

static void *keypad_thread_func(void *arg) {
  (void)arg;

  LOG_INFO("Keypad thread started");

  /* Number of consecutive no-key polls before considering key released.
   * Linux key repeat has gaps between events, so we need debouncing. */
  static const int RELEASE_THRESHOLD = 6; /* 6 polls x 50ms = 300ms */
  int no_key_count = 0;

  int consecutive_errors = 0;

  while (running) {
    char key;

    // Poll for key state
    int read_result = comm_read_keypad(&key);
    if (read_result != HAMPOD_OK) {
      if (read_result == HAMPOD_TIMEOUT) {
        /* Timeouts are recoverable - just retry.
         * Can happen during heavy load (e.g. TTS/audio processing) */
        LOG_ERROR("Keypad read timeout, retrying...");
        consecutive_errors = 0; /* Timeouts don't count as errors */
        continue;
      }
      /* Real error - allow a few retries before giving up */
      consecutive_errors++;
      LOG_ERROR("Failed to read keypad (%d consecutive errors)",
                consecutive_errors);
      if (consecutive_errors >= 3) {
        LOG_ERROR("Too many keypad errors, stopping");
        break;
      }
      usleep(100000); /* 100ms backoff before retry */
      continue;
    }
    consecutive_errors = 0;

    // Is a key currently pressed?
    bool key_pressed = (key != '-' && key != 0xFF && key != 0x00);

    if (key_pressed) {
      // Key is being reported - reset no-key counter
      no_key_count = 0;

      if (last_key == '-') {
        // First time seeing this key - record it and the time
        last_key = key;
        clock_gettime(CLOCK_MONOTONIC, &key_press_time);
        hold_event_fired = false;
        LOG_DEBUG("Key down: '%c'", key);

      } else if (last_key == key) {
        // Same key still being reported - check for hold
        if (!hold_event_fired && elapsed_since_press() >= hold_threshold_ms) {
          fire_event(key, true); // Hold event
          hold_event_fired = true;
        }

      } else {
        // Different key - handle as release of old + press of new
        if (!hold_event_fired) {
          fire_event(last_key, false); // Press event for old key
        }
        last_key = key;
        clock_gettime(CLOCK_MONOTONIC, &key_press_time);
        hold_event_fired = false;
        LOG_DEBUG("Key changed to: '%c'", key);
      }

    } else {
      // No key pressed - but might just be a gap between repeat events
      if (last_key != '-') {
        no_key_count++;

        // Check for hold while waiting for release confirmation
        if (!hold_event_fired && elapsed_since_press() >= hold_threshold_ms) {
          fire_event(last_key, true); // Hold event
          hold_event_fired = true;
        }

        // Only consider truly released after threshold
        if (no_key_count >= RELEASE_THRESHOLD) {
          long hold_time = elapsed_since_press();

          if (!hold_event_fired) {
            // Determine if it was a hold or press based on duration
            if (hold_time >= hold_threshold_ms) {
              fire_event(last_key, true); // Hold
            } else {
              fire_event(last_key, false); // Press
            }
          }

          LOG_DEBUG("Key up: '%c' (held for %ldms)", last_key, hold_time);
          last_key = '-';
          no_key_count = 0;
        }
      }
    }

    // Sleep between polls
    usleep(poll_interval_ms * 1000);
  }

  LOG_INFO("Keypad thread exiting");

  return NULL;
}

// ============================================================================
// Public API - Initialization
// ============================================================================

int keypad_init(void) {
  if (running) {
    LOG_ERROR("Keypad system already running");
    return HAMPOD_ERROR;
  }

  LOG_INFO("Initializing keypad system...");

  // Reset state
  last_key = '-';
  hold_event_fired = false;

  // Start keypad thread
  running = true;
  if (pthread_create(&keypad_thread, NULL, keypad_thread_func, NULL) != 0) {
    LOG_ERROR("Failed to create keypad thread");
    running = false;
    return HAMPOD_ERROR;
  }

  LOG_INFO(
      "Keypad system initialized (hold threshold: %dms, poll interval: %dms)",
      hold_threshold_ms, poll_interval_ms);
  return HAMPOD_OK;
}

void keypad_shutdown(void) {
  if (!running) {
    return;
  }

  LOG_INFO("Shutting down keypad system...");

  // Signal thread to stop
  running = false;

  // Wait for thread to finish
  pthread_join(keypad_thread, NULL);

  LOG_INFO("Keypad system shutdown complete");
}

bool keypad_is_running(void) { return running; }

// ============================================================================
// Public API - Callback Registration
// ============================================================================

void keypad_register_callback(KeypadCallback callback) {
  user_callback = callback;

  if (callback != NULL) {
    LOG_INFO("Keypad callback registered");
  } else {
    LOG_INFO("Keypad callback unregistered");
  }
}

// ============================================================================
// Public API - Configuration
// ============================================================================

void keypad_set_hold_threshold(int ms) {
  if (ms > 0) {
    hold_threshold_ms = ms;
    LOG_INFO("Keypad hold threshold set to %dms", ms);
  }
}

void keypad_set_poll_interval(int ms) {
  if (ms > 0) {
    poll_interval_ms = ms;
    LOG_INFO("Keypad poll interval set to %dms", ms);
  }
}
