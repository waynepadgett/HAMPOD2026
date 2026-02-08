/**
 * @file hal_audio_usb.c
 * @brief USB Audio implementation of the audio HAL
 *
 * This implementation plays audio through a USB audio device
 * using direct ALSA PCM API for low latency and true interrupt support.
 *
 * Phase 3: Direct ALSA Implementation
 * - Uses snd_pcm_* API instead of popen("aplay")
 * - snd_pcm_drop() for immediate interrupt
 * - RAM-cached beep support
 */

#include "hal_audio.h"
#include "hal_usb_util.h"
#include <alsa/asoundlib.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Default audio device - uses system default which should be configured for
 * dmix */
static char audio_device[256] = "default";
static int initialized = 0;

/* Selected audio device info (from USB enumeration) */
static AudioDeviceInfo selected_audio_device = {0};

/* Direct ALSA PCM handle (replaces popen/aplay pipeline) */
static snd_pcm_t *pcm_handle = NULL;

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

/* ============================================================================
 * RAM-Cached Beeps (Phase 2)
 * ============================================================================
 */

/* Beep file paths (relative to Firmware directory) */
#ifndef BEEP_BASE_PATH
#define BEEP_BASE_PATH "pregen_audio/"
#endif

#define BEEP_KEYPRESS_PATH BEEP_BASE_PATH "beep_keypress.wav"
#define BEEP_HOLD_PATH BEEP_BASE_PATH "beep_hold.wav"
#define BEEP_ERROR_PATH BEEP_BASE_PATH "beep_error.wav"

/* Structure for caching audio in RAM */
typedef struct {
  int16_t *samples;   /* PCM data in memory */
  size_t num_samples; /* Number of samples */
  int loaded;         /* 1 if successfully loaded, 0 otherwise */
} CachedAudio;

/* Cached beep sounds */
static CachedAudio beep_keypress = {0};
static CachedAudio beep_hold = {0};
static CachedAudio beep_error = {0};

/**
 * @brief Load a WAV file into the cache
 *
 * Reads the WAV file, validates format, and stores PCM data in memory.
 *
 * @param filepath Path to WAV file
 * @param cache Pointer to CachedAudio structure to fill
 * @return 0 on success, -1 on failure
 */
static int load_wav_to_cache(const char *filepath, CachedAudio *cache) {
  FILE *wav_file;
  uint8_t header[44];
  uint32_t data_size;
  uint32_t sample_rate;
  uint16_t num_channels;
  uint16_t bits_per_sample;
  size_t samples_read;

  if (cache == NULL)
    return -1;

  /* Clear existing cache */
  if (cache->samples != NULL) {
    free(cache->samples);
    cache->samples = NULL;
  }
  cache->num_samples = 0;
  cache->loaded = 0;

  /* Open WAV file */
  wav_file = fopen(filepath, "rb");
  if (wav_file == NULL) {
    fprintf(stderr, "HAL Audio: Cannot open beep file: %s\n", filepath);
    return -1;
  }

  /* Read and parse WAV header */
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

  /* Parse header fields */
  num_channels = header[22] | (header[23] << 8);
  sample_rate =
      header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
  bits_per_sample = header[34] | (header[35] << 8);
  data_size =
      header[40] | (header[41] << 8) | (header[42] << 16) | (header[43] << 24);

  /* Validate format */
  if (sample_rate != AUDIO_SAMPLE_RATE || num_channels != AUDIO_CHANNELS ||
      bits_per_sample != 16) {
    fprintf(stderr,
            "HAL Audio: Beep format mismatch (rate=%u, ch=%u, bits=%u): %s\n",
            sample_rate, num_channels, bits_per_sample, filepath);
    fclose(wav_file);
    return -1;
  }

  /* Allocate memory for samples */
  cache->num_samples = data_size / 2; /* 16-bit = 2 bytes per sample */
  cache->samples = (int16_t *)malloc(data_size);
  if (cache->samples == NULL) {
    fprintf(stderr, "HAL Audio: Failed to allocate memory for beep: %s\n",
            filepath);
    fclose(wav_file);
    return -1;
  }

  /* Read PCM data */
  samples_read = fread(cache->samples, 2, cache->num_samples, wav_file);
  fclose(wav_file);

  if (samples_read != cache->num_samples) {
    fprintf(stderr, "HAL Audio: Incomplete read (got %zu of %zu): %s\n",
            samples_read, cache->num_samples, filepath);
    free(cache->samples);
    cache->samples = NULL;
    cache->num_samples = 0;
    return -1;
  }

  cache->loaded = 1;
  printf("HAL Audio: Cached beep: %s (%zu samples, %zu ms)\n", filepath,
         cache->num_samples, (cache->num_samples * 1000) / AUDIO_SAMPLE_RATE);
  return 0;
}

