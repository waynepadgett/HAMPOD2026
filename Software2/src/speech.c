/**
 * speech.c - Non-blocking Speech System Implementation
 * 
 * Implements a thread-safe speech queue using pthreads.
 * The speech thread runs in the background, dequeueing items
 * and sending them to Firmware via comm_send_audio_sync().
 * 
 * Part of Phase 0: Core Infrastructure (Step 2.1)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "speech.h"
#include "comm.h"

// ============================================================================
// Constants
// ============================================================================

#define DEFAULT_MAX_QUEUE_SIZE 32
#define MAX_TEXT_LENGTH 256

// ============================================================================
// Audio Packet Types
// ============================================================================

typedef struct {
    char type;                      // AUDIO_TYPE_TTS, AUDIO_TYPE_SPELL, AUDIO_TYPE_FILE
    char payload[MAX_TEXT_LENGTH];  // Text or file path
} SpeechItem;

// ============================================================================
// Queue Data Structure (Circular Buffer)
// ============================================================================

typedef struct {
    SpeechItem* items;       // Array of items
    int capacity;            // Maximum size
    int head;                // Index of first item
    int tail;                // Index of next empty slot
    int count;               // Current number of items
    pthread_mutex_t mutex;   // Protects queue access
    pthread_cond_t not_empty;  // Signaled when item added
    pthread_cond_t not_full;   // Signaled when item removed
} SpeechQueue;

// ============================================================================
// Module State
// ============================================================================

static SpeechQueue queue;
static pthread_t speech_thread;
static volatile bool running = false;
static int max_queue_size = DEFAULT_MAX_QUEUE_SIZE;

// ============================================================================
// Private Functions
// ============================================================================

static int queue_init(int capacity) {
    queue.items = (SpeechItem*)malloc(capacity * sizeof(SpeechItem));
    if (queue.items == NULL) {
        LOG_ERROR("Failed to allocate speech queue");
        return HAMPOD_ERROR;
    }
    
    queue.capacity = capacity;
    queue.head = 0;
    queue.tail = 0;
    queue.count = 0;
    
    pthread_mutex_init(&queue.mutex, NULL);
    pthread_cond_init(&queue.not_empty, NULL);
    pthread_cond_init(&queue.not_full, NULL);
    
    return HAMPOD_OK;
}

static void queue_destroy(void) {
    pthread_mutex_lock(&queue.mutex);
    if (queue.items != NULL) {
        free(queue.items);
        queue.items = NULL;
    }
    queue.count = 0;
    pthread_mutex_unlock(&queue.mutex);
    
    pthread_mutex_destroy(&queue.mutex);
    pthread_cond_destroy(&queue.not_empty);
    pthread_cond_destroy(&queue.not_full);
}

static int queue_push(char type, const char* payload) {
    pthread_mutex_lock(&queue.mutex);
    
    // Wait if queue is full (with timeout to check running flag)
    while (queue.count >= queue.capacity && running) {
        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_nsec += 100000000;  // 100ms timeout
        if (timeout.tv_nsec >= 1000000000) {
            timeout.tv_sec += 1;
            timeout.tv_nsec -= 1000000000;
        }
        pthread_cond_timedwait(&queue.not_full, &queue.mutex, &timeout);
    }
    
    if (!running) {
        pthread_mutex_unlock(&queue.mutex);
        return HAMPOD_ERROR;
    }
    
    if (queue.count >= queue.capacity) {
        pthread_mutex_unlock(&queue.mutex);
        LOG_ERROR("Speech queue is full");
        return HAMPOD_ERROR;
    }
    
    // Add item to tail
    SpeechItem* item = &queue.items[queue.tail];
    item->type = type;
    strncpy(item->payload, payload, MAX_TEXT_LENGTH - 1);
    item->payload[MAX_TEXT_LENGTH - 1] = '\0';
    
    queue.tail = (queue.tail + 1) % queue.capacity;
    queue.count++;
    
    // Signal that queue is not empty
    pthread_cond_signal(&queue.not_empty);
    
    pthread_mutex_unlock(&queue.mutex);
    
    LOG_DEBUG("Queued speech: type='%c', payload='%s' (queue size=%d)", 
              type, payload, queue.count);
    
    return HAMPOD_OK;
}

static int queue_pop(SpeechItem* item) {
    pthread_mutex_lock(&queue.mutex);
    
    // Wait if queue is empty (with timeout to check running flag)
    while (queue.count == 0 && running) {
        struct timespec timeout;
        clock_gettime(CLOCK_REALTIME, &timeout);
        timeout.tv_nsec += 100000000;  // 100ms timeout
        if (timeout.tv_nsec >= 1000000000) {
            timeout.tv_sec += 1;
            timeout.tv_nsec -= 1000000000;
        }
        pthread_cond_timedwait(&queue.not_empty, &queue.mutex, &timeout);
    }
    
    if (!running && queue.count == 0) {
        pthread_mutex_unlock(&queue.mutex);
        return HAMPOD_ERROR;
    }
    
    if (queue.count == 0) {
        pthread_mutex_unlock(&queue.mutex);
        return HAMPOD_NOT_FOUND;
    }
    
    // Remove item from head
    *item = queue.items[queue.head];
    queue.head = (queue.head + 1) % queue.capacity;
    queue.count--;
    
    // Signal that queue is not full
    pthread_cond_signal(&queue.not_full);
    
    pthread_mutex_unlock(&queue.mutex);
    
    return HAMPOD_OK;
}

// ============================================================================
// Speech Thread
// ============================================================================

static void* speech_thread_func(void* arg) {
    (void)arg;
    
    LOG_INFO("Speech thread started");
    
    while (running) {
        SpeechItem item;
        
        if (queue_pop(&item) != HAMPOD_OK) {
            // Queue empty or shutting down
            continue;
        }
        
        LOG_DEBUG("Speaking: type='%c', payload='%s'", item.type, item.payload);
        
        // Send to Firmware (non-blocking, don't wait for response)
        // Note: We use non-blocking send because keypad thread also reads responses
        if (comm_send_audio(item.type, item.payload) != HAMPOD_OK) {
            LOG_ERROR("Failed to send audio: %s", item.payload);
        }
        
        // Brief delay to let audio play (simple throttle)
        usleep(100000);  // 100ms
    }
    
    LOG_INFO("Speech thread exiting");
    
    return NULL;
}

// ============================================================================
// Public API - Initialization
// ============================================================================

int speech_init(void) {
    if (running) {
        LOG_ERROR("Speech system already running");
        return HAMPOD_ERROR;
    }
    
    LOG_INFO("Initializing speech system...");
    
    // Initialize queue
    if (queue_init(max_queue_size) != HAMPOD_OK) {
        return HAMPOD_ERROR;
    }
    
    // Start speech thread
    running = true;
    if (pthread_create(&speech_thread, NULL, speech_thread_func, NULL) != 0) {
        LOG_ERROR("Failed to create speech thread");
        running = false;
        queue_destroy();
        return HAMPOD_ERROR;
    }
    
    LOG_INFO("Speech system initialized");
    return HAMPOD_OK;
}

void speech_shutdown(void) {
    if (!running) {
        return;
    }
    
    LOG_INFO("Shutting down speech system...");
    
    // Signal thread to stop
    running = false;
    
    // Wake up the thread if it's waiting
    pthread_cond_broadcast(&queue.not_empty);
    pthread_cond_broadcast(&queue.not_full);
    
    // Wait for thread to finish
    pthread_join(speech_thread, NULL);
    
    // Clean up queue
    queue_destroy();
    
    LOG_INFO("Speech system shutdown complete");
}

bool speech_is_running(void) {
    return running;
}

// ============================================================================
// Public API - Queue Operations
// ============================================================================

int speech_say_text(const char* text) {
    if (text == NULL) {
        LOG_ERROR("speech_say_text: NULL text");
        return HAMPOD_ERROR;
    }
    return queue_push(AUDIO_TYPE_TTS, text);
}

int speech_spell_text(const char* text) {
    if (text == NULL) {
        LOG_ERROR("speech_spell_text: NULL text");
        return HAMPOD_ERROR;
    }
    return queue_push(AUDIO_TYPE_SPELL, text);
}

int speech_play_file(const char* filepath) {
    if (filepath == NULL) {
        LOG_ERROR("speech_play_file: NULL filepath");
        return HAMPOD_ERROR;
    }
    return queue_push(AUDIO_TYPE_FILE, filepath);
}

void speech_wait_complete(void) {
    // Poll queue size until empty
    while (speech_queue_size() > 0 && running) {
        usleep(50000);  // 50ms
    }
}

void speech_clear_queue(void) {
    pthread_mutex_lock(&queue.mutex);
    queue.head = 0;
    queue.tail = 0;
    queue.count = 0;
    pthread_cond_broadcast(&queue.not_full);
    pthread_mutex_unlock(&queue.mutex);
    
    LOG_INFO("Speech queue cleared");
}

int speech_queue_size(void) {
    pthread_mutex_lock(&queue.mutex);
    int size = queue.count;
    pthread_mutex_unlock(&queue.mutex);
    return size;
}

// ============================================================================
// Public API - Configuration
// ============================================================================

void speech_set_max_queue_size(int size) {
    if (!running && size > 0) {
        max_queue_size = size;
    }
}
