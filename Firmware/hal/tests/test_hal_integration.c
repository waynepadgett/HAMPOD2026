/**
 * @file test_hal_integration.c
 * @brief Integration test for Keypad + Speech + Audio
 * 
 * This test verifies the complete loop:
 * 1. Read key from USB keypad (HAL)
 * 2. Convert key to word
 * 3. Generate speech WAV (text2wave)
 * 4. Play speech WAV (HAL)
 * 
 * Compile: make test_integration
 * Run: sudo ./test_integration
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "hal_keypad.h"
#include "hal_audio.h"

static volatile int running = 1;

void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\nShutting down...\n");
        running = 0;
    }
}

/* Map key character to spoken word */
const char* get_spoken_word(char key) {
    switch(key) {
        case '0': return "zero";
        case '1': return "one";
        case '2': return "two";
        case '3': return "three";
        case '4': return "four";
        case '5': return "five";
        case '6': return "six";
        case '7': return "seven";
        case '8': return "eight";
        case '9': return "nine";
        case 'A': return "A";
        case 'B': return "B";
        case 'C': return "C";
        case 'D': return "D";
        case '*': return "star";
        case '#': return "pound";
        case 'X': return "num lock";
        case 'Y': return "backspace";
        default:  return "unknown";
    }
}

int main() {
    printf("=== HAMPOD Integration Test: Keypad + Speech + Audio ===\n");
    printf("Press Ctrl+C to exit\n\n");
    
    /* Set up signal handler */
    signal(SIGINT, signal_handler);
    
    /* Initialize HALs */
    printf("Initializing Keypad HAL...\n");
    if (hal_keypad_init() != 0) {
        fprintf(stderr, "ERROR: Failed to initialize keypad\n");
        return 1;
    }
    
    printf("Initializing Audio HAL...\n");
    if (hal_audio_init() != 0) {
        fprintf(stderr, "ERROR: Failed to initialize audio\n");
        hal_keypad_cleanup();
        return 1;
    }
    
    printf("\nSystem Ready!\n");
    printf("Press any key on the keypad. The Pi should speak the key name.\n\n");
    
    char command[512];
    const char* output_file = "/tmp/hampod_speak.wav";
    
    while (running) {
        KeypadEvent event = hal_keypad_read();
        
        if (event.valid) {
            const char* word = get_spoken_word(event.key);
            printf("Key: '%c' -> Speaking: \"%s\"\n", event.key, word);
            
            /* Generate speech using text2wave */
            /* echo "word" | text2wave -o /tmp/hampod_speak.wav */
            snprintf(command, sizeof(command), "echo \"%s\" | text2wave -o %s", word, output_file);
            int ret = system(command);
            
            if (ret == 0) {
                /* Play the generated file */
                hal_audio_play_file(output_file);
            } else {
                fprintf(stderr, "Error generating speech (is festival/text2wave installed?)\n");
            }
        }
        
        /* Small delay to avoid CPU spinning */
        usleep(10000);  /* 10ms */
    }
    
    /* Cleanup */
    printf("\nCleaning up...\n");
    hal_keypad_cleanup();
    hal_audio_cleanup();
    remove(output_file);
    
    return 0;
}