/**
 * @brief Free cached audio memory
 */
static void free_cached_audio(CachedAudio *cache) {
  if (cache != NULL && cache->samples != NULL) {
    free(cache->samples);
    cache->samples = NULL;
    cache->num_samples = 0;
    cache->loaded = 0;
  }
}

/**
 * @brief Open the ALSA PCM device for direct audio output
 *
 * Uses snd_pcm_* API for low latency and interruptible playback.
 * Buffer size: 100ms total, 25ms periods - allows interrupt within 25ms
 *
 * @return 0 on success, -1 on failure
 */
static int open_pcm_device(void) {
  int err;
  snd_pcm_hw_params_t *hw_params = NULL;
  unsigned int rate = AUDIO_SAMPLE_RATE;
  snd_pcm_uframes_t buffer_frames;
  snd_pcm_uframes_t period_frames;

  if (pcm_handle != NULL) {
    return 0; /* Already open */
  }

  /* Open PCM device for playback */
  err = snd_pcm_open(&pcm_handle, audio_device, SND_PCM_STREAM_PLAYBACK, 0);
  if (err < 0) {
    fprintf(stderr, "HAL Audio: Cannot open device '%s': %s\n", audio_device,
            snd_strerror(err));
    return -1;
  }

  /* Allocate hardware parameters */
  snd_pcm_hw_params_malloc(&hw_params);
  snd_pcm_hw_params_any(pcm_handle, hw_params);

  /* Set access type: interleaved read/write */
  err = snd_pcm_hw_params_set_access(pcm_handle, hw_params,
                                     SND_PCM_ACCESS_RW_INTERLEAVED);
  if (err < 0) {
    fprintf(stderr, "HAL Audio: Cannot set access type: %s\n",
            snd_strerror(err));
    goto error;
  }

  /* Set sample format: 16-bit signed little-endian */
  err = snd_pcm_hw_params_set_format(pcm_handle, hw_params,
                                     SND_PCM_FORMAT_S16_LE);
  if (err < 0) {
    fprintf(stderr, "HAL Audio: Cannot set format: %s\n", snd_strerror(err));
    goto error;
  }

  /* Set channel count: mono */
  err = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, AUDIO_CHANNELS);
  if (err < 0) {
    fprintf(stderr, "HAL Audio: Cannot set channels: %s\n", snd_strerror(err));
    goto error;
  }

  /* Set sample rate (16kHz - matching Piper TTS) */
  err = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &rate, NULL);
  if (err < 0) {
    fprintf(stderr, "HAL Audio: Cannot set rate: %s\n", snd_strerror(err));
    goto error;
  }
  if (rate != AUDIO_SAMPLE_RATE) {
    fprintf(stderr, "HAL Audio: Rate mismatch, got %u instead of %d\n", rate,
            AUDIO_SAMPLE_RATE);
  }

  /* Set buffer size: 100ms = 1600 samples at 16kHz */
  buffer_frames = AUDIO_SAMPLE_RATE / 10; /* 100ms */
  err = snd_pcm_hw_params_set_buffer_size_near(pcm_handle, hw_params,
                                               &buffer_frames);
  if (err < 0) {
    fprintf(stderr, "HAL Audio: Cannot set buffer size: %s\n",
            snd_strerror(err));
    goto error;
  }

  /* Set period size: 25ms = 400 samples (4 periods per buffer) */
  period_frames = buffer_frames / 4;
  err = snd_pcm_hw_params_set_period_size_near(pcm_handle, hw_params,
                                               &period_frames, NULL);
  if (err < 0) {
    fprintf(stderr, "HAL Audio: Cannot set period size: %s\n",
            snd_strerror(err));
    goto error;
  }

  /* Apply hardware parameters */
  err = snd_pcm_hw_params(pcm_handle, hw_params);
  if (err < 0) {
    fprintf(stderr, "HAL Audio: Cannot apply hw params: %s\n",
            snd_strerror(err));
    goto error;
  }

  /* Prepare device for playback */
  err = snd_pcm_prepare(pcm_handle);
  if (err < 0) {
    fprintf(stderr, "HAL Audio: Cannot prepare device: %s\n",
            snd_strerror(err));
    goto error;
  }

  snd_pcm_hw_params_free(hw_params);
  printf("HAL Audio: ALSA PCM device opened (device=%s, rate=%u, buffer=%lu, "
         "period=%lu)\n",
         audio_device, rate, buffer_frames, period_frames);
  return 0;

