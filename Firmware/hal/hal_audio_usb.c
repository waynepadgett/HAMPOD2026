/**
 * @file hal_audio_usb.c
 * @brief USB Audio implementation of the audio HAL
 *
 * This implementation plays audio through a USB audio device
 * using ALSA with a persistent pipeline for low latency.
 *
 * Changes for Audio Latency Improvement:
 * - Persistent aplay process for raw PCM streaming
 * - Support for interruptible playback
 * - RAM-cached beep support
 */

#include "hal_audio.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Default audio device - uses system default which should be configured for
 * dmix */
static char audio_device[256] = "default";
static int initialized = 0;

/* Persistent audio pipeline for raw PCM streaming */
static FILE *audio_pipe = NULL;

/* Audio playback state for interrupt support */
static volatile int audio_interrupted = 0;
static volatile int audio_playing = 0;

/* Chunk size for streaming: 50ms at 16kHz mono = 800 samples = 1600 bytes */
#define AUDIO_SAMPLE_RATE 16000
#define AUDIO_CHANNELS 1
#define AUDIO_BYTES_PER_SAMPLE 2
#define AUDIO_CHUNK_MS 50
#define AUDIO_CHUNK_SAMPLES ((AUDIO_SAMPLE_RATE * AUDIO_CHUNK_MS) / 1000)
#define AUDIO_CHUNK_BYTES                                                      \
  (AUDIO_CHUNK_SAMPLES * AUDIO_CHANNELS * AUDIO_BYTES_PER_SAMPLE)

/**
 * @brief Start the persistent audio pipeline
 *
 * Opens an aplay process in raw PCM mode for streaming audio data
 *
 * @return 0 on success, -1 on failure
 */
static int start_audio_pipeline(void) {
  char command[512];

  if (audio_pipe != NULL) {
    return 0; /* Already running */
  }

  /* Build aplay command for raw PCM streaming:
   * -D device: audio device
   * -r 16000: 16kHz sample rate (matches Piper TTS)
   * -f S16_LE: 16-bit signed little-endian
   * -c 1: mono
   * -t raw: raw PCM data (no header)
   * -q: quiet mode
   * -: read from stdin
   */
  snprintf(command, sizeof(command),
           "aplay -D %s -r %d -f S16_LE -c %d -t raw -q - 2>/dev/null",
           audio_device, AUDIO_SAMPLE_RATE, AUDIO_CHANNELS);

  audio_pipe = popen(command, "w");
  if (audio_pipe == NULL) {
    fprintf(stderr, "HAL Audio: Failed to start audio pipeline\n");
    return -1;
  }

  printf("HAL Audio: Pipeline started (device=%s, rate=%d, channels=%d)\n",
         audio_device, AUDIO_SAMPLE_RATE, AUDIO_CHANNELS);
  return 0;
}

/**
 * @brief Stop the persistent audio pipeline
 */
static void stop_audio_pipeline(void) {
  if (audio_pipe != NULL) {
    pclose(audio_pipe);
    audio_pipe = NULL;
    printf("HAL Audio: Pipeline stopped\n");
  }
}

/**
 * @brief Detect USB audio device
 *
 * Parses output of 'aplay -l' to find USB audio device
 *
 * @return 0 on success, -1 on failure
 */
static int detect_usb_audio(void) {
  FILE *fp;
  char line[512];
  char card_name[64] = "";

  /* Run aplay -l to list audio devices */
  fp = popen("aplay -l 2>/dev/null", "r");
  if (fp == NULL) {
    fprintf(stderr, "HAL Audio: Failed to run aplay -l\n");
    return -1;
  }

  /* Parse output looking for USB device */
  while (fgets(line, sizeof(line), fp) != NULL) {
    /* Look for card line, typically: "card 1: Device [...]" */
    if (strstr(line, "card") && strstr(line, ":")) {
      /* Extract card number and name */
      int card_num;
      char name[128];
      if (sscanf(line, "card %d: %127s", &card_num, name) == 2) {
        /* Remove trailing colon if present */
        char *colon = strchr(name, ',');
        if (colon)
          *colon = '\0';

        /* Prefer USB or Device in the name */
        if (strstr(line, "USB") || strstr(name, "Device")) {
          /* Use plughw format for direct access without needing .asoundrc */
          snprintf(audio_device, sizeof(audio_device), "plughw:%d,0", card_num);
          pclose(fp);
          return 0;
        }

        /* Save first device as fallback */
        if (card_name[0] == '\0') {
          strncpy(card_name, name, sizeof(card_name) - 1);
        }
      }
    }
  }

  pclose(fp);

  /* If USB not found but we have a device, use that */
  if (card_name[0] != '\0') {
    snprintf(audio_device, sizeof(audio_device), "sysdefault:CARD=%s",
             card_name);
    return 0;
  }

  /* No audio device found */
  fprintf(stderr, "HAL Audio: No audio devices found\n");
  return -1;
}

