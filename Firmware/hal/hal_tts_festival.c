/**
 * @file hal_tts_festival.c
 * @brief Festival TTS implementation of the TTS HAL
 *
 * Uses Festival's text2wave command for speech synthesis.
 */

#include "hal_audio.h"
#include "hal_tts.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int initialized = 0;

int hal_tts_init(void) {
  if (initialized)
    return 0;

  /* Check for text2wave command */
  if (system("which text2wave > /dev/null 2>&1") != 0) {
    fprintf(stderr, "\n");
    fprintf(stderr, "===================================================\n");
    fprintf(stderr, "ERROR: Festival 'text2wave' not found!\n");
    fprintf(stderr, "===================================================\n");
    fprintf(stderr, "Install with:\n");
    fprintf(stderr, "    sudo apt-get install festival festvox-kallpc16k\n");
    fprintf(stderr, "===================================================\n");
    return -1;
  }

  printf("HAL TTS: Festival initialized\n");
  initialized = 1;
  return 0;
}

int hal_tts_speak(const char *text, const char *output_file) {
  if (!initialized) {
    fprintf(stderr, "HAL TTS: Not initialized\n");
    return -1;
  }

  char command[1024];
  const char *out = output_file ? output_file : "/tmp/hampod_speak.wav";

  /* Generate speech with text2wave */
  snprintf(command, sizeof(command),
           "echo '%s' | text2wave -o '%s' 2>/dev/null", text, out);

  int result = system(command);
  if (result != 0) {
    fprintf(stderr, "HAL TTS: text2wave failed\n");
    return -1;
  }

  /* Play the generated file */
  return hal_audio_play_file(out);
}

void hal_tts_interrupt(void) {
  /* Festival doesn't use a persistent process, but we can signal
   * hal_audio to stop playing the generated file. */
  hal_audio_interrupt();
}

void hal_tts_cleanup(void) {
  initialized = 0;
  printf("HAL TTS: Festival cleaned up\n");
}

const char *hal_tts_get_impl_name(void) { return "Festival"; }
