/**
 * test_speech_queue.c - Test Non-blocking Speech Queue
 * 
 * Verification for Phase 0 Step 2.1: Speech Queue
 * 
 * This test:
 * 1. Initializes comm and speech systems
 * 2. Quickly queues multiple speech items
 * 3. Verifies they play sequentially without blocking
 * 
 * Prerequisites:
 * - Firmware must be running: cd ../Firmware && ./firmware.elf
 * 
 * Usage:
 *   make tests
 *   ./bin/test_speech_queue
 * 
 * Expected:
 *   You should hear "One", "Two", "Three" spoken sequentially.
 *   The queue calls return immediately (non-blocking).
 */

#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include "hampod_core.h"
#include "comm.h"
#include "speech.h"

// Measure elapsed time in milliseconds
long elapsed_ms(struct timespec* start) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return (now.tv_sec - start->tv_sec) * 1000 + 
           (now.tv_nsec - start->tv_nsec) / 1000000;
}

int main() {
    printf("=== Phase 0 Step 2.1: Speech Queue Test ===\n\n");
    
    // Initialize communication
    LOG_INFO("Initializing communication...");
    if (comm_init() != HAMPOD_OK) {
        LOG_ERROR("Failed to connect to Firmware");
        LOG_ERROR("Make sure Firmware is running: cd ../Firmware && ./firmware.elf");
        return 1;
    }
    
    // Wait for Firmware ready signal
    if (comm_wait_ready() != HAMPOD_OK) {
        LOG_ERROR("Firmware not ready");
        comm_close();
        return 1;
    }
    
    LOG_INFO("Connected to Firmware!");
    
    // Initialize speech system
    LOG_INFO("Initializing speech system...");
    if (speech_init() != HAMPOD_OK) {
        LOG_ERROR("Failed to initialize speech system");
        comm_close();
        return 1;
    }
    
    printf("\n");
    LOG_INFO("=== Test 1: Non-blocking Queue ===");
    
    struct timespec start_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    // Queue multiple items quickly
    speech_say_text("One");
    LOG_INFO("Queued 'One' at %ldms (queue size: %d)", elapsed_ms(&start_time), speech_queue_size());
    
    speech_say_text("Two");
    LOG_INFO("Queued 'Two' at %ldms (queue size: %d)", elapsed_ms(&start_time), speech_queue_size());
    
    speech_say_text("Three");
    LOG_INFO("Queued 'Three' at %ldms (queue size: %d)", elapsed_ms(&start_time), speech_queue_size());
    
    // Check that queueing was fast (should be < 50ms total)
    long queue_time = elapsed_ms(&start_time);
    if (queue_time < 100) {
        LOG_INFO("Test 1 PASS: All items queued in %ldms (non-blocking)", queue_time);
    } else {
        LOG_ERROR("Test 1 FAIL: Queueing took %ldms (expected < 100ms)", queue_time);
    }
    
    printf("\n");
    LOG_INFO("=== Test 2: Wait for Completion ===");
    LOG_INFO("Waiting for speech to complete...");
    
    speech_wait_complete();
    
    long total_time = elapsed_ms(&start_time);
    LOG_INFO("All speech completed in %ldms", total_time);
    
    // Speech should take a few seconds (TTS + playback for 3 phrases)
    if (total_time > 1000) {
        LOG_INFO("Test 2 PASS: Speech took reasonable time");
    } else {
        LOG_INFO("Test 2 NOTE: Speech completed very quickly (%ldms)", total_time);
    }
    
    printf("\n");
    LOG_INFO("=== Test 3: Clear Queue ===");
    
    // Queue items but clear before they play
    speech_say_text("This should not be spoken");
    speech_say_text("Neither should this");
    LOG_INFO("Queued 2 items (queue size: %d)", speech_queue_size());
    
    speech_clear_queue();
    LOG_INFO("Cleared queue (queue size: %d)", speech_queue_size());
    
    if (speech_queue_size() == 0) {
        LOG_INFO("Test 3 PASS: Queue cleared successfully");
    } else {
        LOG_ERROR("Test 3 FAIL: Queue not empty after clear");
    }
    
    // Give a moment for any in-flight audio
    usleep(500000);
    
    printf("\n");
    LOG_INFO("=== All tests completed ===");
    LOG_INFO("If you heard 'One', 'Two', 'Three' (and NOT the cleared items), tests passed!");
    
    // Shutdown
    speech_shutdown();
    comm_close();
    
    return 0;
}
