/**
 * @file hal_tts_piper.c
 * @brief Piper TTS implementation of the TTS HAL (Persistent Mode)
 * 
 * Uses Piper with a persistent pipeline for zero-latency speech synthesis.
 * The pipeline is started once at init and text is written to it for immediate playback.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "hal_tts.h"
#include "hal_audio.h"

#ifndef PIPER_MODEL_PATH
#define PIPER_MODEL_PATH "models/en_US-lessac-low.onnx"
#endif

#ifndef PIPER_SPEED
#define PIPER_SPEED "1.0"
#endif

static FILE *persistent_pipe = NULL;
static int initialized = 0;

int hal_tts_init(void) {
    if (initialized) return 0;
    
    /* 1. Check if 'piper' is installed */
    if (system("which piper > /dev/null 2>&1") != 0) {
        fprintf(stderr, "\n");
        fprintf(stderr, "===================================================\n");
        fprintf(stderr, "ERROR: Piper TTS is not installed!\n");
        fprintf(stderr, "===================================================\n");
        fprintf(stderr, "To install Piper and the voice model, run:\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "    ./Documentation/scripts/install_piper.sh\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "Or build with Festival instead:\n");
        fprintf(stderr, "    make TTS_ENGINE=festival\n");
        fprintf(stderr, "===================================================\n");
        return -1;
    }

    /* 2. Check if model file exists */
    if (access(PIPER_MODEL_PATH, F_OK) != 0) {
        fprintf(stderr, "\n");
        fprintf(stderr, "===================================================\n");
        fprintf(stderr, "ERROR: Piper voice model not found!\n");
        fprintf(stderr, "Expected: %s\n", PIPER_MODEL_PATH);
        fprintf(stderr, "===================================================\n");
        fprintf(stderr, "To download the model, run:\n");
        fprintf(stderr, "    ./Documentation/scripts/install_piper.sh\n");
        fprintf(stderr, "===================================================\n");
        return -1;
    }

    /* 3. Start persistent pipeline */
    char command[1024];
    const char* audio_dev = hal_audio_get_device();
    
    /* Low quality model uses 16kHz sample rate */
    snprintf(command, sizeof(command), 
             "piper --model %s --length_scale %s --output_raw 2>/dev/null | aplay -D %s -r 16000 -f S16_LE -t raw - 2>/dev/null",
             PIPER_MODEL_PATH, PIPER_SPEED, audio_dev);
    
    persistent_pipe = popen(command, "w");
    if (persistent_pipe == NULL) {
        fprintf(stderr, "HAL TTS: Failed to start Piper pipeline\n");
        return -1;
    }
    
    printf("HAL TTS: Piper initialized (model=%s, speed=%s)\n", PIPER_MODEL_PATH, PIPER_SPEED);
    initialized = 1;
    return 0;
}

int hal_tts_speak(const char* text, const char* output_file) {
    (void)output_file; /* Ignored in persistent mode */
    
    if (!initialized || persistent_pipe == NULL) {
        fprintf(stderr, "HAL TTS: Not initialized\n");
        return -1;
    }
    
    /* Write text to the persistent pipeline */
    fprintf(persistent_pipe, "%s\n", text);
    fflush(persistent_pipe);
    
    return 0;
}

void hal_tts_cleanup(void) {
    if (persistent_pipe) {
        pclose(persistent_pipe);
        persistent_pipe = NULL;
    }
    initialized = 0;
    printf("HAL TTS: Piper cleaned up\n");
}

const char* hal_tts_get_impl_name(void) {
    return "Piper (Persistent)";
}
