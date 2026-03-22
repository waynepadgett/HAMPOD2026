/**
 * @file radio.c
 * @brief Radio control module implementation using Hamlib
 *
 * Part of Phase 1: Frequency Mode Implementation
 */

#include "radio.h"
#include "config.h"
#include "hampod_core.h"

#include <fcntl.h>
#include <hamlib/rig.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// ============================================================================
// State Variables
// ============================================================================

RIG *g_rig = NULL;
bool g_connected = false;
pthread_mutex_t g_rig_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_radio_debug_mode = false;

// Polling state
static pthread_t g_poll_thread;
static volatile bool g_polling_active = false;
static radio_freq_change_callback g_freq_callback = NULL;

// Reconnect state
static pthread_t g_reconnect_thread;
static volatile bool g_reconnect_active = false;
static radio_connect_callback g_connect_callback = NULL;
static radio_disconnect_callback g_disconnect_callback = NULL;

// Polling parameters
#define POLL_INTERVAL_MS 100
#define DEBOUNCE_TIME_MS 1000
#define DISCONNECT_THRESHOLD                                                   \
  3 // consecutive failures before declaring disconnect
#define RECONNECT_INTERVAL_SEC 1 // seconds between reconnect attempts

// ============================================================================
// Initialization & Cleanup
// ============================================================================

int radio_init(bool debug_mode) {
  pthread_mutex_lock(&g_rig_mutex);

  // Store debug mode for auto-reconnects
  g_radio_debug_mode = debug_mode;

  if (g_connected) {
    pthread_mutex_unlock(&g_rig_mutex);
    fprintf(stderr, "radio_init: Already connected\n");
    return -1;
  }
  pthread_mutex_unlock(&g_rig_mutex);

  int model = config_get_radio_model();
  const char *device = config_get_radio_device();
  int baud = config_get_radio_baud();

  DEBUG_PRINT("radio_init: model=%d device=%s baud=%d\n", model, device, baud);

  // Initialize Hamlib rig locally first
  RIG *temp_rig = rig_init(model);
  if (!temp_rig) {
    fprintf(stderr, "radio_init: rig_init failed for model %d\n", model);
    return -1;
  }

  // Silence Hamlib verbose output to prevent console spam
  // (It logs every polling event by default, printing 100+ lines/sec)
  if (!debug_mode) {
    rig_set_debug(RIG_DEBUG_NONE);
  } else {
    rig_set_debug(RIG_DEBUG_TRACE);
  }

  // Configure serial port
  strncpy(temp_rig->state.rigport.pathname, device,
          sizeof(temp_rig->state.rigport.pathname) - 1);
  temp_rig->state.rigport.parm.serial.rate = baud;

  // Dramatically reduce Hamlib timeout latency to prevent UI freezes on disconnect
  // Using explicit API token configuration so backends don't override with defaults
  token_t t_timeout = rig_token_lookup(temp_rig, "timeout");
  if (t_timeout != RIG_CONF_END) {
      rig_set_conf(temp_rig, t_timeout, "200");
  }
  token_t t_retry = rig_token_lookup(temp_rig, "retry");
  if (t_retry != RIG_CONF_END) {
      rig_set_conf(temp_rig, t_retry, "0");
  }

  // Open connection (can block for several seconds on timeout)
  // We do NOT hold the g_rig_mutex during this time to avoid blocking UI keys
  int retcode = rig_open(temp_rig);

  if (retcode == RIG_OK) {
    // Actively verify the radio is responding (powered on).
    // An active USB-serial adapter might succeed in rig_open, but will time out here.
    freq_t test_freq;
    retcode = rig_get_freq(temp_rig, RIG_VFO_CURR, &test_freq);
    if (retcode != RIG_OK) {
      DEBUG_PRINT("radio_init: rig_open succeeded, but radio isn't responding (off?).\n");
      rig_close(temp_rig); // Close since open succeeded
    }
  }

  // Re-acquire mutex to update global state
  pthread_mutex_lock(&g_rig_mutex);

  if (retcode != RIG_OK) {
    fprintf(stderr, "radio_init: Connection to radio failed: %s\n", rigerror(retcode));
    rig_cleanup(temp_rig);
    pthread_mutex_unlock(&g_rig_mutex);
    return -1;
  }

  g_rig = temp_rig;
  g_connected = true;
  DEBUG_PRINT("radio_init: Connected to radio\n");

  pthread_mutex_unlock(&g_rig_mutex);
  return 0;
}