/* ============================================================================
 * Public HAL API Functions
 * ============================================================================
 */

int hal_audio_init(void) {
  if (initialized) {
    return 0; /* Already initialized */
  }

  /* Attempt to detect USB audio device */
  if (detect_usb_audio() != 0) {
    fprintf(stderr, "HAL Audio: Using default device: %s\n", audio_device);
    /* Continue anyway with default, don't fail */
  } else {
    printf("HAL Audio: Detected audio device: %s\n", audio_device);
  }

  /* Start the persistent audio pipeline */
  if (start_audio_pipeline() != 0) {
    fprintf(stderr, "HAL Audio: Failed to start pipeline, falling back to "
                    "system() calls\n");
    /* Don't fail - hal_audio_play_file will fall back to system() if pipe is
     * NULL */
  }

  initialized = 1;
  return 0;
}

int hal_audio_set_device(const char *device_name) {
  if (device_name == NULL) {
    return -1;
  }

  strncpy(audio_device, device_name, sizeof(audio_device) - 1);
  audio_device[sizeof(audio_device) - 1] = '\0';

  /* Restart pipeline with new device if already initialized */
  if (initialized && audio_pipe != NULL) {
    stop_audio_pipeline();
    start_audio_pipeline();
  }

  printf("HAL Audio: Device set to: %s\n", audio_device);
  return 0;
}

const char *hal_audio_get_device(void) { return audio_device; }

/**
 * @brief Write raw PCM samples to the audio pipeline
 *
 * @param samples Pointer to 16-bit signed samples
 * @param num_samples Number of samples to write
 * @return 0 on success, -1 on failure
 */
int hal_audio_write_raw(const int16_t *samples, size_t num_samples) {
  if (!initialized || audio_pipe == NULL) {
    fprintf(stderr, "HAL Audio: Pipeline not available\n");
    return -1;
  }

  if (samples == NULL || num_samples == 0) {
    return 0;
  }

  size_t written = fwrite(samples, sizeof(int16_t), num_samples, audio_pipe);
  fflush(audio_pipe);

  if (written != num_samples) {
    fprintf(stderr, "HAL Audio: Write failed (%zu of %zu samples)\n", written,
            num_samples);
    return -1;
  }

  return 0;
}

