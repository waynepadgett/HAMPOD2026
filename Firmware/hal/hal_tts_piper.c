/**
 * @file hal_tts_piper.c
 * @brief Piper TTS implementation of the TTS HAL
 *
 * Routes Piper's raw PCM output through the audio HAL for playback.
 * This makes TTS interruptible and avoids audio device conflicts.
 */

#include "hal_audio.h"
#include "hal_tts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef PIPER_MODEL_PATH
#define PIPER_MODEL_PATH "models/en_US-lessac-low.onnx"
#endif

#ifndef PIPER_SPEED
#define PIPER_SPEED "1.0"
#endif

/* Chunk size for streaming: 50ms at 16kHz mono = 800 samples = 1600 bytes */
#define TTS_CHUNK_SAMPLES 800
#define TTS_CHUNK_BYTES (TTS_CHUNK_SAMPLES * 2)

static int initialized = 0;
static volatile int tts_interrupted = 0;

int hal_tts_init(void) {
  if (initialized)
    return 0;

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

  printf("HAL TTS: Piper initialized (model=%s, speed=%s)\n", PIPER_MODEL_PATH,
         PIPER_SPEED);
  initialized = 1;
  return 0;
}

int hal_tts_speak(const char *text, const char *output_file) {
  (void)output_file; /* Ignored - we stream directly */

  FILE *piper_pipe;
  char command[2048];
  int16_t chunk_buffer[TTS_CHUNK_SAMPLES];
  size_t bytes_read;

  if (!initialized) {
    if (hal_tts_init() != 0) {
      return -1;
    }
  }

  /* Ensure audio pipeline is ready */
  if (!hal_audio_pipeline_ready()) {
    fprintf(stderr, "HAL TTS: Audio pipeline not ready\n");
    return -1;
  }

  /* Clear interrupt flag */
  tts_interrupted = 0;

  /* Build command: echo text | piper --output_raw
   * We read the raw PCM from piper's stdout */
  snprintf(
      command, sizeof(command),
      "echo '%s' | piper --model %s --length_scale %s --output_raw 2>/dev/null",
      text, PIPER_MODEL_PATH, PIPER_SPEED);

  piper_pipe = popen(command, "r");
  if (piper_pipe == NULL) {
    fprintf(stderr, "HAL TTS: Failed to start Piper\n");
    return -1;
  }

  /* Stream Piper output through audio HAL in chunks */
  while (!tts_interrupted) {
    bytes_read = fread(chunk_buffer, 1, TTS_CHUNK_BYTES, piper_pipe);

    if (bytes_read == 0) {
      break; /* EOF - Piper finished */
    }

    /* Write chunk to audio HAL */
    if (hal_audio_write_raw(chunk_buffer, bytes_read / 2) != 0) {
      fprintf(stderr, "HAL TTS: Audio write failed\n");
      break;
    }
  }

  /* Clean up */
  pclose(piper_pipe);

  if (tts_interrupted) {
    printf("HAL TTS: Speech interrupted\n");
  }

  return 0;
}

void hal_tts_interrupt(void) {
  tts_interrupted = 1;
  /* Also interrupt the audio HAL to stop any buffered audio */
  hal_audio_interrupt();
  printf("HAL TTS: Interrupt requested\n");
}

void hal_tts_cleanup(void) {
  initialized = 0;
  printf("HAL TTS: Piper cleaned up\n");
}

const char *hal_tts_get_impl_name(void) { return "Piper (HAL Routed)"; }
