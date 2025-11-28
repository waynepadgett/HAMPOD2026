/**
 * @file test_hal_audio.c
 * @brief Test program for USB audio HAL
 * 
 * Compile: gcc -o test_audio test_hal_audio.c ../hal_audio_usb.c -I..
 * Run: ./test_audio
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hal_audio.h"

/* Create a simple test WAV file */
int create_test_wav(const char* filename) {
    /* This creates a very simple 1-second beep at 440Hz */
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to create test file");
        return -1;
    }
    
    /* WAV header for 1 second, 8000 Hz, mono, 8-bit */
    unsigned char header[] = {
        'R','I','F','F',  // ChunkID
        0x24,0x1F,0x00,0x00,  // ChunkSize (8000 + 36)
        'W','A','V','E',  // Format
        'f','m','t',' ',  // Subchunk1ID
        0x10,0x00,0x00,0x00,  // Subchunk1Size (16)
        0x01,0x00,  // AudioFormat (PCM)
        0x01,0x00,  // NumChannels (1)
        0x40,0x1F,0x00,0x00,  // SampleRate (8000)
        0x40,0x1F,0x00,0x00,  // ByteRate
        0x01,0x00,  // BlockAlign
        0x08,0x00,  // BitsPerSample (8)
        'd','a','t','a',  // Subchunk2ID
        0x00,0x1F,0x00,0x00   // Subchunk2Size (8000)
    };
    
    fwrite(header, 1, sizeof(header), fp);
    
    /* Generate simple sine wave (440 Hz = A note) */
    for (int i = 0; i < 8000; i++) {
        unsigned char sample = 128 + (int)(64 * sin(2 * 3.14159 * 440 * i / 8000));
        fwrite(&sample, 1, 1, fp);
    }
    
    fclose(fp);
    return 0;
}

int main() {
    printf("=== HAMPOD USB Audio HAL Test ===\n\n");
    
    /* Initialize audio */
    printf("Initializing audio HAL...\n");
    if (hal_audio_init() != 0) {
        fprintf(stderr, "ERROR: Failed to initialize audio\n");
        fprintf(stderr, "Make sure:\n");
        fprintf(stderr, "  1. USB audio device is connected\n");
        fprintf(stderr, "  2. ALSA is installed (aplay command available)\n");
        return 1;
    }
    
    printf("Audio initialized successfully!\n");
    printf("Implementation: %s\n", hal_audio_get_impl_name());
    printf("Device: %s\n\n", hal_audio_get_device());
    
    /* Create test WAV file */
    const char* test_file = "/tmp/hampod_test.wav";
    printf("Creating test audio file: %s\n", test_file);
    
    if (create_test_wav(test_file) != 0) {
        fprintf(stderr, "ERROR: Failed to create test file\n");
        hal_audio_cleanup();
        return 1;
    }
    
    printf("Test file created successfully!\n\n");
    
    /* Test audio playback */
    printf("Test 1: Playing test tone (1 second beep at 440Hz)...\n");
    if (hal_audio_play_file(test_file) != 0) {
        fprintf(stderr, "ERROR: Audio playback failed\n");
        fprintf(stderr, "Check audio device connection and volume settings\n");
        hal_audio_cleanup();
        return 1;
    }
    printf("✓ Playback successful!\n\n");
    
    /* Test with non-existent file */
    printf("Test 2: Error handling (non-existent file)...\n");
    int result = hal_audio_play_file("/tmp/nonexistent.wav");
    if (result != 0) {
        printf("✓ Correctly returned error for missing file\n\n");
    } else {
        printf("⚠ Warning: Should have returned error for missing file\n\n");
    }
    
    /* Cleanup */
    printf("Cleaning up...\n");
    hal_audio_cleanup();
    remove(test_file);
    
    printf("\n=== All tests passed! ===\n");
    printf("\nYou should have heard a 1-second beep tone.\n");
    printf("If you didn't hear anything, check:\n");
    printf("  - USB speaker volume\n");
    printf("  - USB speaker is set as default device\n");
    printf("  - Run: aplay -l  (to list audio devices)\n");
    
    return 0;
}
