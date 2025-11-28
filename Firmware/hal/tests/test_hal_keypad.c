/**
 * @file test_hal_keypad.c
 * @brief Test program for USB keypad HAL
 * 
 * Compile: gcc -o test_keypad test_hal_keypad.c ../hal_keypad_usb.c -I..
 * Run: ./test_keypad
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include "hal_keypad.h"

static volatile int running = 1;

void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\nShutting down...\n");
        running = 0;
    }
}

int main() {
    printf("=== HAMPOD USB Keypad HAL Test ===\n");
    printf("Press Ctrl+C to exit\n\n");
    
    /* Set up signal handler */
    signal(SIGINT, signal_handler);
    
    /* Initialize keypad */
    printf("Initializing keypad HAL...\n");
    if (hal_keypad_init() != 0) {
        fprintf(stderr, "ERROR: Failed to initialize keypad\n");
        fprintf(stderr, "Make sure:\n");
        fprintf(stderr, "  1. USB keypad is connected\n");
        fprintf(stderr, "  2. You have read permissions for /dev/input devices\n");
        fprintf(stderr, "  3. Run as: sudo ./test_keypad  (if needed)\n");
        return 1;
    }
    
    printf("Keypad initialized successfully!\n");
    printf("Implementation: %s\n\n", hal_keypad_get_impl_name());
    printf("Waiting for key presses...\n");
    printf("(Try pressing keys 0-9, /=A, *=B, -=C, +=D, ENTER=#)\n\n");
    
    int key_count = 0;
    
    /* Main loop */
    while (running) {
        KeypadEvent event = hal_keypad_read();
        
        if (event.valid) {
            key_count++;
            printf("[%3d] Key pressed: '%c' (raw code: %d)\n", 
                   key_count, event.key, event.raw_code);
            fflush(stdout);
        }
        
        /* Small delay to avoid CPU spinning */
        usleep(10000);  /* 10ms */
    }
    
    /* Cleanup */
    printf("\nCleaning up...\n");
    hal_keypad_cleanup();
    
    printf("\nTest summary:\n");
    printf("  Total keys pressed: %d\n", key_count);
    printf("\nTest completed successfully!\n");
    
    return 0;
}
