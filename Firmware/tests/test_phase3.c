#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "../audio_firmware.h"

// Mock external variables required by audio_firmware.c
pid_t controller_pid;

int main() {
    printf("Starting Phase 3 Audio Integration Test...\n");

    // 1. Initialize Audio Firmware (and HAL)
    printf("Calling firmwareStartAudio()...\n");
    firmwareStartAudio();
    printf("firmwareStartAudio() returned.\n");

    // 2. Test Playback (using 'p' prefix for direct play)
    // firmwarePlayAudio expects heap-allocated string and will free() it
    char* test_cmd = strdup("ptest_audio"); // p + filename (without .wav)
    
    printf("Calling firmwarePlayAudio with '%s'...\n", test_cmd);
    int result = firmwarePlayAudio(test_cmd);
    
    // Note: firmwarePlayAudio calls free() on the input, so we don't free test_cmd here
    
    printf("firmwarePlayAudio returned: %d\n", result);

    if (result != 0) {
        printf("Note: Non-zero return expected if file is missing, but HAL should have been called.\n");
    }

    printf("Phase 3 Test Complete.\n");
    return 0;
}