int hal_audio_play_file(const char *filepath) {
  FILE *wav_file;
  uint8_t header[44];
  uint32_t data_size;
  uint32_t sample_rate;
  uint16_t num_channels;
  uint16_t bits_per_sample;
  uint8_t *chunk_buffer;
  size_t bytes_remaining;
  size_t bytes_to_read;
  size_t bytes_read;

  if (!initialized) {
    fprintf(stderr, "HAL Audio: Not initialized\n");
    return -1;
  }

  if (filepath == NULL) {
    return -1;
  }

  /* Open WAV file */
  wav_file = fopen(filepath, "rb");
  if (wav_file == NULL) {
    fprintf(stderr, "HAL Audio: Cannot open file: %s\n", filepath);
    return -1;
  }

  /* Read WAV header (44 bytes for standard WAV) */
  if (fread(header, 1, 44, wav_file) != 44) {
    fprintf(stderr, "HAL Audio: Failed to read WAV header: %s\n", filepath);
    fclose(wav_file);
    return -1;
  }

  /* Verify RIFF header */
  if (memcmp(header, "RIFF", 4) != 0 || memcmp(header + 8, "WAVE", 4) != 0) {
    fprintf(stderr, "HAL Audio: Not a valid WAV file: %s\n", filepath);
    fclose(wav_file);
    return -1;
  }

  /* Parse WAV header fields (little-endian) */
  num_channels = header[22] | (header[23] << 8);
  sample_rate =
      header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
  bits_per_sample = header[34] | (header[35] << 8);
  data_size =
      header[40] | (header[41] << 8) | (header[42] << 16) | (header[43] << 24);

  /* Validate format matches our pipeline configuration */
  if (sample_rate != AUDIO_SAMPLE_RATE) {
    fprintf(stderr, "HAL Audio: Sample rate mismatch (%u vs %d): %s\n",
            sample_rate, AUDIO_SAMPLE_RATE, filepath);
    /* Fall back to system() for non-matching files */
    fclose(wav_file);
    goto fallback_system;
  }

  if (num_channels != AUDIO_CHANNELS) {
    fprintf(stderr, "HAL Audio: Channel mismatch (%u vs %d): %s\n",
            num_channels, AUDIO_CHANNELS, filepath);
    fclose(wav_file);
    goto fallback_system;
  }

  if (bits_per_sample != 16) {
    fprintf(stderr, "HAL Audio: Bit depth mismatch (%u vs 16): %s\n",
            bits_per_sample, filepath);
    fclose(wav_file);
    goto fallback_system;
  }

  /* Check if pipeline is available */
  if (audio_pipe == NULL) {
    fclose(wav_file);
    goto fallback_system;
  }

  /* Allocate chunk buffer */
  chunk_buffer = (uint8_t *)malloc(AUDIO_CHUNK_BYTES);
  if (chunk_buffer == NULL) {
    fprintf(stderr, "HAL Audio: Failed to allocate chunk buffer\n");
    fclose(wav_file);
    return -1;
  }

  /* Stream audio data in chunks */
  audio_playing = 1;
  audio_interrupted = 0;
  bytes_remaining = data_size;

  while (bytes_remaining > 0 && !audio_interrupted) {
    bytes_to_read = (bytes_remaining > AUDIO_CHUNK_BYTES) ? AUDIO_CHUNK_BYTES
                                                          : bytes_remaining;
    bytes_read = fread(chunk_buffer, 1, bytes_to_read, wav_file);

    if (bytes_read == 0) {
      break; /* EOF or error */
    }

    /* Write to pipeline */
    size_t written = fwrite(chunk_buffer, 1, bytes_read, audio_pipe);
    fflush(audio_pipe);

    if (written != bytes_read) {
      fprintf(stderr, "HAL Audio: Write error during streaming\n");
      break;
    }

    bytes_remaining -= bytes_read;
  }

  audio_playing = 0;
  audio_interrupted = 0;
  free(chunk_buffer);
  fclose(wav_file);
  return 0;

fallback_system: {
  /* Fallback to system() call for non-matching formats */
  char command[1024];
  int result;

  snprintf(command, sizeof(command), "aplay -D %s '%s' 2>/dev/null",
           audio_device, filepath);

  audio_playing = 1;
  result = system(command);
  audio_playing = 0;

  if (result != 0) {
    fprintf(stderr, "HAL Audio: Failed to play file: %s\n", filepath);
    return -1;
  }
  return 0;
}
}

/**
 * @brief Interrupt current audio playback
 *
 * Sets the interrupt flag to stop playback at the next chunk boundary.
 * Playback will stop within approximately 50ms.
 */
void hal_audio_interrupt(void) { audio_interrupted = 1; }

/**
 * @brief Check if audio is currently playing
 *
 * @return 1 if playing, 0 otherwise
 */
int hal_audio_is_playing(void) { return audio_playing; }

/**
 * @brief Check if pipeline is ready for streaming
 *
 * @return 1 if ready, 0 otherwise
 */
int hal_audio_pipeline_ready(void) {
  return (initialized && audio_pipe != NULL) ? 1 : 0;
}

void hal_audio_cleanup(void) {
  stop_audio_pipeline();
  initialized = 0;
  printf("HAL Audio: Cleaned up\n");
}

const char *hal_audio_get_impl_name(void) {
  return "USB Audio (ALSA Persistent Pipeline)";
}