error:
  if (hw_params)
    snd_pcm_hw_params_free(hw_params);
  if (pcm_handle) {
    snd_pcm_close(pcm_handle);
    pcm_handle = NULL;
  }
  return -1;
}

/**
 * @brief Close the ALSA PCM device
 */
static void close_pcm_device(void) {
  if (pcm_handle != NULL) {
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
    pcm_handle = NULL;
    printf("HAL Audio: PCM device closed\n");
  }
}

/**
 * @brief Detect USB audio device using enumeration utility
 *
 * Uses hal_usb_find_audio to find the best audio device with this priority:
 * 1. "USB2.0 Device" (preferred external speaker)
 * 2. Any USB audio device
 * 3. Headphones
 * 4. Default
 *
 * @return 0 on success, -1 on failure
 */
static int detect_usb_audio(void) {
  AudioDeviceInfo result;

  /* Use the USB enumeration utility with "USB2.0 Device" as preferred */
  if (hal_usb_find_audio("USB2.0 Device", &result) == 0) {
    /* Copy the selected device info */
    selected_audio_device = result;
    strncpy(audio_device, result.device_path, sizeof(audio_device) - 1);
    audio_device[sizeof(audio_device) - 1] = '\0';
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

  /* Open the PCM device for direct audio output */
  if (open_pcm_device() != 0) {
    fprintf(stderr,
            "HAL Audio: Failed to open PCM device, audio will not work\n");
    /* Don't fail - we'll try again on first write */
  }

  /* Load beep sounds into RAM cache */
  printf("HAL Audio: Loading beep sounds...\n");
  if (load_wav_to_cache(BEEP_KEYPRESS_PATH, &beep_keypress) != 0) {
    fprintf(stderr, "HAL Audio: Warning - keypress beep not loaded\n");
  }
  if (load_wav_to_cache(BEEP_HOLD_PATH, &beep_hold) != 0) {
    fprintf(stderr, "HAL Audio: Warning - hold beep not loaded\n");
  }
  if (load_wav_to_cache(BEEP_ERROR_PATH, &beep_error) != 0) {
    fprintf(stderr, "HAL Audio: Warning - error beep not loaded\n");
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

  /* Restart PCM device with new device if already initialized */
  if (initialized && pcm_handle != NULL) {
    close_pcm_device();
    open_pcm_device();
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
  snd_pcm_sframes_t frames;

  if (!initialized || pcm_handle == NULL) {
    fprintf(stderr, "HAL Audio: PCM device not available\n");
    return -1;
  }

  if (samples == NULL || num_samples == 0) {
    return 0;
  }

  /* Check if we've been interrupted - return early without writing */
  if (audio_interrupted) {
    return 0; /* Silently drop audio - we're being interrupted */
  }

  /* Write samples to ALSA PCM device */
  frames = snd_pcm_writei(pcm_handle, samples, num_samples);

  if (frames < 0) {
    /* Try to recover from underrun or other errors */
    frames = snd_pcm_recover(pcm_handle, frames, 0);
    if (frames < 0) {
      fprintf(stderr, "HAL Audio: Write failed: %s\n", snd_strerror(frames));
      return -1;
    }
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

  /* Check if PCM device is available */
  if (pcm_handle == NULL) {
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

    /* Write to ALSA PCM device */
    snd_pcm_sframes_t frames =
        snd_pcm_writei(pcm_handle, chunk_buffer, bytes_read / 2);

    if (frames < 0) {
      frames = snd_pcm_recover(pcm_handle, frames, 0);
      if (frames < 0) {
        fprintf(stderr, "HAL Audio: Write error during streaming\n");
        break;
      }
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
 * @brief Interrupt current audio playback immediately
 *
 * Uses snd_pcm_drop() to immediately stop playback and discard any
 * buffered audio. This provides true interrupt capability.
 */
void hal_audio_interrupt(void) {
  audio_interrupted = 1;

  /* Immediately stop playback and discard buffer */
  if (pcm_handle != NULL) {
    snd_pcm_drop(pcm_handle);
  }
}

/**
 * @brief Clear the interrupt flag and prepare device for new audio
 *
 * This should be called at the start of a new audio operation
 * to reset from a previous interrupt. Only calls snd_pcm_prepare() if
 * we were actually interrupted (after snd_pcm_drop was called).
 */
void hal_audio_clear_interrupt(void) {
  /* Only call snd_pcm_prepare if we were actually interrupted.
   * This prevents clearing the buffer when transitioning between
   * audio packets (e.g., beep -> TTS) without an interrupt. */
  if (audio_interrupted && pcm_handle != NULL) {
    snd_pcm_prepare(pcm_handle);
  }
  audio_interrupted = 0;
}

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
  return (initialized && pcm_handle != NULL) ? 1 : 0;
}

void hal_audio_cleanup(void) {
  /* Free cached beeps */
  free_cached_audio(&beep_keypress);
  free_cached_audio(&beep_hold);
  free_cached_audio(&beep_error);

  close_pcm_device();
  initialized = 0;
  printf("HAL Audio: Cleaned up\n");
}

/**
 * @brief Play a beep from RAM cache
 *
 * Plays a pre-loaded beep sound with minimal latency.
 *
 * @param type Type of beep to play (BEEP_KEYPRESS, BEEP_HOLD, BEEP_ERROR)
 * @return 0 on success, -1 on failure
 */
int hal_audio_play_beep(BeepType type) {
  CachedAudio *beep = NULL;

  printf("HAL Audio: play_beep called, type=%d\n", type);

  switch (type) {
  case BEEP_KEYPRESS:
    beep = &beep_keypress;
    break;
  case BEEP_HOLD:
    beep = &beep_hold;
    break;
  case BEEP_ERROR:
    beep = &beep_error;
    break;
  default:
    fprintf(stderr, "HAL Audio: Unknown beep type: %d\n", type);
    return -1;
  }

  if (!beep->loaded || beep->samples == NULL) {
    fprintf(stderr,
            "HAL Audio: Beep not loaded (type=%d, loaded=%d, samples=%p)\n",
            type, beep->loaded, (void *)beep->samples);
    return -1;
  }

  if (!initialized || pcm_handle == NULL) {
    fprintf(stderr, "HAL Audio: PCM device not ready for beep\n");
    return -1;
  }

  printf("HAL Audio: Writing %zu samples for beep\n", beep->num_samples);

  /* Write beep samples directly to ALSA (fast, no file I/O) */
  snd_pcm_sframes_t frames =
      snd_pcm_writei(pcm_handle, beep->samples, beep->num_samples);

  printf("HAL Audio: snd_pcm_writei returned %ld\n", (long)frames);

  if (frames < 0) {
    printf("HAL Audio: Attempting recovery from %ld\n", (long)frames);
    frames = snd_pcm_recover(pcm_handle, frames, 0);
    if (frames < 0) {
      fprintf(stderr, "HAL Audio: Beep write error: %s\n",
              snd_strerror(frames));
      return -1;
    }
  }

  /* Drain to ensure beep plays completely before returning.
   * This prevents TTS from starting before the beep is heard. */
  printf("HAL Audio: Draining...\n");
  snd_pcm_drain(pcm_handle);
  printf("HAL Audio: Preparing for next audio...\n");
  snd_pcm_prepare(pcm_handle); /* Prepare for next audio */

  printf("HAL Audio: Beep complete\n");
  return 0;
}

const char *hal_audio_get_impl_name(void) {
  return "USB Audio (Direct ALSA PCM)";
}

int hal_audio_get_card_number(void) {
  if (!initialized) {
    return -1;
  }
  return selected_audio_device.card_number;
}

const char *hal_audio_get_port_path(void) {
  if (!initialized || !selected_audio_device.is_usb) {
    return NULL;
  }
  return selected_audio_device.usb_port;
}