void radio_cleanup(void) {
  // Stop polling first
  radio_stop_polling();

  pthread_mutex_lock(&g_rig_mutex);

  if (g_rig) {
    rig_close(g_rig);
    rig_cleanup(g_rig);
    g_rig = NULL;
  }

  g_connected = false;
  DEBUG_PRINT("radio_cleanup: Disconnected from radio\n");

  pthread_mutex_unlock(&g_rig_mutex);
}

bool radio_is_connected(void) {
  pthread_mutex_lock(&g_rig_mutex);
  bool connected = g_connected;
  pthread_mutex_unlock(&g_rig_mutex);
  return connected;
}

// ============================================================================
// Frequency Operations
// ============================================================================

double radio_get_frequency(void) {
  pthread_mutex_lock(&g_rig_mutex);

  if (!g_connected || !g_rig) {
    pthread_mutex_unlock(&g_rig_mutex);
    return -1.0;
  }

  freq_t freq;
  int retcode = rig_get_freq(g_rig, RIG_VFO_CURR, &freq);

  pthread_mutex_unlock(&g_rig_mutex);

  if (retcode != RIG_OK) {
    fprintf(stderr, "radio_get_frequency: %s\n", rigerror(retcode));
    return -1.0;
  }

  return (double)freq;
}

int radio_set_frequency(double freq_hz) {
  pthread_mutex_lock(&g_rig_mutex);

  if (!g_connected || !g_rig) {
    pthread_mutex_unlock(&g_rig_mutex);
    return -1;
  }

  int retcode = rig_set_freq(g_rig, RIG_VFO_CURR, (freq_t)freq_hz);

  pthread_mutex_unlock(&g_rig_mutex);

  if (retcode != RIG_OK) {
    fprintf(stderr, "radio_set_frequency: %s\n", rigerror(retcode));
    return -1;
  }

  DEBUG_PRINT("radio_set_frequency: Set to %.3f Hz\n", freq_hz);
  return 0;
}

// ============================================================================
// Radio Polling Thread
// ============================================================================

static void *polling_thread_func(void *arg) {
  (void)arg; // Unused

  double last_freq = -1.0;
  double stable_freq = -1.0;
  int stable_count = 0;
  int debounce_ticks = DEBOUNCE_TIME_MS / POLL_INTERVAL_MS;
  int fail_count = 0;

  DEBUG_PRINT("polling_thread: Started (debounce=%d ticks)\n", debounce_ticks);

  while (g_polling_active) {
    double current_freq = radio_get_frequency();

    if (current_freq > 0) {
      fail_count = 0; // Reset on success

      if (current_freq != last_freq) {
        // Frequency changed, reset debounce
        stable_freq = current_freq;
        stable_count = 0;
        last_freq = current_freq;
      } else {
        // Frequency stable
        stable_count++;

        if (stable_count == debounce_ticks && g_freq_callback) {
          // Debounce complete, announce
          DEBUG_PRINT("polling_thread: Stable at %.3f Hz\n", stable_freq);
          g_freq_callback(stable_freq);
        }
      }
    } else {
      // Query failed
      fail_count++;
      if (fail_count >= DISCONNECT_THRESHOLD) {
        printf("polling_thread: %d consecutive failures, radio disconnected\n",
               fail_count);

        // Directly close rig and mark disconnected (can't call radio_cleanup
        // here because it calls radio_stop_polling which would try to join
        // this thread on itself = deadlock)
        pthread_mutex_lock(&g_rig_mutex);
        if (g_rig) {
          rig_close(g_rig);
          rig_cleanup(g_rig);
          g_rig = NULL;
        }
        g_connected = false;
        pthread_mutex_unlock(&g_rig_mutex);

        // Stop this polling thread (reconnect thread will restart it)
        g_polling_active = false;

        // Notify via callback (after releasing locks)
        if (g_disconnect_callback) {
          g_disconnect_callback();
        }
        break;
      }
    }

    // Sleep for poll interval
    struct timespec ts = {0, POLL_INTERVAL_MS * 1000000};
    nanosleep(&ts, NULL);
  }

  DEBUG_PRINT("polling_thread: Stopped\n");
  return NULL;
}

