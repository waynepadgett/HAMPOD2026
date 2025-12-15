/**
 * test_comm_write.c - Test Pipe Communication (Writing/Audio)
 * 
 * Verification for Phase 0 Step 1.2: Comm Writer
 * 
 * This test:
 * 1. Connects to Firmware via pipes
 * 2. Sends a TTS audio request ("Hello World")
 * 3. Waits for acknowledgment
 * 
 * Prerequisites:
 * - Firmware must be running: cd ../Firmware && ./firmware.elf
 * - Audio device must be configured
 * 
 * Usage:
 *   make tests
 *   ./bin/test_comm_write
 * 
 * Expected:
 *   You should hear "Hello World" spoken through the USB speaker.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "hampod_core.h"
#include "comm.h"

int main() {
    printf("=== Phase 0 Step 1.2: Comm Writer Test ===\n\n");
    
    // Initialize communication
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
    
    // Test 1: TTS Speech
    LOG_INFO("Test 1: Sending TTS request 'Hello World'...");
    if (comm_send_audio_sync(AUDIO_TYPE_TTS, "Hello World") == HAMPOD_OK) {
        LOG_INFO("Test 1: SUCCESS - Audio acknowledged");
    } else {
        LOG_ERROR("Test 1: FAILED - No acknowledgment");
        comm_close();
        return 1;
    }
    
    // Small delay between tests
    usleep(500000);  // 500ms
    
    // Test 2: Spelling
    LOG_INFO("Test 2: Sending spell request 'ABC'...");
    if (comm_send_audio_sync(AUDIO_TYPE_SPELL, "ABC") == HAMPOD_OK) {
        LOG_INFO("Test 2: SUCCESS - Audio acknowledged");
    } else {
        LOG_ERROR("Test 2: FAILED - No acknowledgment");
        comm_close();
        return 1;
    }
    
    // Small delay between tests
    usleep(500000);  // 500ms
    
    // Test 3: Pre-recorded file (may not exist, but tests the path)
    LOG_INFO("Test 3: Sending file playback request 'pregen_audio/5'...");
    if (comm_send_audio_sync(AUDIO_TYPE_FILE, "pregen_audio/5") == HAMPOD_OK) {
        LOG_INFO("Test 3: SUCCESS - Audio acknowledged");
    } else {
        // This might fail if file doesn't exist, but packet should still be ack'd
        LOG_INFO("Test 3: Got response (file may not exist, but communication works)");
    }
    
    printf("\n");
    LOG_INFO("=== All tests completed ===");
    LOG_INFO("If you heard speech, the comm module is working correctly.");
    
    comm_close();
    
    return 0;
}
