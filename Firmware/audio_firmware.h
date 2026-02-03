#ifndef AUDIO_FIRMWARE
#define AUDIO_FIRMWARE

#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "hampod_firm_packet.h"
#include "hampod_queue.h"

#define HASHING_PRIME 183373
#define PRIME2 17
#define MAXSTRINGSIZE 512
#define STRING_COUNT 16
#define TABLE_SIZE 0x1000
#define AUDIO_O "../Firmware/Speaker_o"
#define AUDIO_I "../Firmware/Speaker_i"

#define AUDIO_THREAD_COLOR "\033[0;34mAudio - Main: "
#define AUDIO_IO_THREAD_COLOR "\033[0;32mAudio - IO: "

#ifdef DEBUG
#define AUDIO_PRINTF(...)                                                      \
  do {                                                                         \
    if (DEBUG) {                                                               \
      printf(AUDIO_THREAD_COLOR);                                              \
      printf(__VA_ARGS__);                                                     \
    }                                                                          \
  } while (0)

#define AUDIO_IO_PRINTF(...)                                                   \
  do {                                                                         \
    if (DEBUG) {                                                               \
      printf(AUDIO_IO_THREAD_COLOR);                                           \
      printf(__VA_ARGS__);                                                     \
    }                                                                          \
  } while (0)
#else

#define AUDIO_PRINTF(...)                                                      \
  do {                                                                         \
  } while (0)

#define AUDIO_IO_PRINTF(...)                                                   \
  do {                                                                         \
  } while (0)

#endif

typedef struct audio_io_packet {
  int pipe_fd;        /* Input pipe for receiving packets */
  int output_pipe_fd; /* Output pipe for sending ack (used for interrupt bypass)
                       */
  Packet_queue *queue;
} audio_io_packet;

#include "hal/hal_audio.h"

/**
 * @brief Play a beep sound directly (low-latency, no pipe IPC)
 *
 * This function plays a pre-generated beep audio file using the HAL.
 * It is thread-safe and can be called from any process/thread.
 *
 * @param type The type of beep to play
 * @return 0 on success, -1 on error
 *
 * @note Requires beep audio files to exist in pregen_audio/:
 *       - beep_keypress.wav
 *       - beep_hold.wav
 *       - beep_error.wav
 *       Generate these with: ./Firmware/pregen_audio/generate_beeps.sh
 */
int audio_play_beep(BeepType type);

void audio_process();
void *audio_io_thread(void *arg);
void firmwareStartAudio();
int firmwarePlayAudio(void *text);
#ifndef SHAREDLIB
#include "audio_firmware.c"
#endif
#endif
