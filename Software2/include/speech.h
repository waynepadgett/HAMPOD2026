/**
 * speech.h - Non-blocking Speech System
 * 
 * Provides a thread-safe speech queue that processes audio requests
 * in the background, allowing the main thread to continue without
 * waiting for speech to complete.
 * 
 * Usage:
 *   speech_init();                    // Start speech thread
 *   speech_say_text("Hello World");   // Queue speech (non-blocking)
 *   speech_say_text("Goodbye");       // Queue another (non-blocking)
 *   // ... speech plays in background ...
 *   speech_shutdown();                // Stop speech thread
 * 
 * Part of Phase 0: Core Infrastructure (Step 2.1)
 */

#ifndef SPEECH_H
#define SPEECH_H

#include "hampod_core.h"

// ============================================================================
// Initialization & Cleanup
// ============================================================================

/**
 * Initialize the speech system.
 * 
 * Starts a background thread that processes queued speech requests.
 * Must be called after comm_init() and comm_wait_ready().
 * 
 * @return HAMPOD_OK on success, HAMPOD_ERROR on failure
 */
int speech_init(void);

/**
 * Shutdown the speech system.
 * 
 * Signals the speech thread to stop and waits for it to finish.
 * Any queued speech will be discarded.
 */
void speech_shutdown(void);

/**
 * Check if speech system is running.
 * @return true if speech thread is active
 */
bool speech_is_running(void);

// ============================================================================
// Speech Queue API
// ============================================================================

/**
 * Queue text for speech synthesis (non-blocking).
 * 
 * The text is added to a queue and spoken in the background.
 * This call returns immediately.
 * 
 * @param text The text to speak
 * @return HAMPOD_OK on success, HAMPOD_ERROR if queue is full
 */
int speech_say_text(const char* text);

/**
 * Queue text to be spelled out character by character (non-blocking).
 * 
 * @param text The text to spell
 * @return HAMPOD_OK on success, HAMPOD_ERROR if queue is full
 */
int speech_spell_text(const char* text);

/**
 * Queue an audio file for playback (non-blocking).
 * 
 * @param filepath Path to the audio file (relative to Firmware directory)
 * @return HAMPOD_OK on success, HAMPOD_ERROR if queue is full
 */
int speech_play_file(const char* filepath);

/**
 * Wait for all queued speech to complete (blocking).
 * 
 * Blocks until the speech queue is empty.
 */
void speech_wait_complete(void);

/**
 * Clear the speech queue, discarding all pending items.
 */
void speech_clear_queue(void);

/**
 * Get the number of items currently in the speech queue.
 * @return Number of queued speech items
 */
int speech_queue_size(void);

// ============================================================================
// Speech Configuration
// ============================================================================

/**
 * Set maximum queue size.
 * Default is 32 items.
 * 
 * @param size Maximum number of items in the queue
 */
void speech_set_max_queue_size(int size);

#endif // SPEECH_H
