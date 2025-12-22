/**
 * @file radio.c
 * @brief Radio control module implementation using Hamlib
 * 
 * Part of Phase 1: Frequency Mode Implementation
 */

#include "radio.h"
#include "config.h"
#include "hampod_core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <hamlib/rig.h>

// ============================================================================
// Module State
// ============================================================================

static RIG *g_rig = NULL;
static bool g_connected = false;
static pthread_mutex_t g_rig_mutex = PTHREAD_MUTEX_INITIALIZER;

// Polling state
static pthread_t g_poll_thread;
static volatile bool g_polling_active = false;
static radio_freq_change_callback g_freq_callback = NULL;

// Polling parameters
#define POLL_INTERVAL_MS 100
#define DEBOUNCE_TIME_MS 1000

// ============================================================================
// Initialization & Cleanup
// ============================================================================

int radio_init(void) {
    pthread_mutex_lock(&g_rig_mutex);
    
    if (g_connected) {
        pthread_mutex_unlock(&g_rig_mutex);
        fprintf(stderr, "radio_init: Already connected\n");
        return -1;
    }
    
    int model = config_get_radio_model();
    const char *device = config_get_radio_device();
    int baud = config_get_radio_baud();
    
    DEBUG_PRINT("radio_init: model=%d device=%s baud=%d\n", model, device, baud);
    
    // Initialize Hamlib rig
    g_rig = rig_init(model);
    if (!g_rig) {
        fprintf(stderr, "radio_init: rig_init failed for model %d\n", model);
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
    // Configure serial port
    strncpy(g_rig->state.rigport.pathname, device, 
            sizeof(g_rig->state.rigport.pathname) - 1);
    g_rig->state.rigport.parm.serial.rate = baud;
    
    // Open connection
    int retcode = rig_open(g_rig);
    if (retcode != RIG_OK) {
        fprintf(stderr, "radio_init: rig_open failed: %s\n", rigerror(retcode));
        rig_cleanup(g_rig);
        g_rig = NULL;
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }
    
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
    (void)arg;  // Unused
    
    double last_freq = -1.0;
    double stable_freq = -1.0;
    int stable_count = 0;
    int debounce_ticks = DEBOUNCE_TIME_MS / POLL_INTERVAL_MS;
    
    DEBUG_PRINT("polling_thread: Started (debounce=%d ticks)\n", debounce_ticks);
    
    while (g_polling_active) {
        double current_freq = radio_get_frequency();
        
        if (current_freq > 0) {
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

bool radio_is_polling(void) {
    return g_polling_active;
}