int radio_start_polling(radio_freq_change_callback on_change) {
  if (g_polling_active) {
    fprintf(stderr, "radio_start_polling: Already polling\n");
    return -1;
  }

  if (!radio_is_connected()) {
    fprintf(stderr, "radio_start_polling: Radio not connected\n");
    return -1;
  }

  g_freq_callback = on_change;
  g_polling_active = true;

  int ret = pthread_create(&g_poll_thread, NULL, polling_thread_func, NULL);
  if (ret != 0) {
    fprintf(stderr, "radio_start_polling: pthread_create failed\n");
    g_polling_active = false;
    return -1;
  }

  DEBUG_PRINT("radio_start_polling: Started\n");
  return 0;
}

void radio_stop_polling(void) {
  if (!g_polling_active) {
    return;
  }

  g_polling_active = false;
  pthread_join(g_poll_thread, NULL);
  g_freq_callback = NULL;

  DEBUG_PRINT("radio_stop_polling: Stopped\n");
}

bool radio_is_polling(void) { return g_polling_active; }

// ============================================================================
// Auto-Reconnect Thread
// ============================================================================

static void *reconnect_thread_func(void *arg) {
  (void)arg;

  DEBUG_PRINT("reconnect_thread: Started\n");

  while (g_reconnect_active) {
    if (radio_is_connected()) {
      // Monitor for USB device disappearance (handles the case where
      // Hamlib's serial calls hang on a dead file descriptor, preventing
      // the polling thread's failure detection from triggering)
      const char *device = config_get_radio_device();
      if (access(device, F_OK) != 0) {
        printf(
            "reconnect_thread: USB device %s disappeared, forcing disconnect\n",
            device);

        // Stop polling first (it may be stuck in a blocking serial read)
        g_polling_active = false;

        // Close rig and mark disconnected
        pthread_mutex_lock(&g_rig_mutex);
        if (g_rig) {
          rig_close(g_rig);
          rig_cleanup(g_rig);
          g_rig = NULL;
        }
        g_connected = false;
        pthread_mutex_unlock(&g_rig_mutex);

        // Notify
        if (g_disconnect_callback) {
          g_disconnect_callback();
        }
      }
    } else {
      // Check if USB device exists before attempting (avoids Hamlib spam)
      const char *device = config_get_radio_device();
      if (access(device, F_OK) == 0) {
        DEBUG_PRINT("reconnect_thread: Device %s found, attempting connect\n",
                    device);

        if (radio_init(g_radio_debug_mode) == 0) {
          printf("reconnect_thread: Radio connected!\n");

          // Notify via callback
          if (g_connect_callback) {
            g_connect_callback();
          }
        } else {
          // Device exists but Hamlib can't open it.
          // Wait and let the loop naturally retry. 
          // The aggressive USBDEVFS_RESET has been removed because it causes
          // serial RS-232 adapters (like TS570) to disconnect and re-enumerate 
          // as a different node (e.g., ttyUSB1), permanently breaking the connection.
          DEBUG_PRINT("reconnect_thread: Device %s exists but init failed. Retrying on next cycle.\n", device);
        }
      }
    }

    // Sleep between attempts
    for (int i = 0; i < RECONNECT_INTERVAL_SEC * 10 && g_reconnect_active;
         i++) {
      struct timespec ts = {0, 100000000}; // 100ms
      nanosleep(&ts, NULL);
    }
  }

  DEBUG_PRINT("reconnect_thread: Stopped\n");
  return NULL;
}

int radio_start_reconnect(radio_connect_callback on_connect,
                          radio_disconnect_callback on_disconnect) {
  if (g_reconnect_active) {
    fprintf(stderr, "radio_start_reconnect: Already running\n");
    return -1;
  }

  g_connect_callback = on_connect;
  g_disconnect_callback = on_disconnect;
  g_reconnect_active = true;

  int ret =
      pthread_create(&g_reconnect_thread, NULL, reconnect_thread_func, NULL);
  if (ret != 0) {
    fprintf(stderr, "radio_start_reconnect: pthread_create failed\n");
    g_reconnect_active = false;
    return -1;
  }

  DEBUG_PRINT("radio_start_reconnect: Started\n");
  return 0;
}

void radio_stop_reconnect(void) {
  if (!g_reconnect_active) {
    return;
  }

  g_reconnect_active = false;
  pthread_join(g_reconnect_thread, NULL);
  g_connect_callback = NULL;
  g_disconnect_callback = NULL;

  DEBUG_PRINT("radio_stop_reconnect: Stopped\n");
}
