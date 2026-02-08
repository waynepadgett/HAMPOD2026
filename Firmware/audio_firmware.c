/* This code is for the audio firmware for the Hampod Program
 * Written by Brendan Perez
 * Last modified on 10/23/2023
 * Updated for Raspberry Pi USB HAL on 11/29/2025
 */

#include "audio_firmware.h"
#include "hal/hal_audio.h"
#include "hal/hal_tts.h"

extern pid_t controller_pid;
unsigned char audio_running = 1;
pthread_mutex_t audio_queue_lock;
pthread_mutex_t audio_queue_available;
pthread_mutex_t audio_lock;

void audio_process() {
  char buffer[MAXSTRINGSIZE];

  AUDIO_PRINTF("Audio process launched\nConnecting to input/output pipes\n");

  int input_pipe_fd = open(AUDIO_I, O_RDONLY);
  if (input_pipe_fd == -1) {
    perror("open");
    // kill(controller_pid, SIGINT);
    exit(0);
  }

  int output_pipe_fd = open(AUDIO_O, O_WRONLY);
  if (output_pipe_fd == -1) {
    perror("open");
    // kill(controller_pid, SIGINT);
    exit(0);
  }

  AUDIO_PRINTF("Pipes successfully connected\nCreating input queue\n");

  if (hal_audio_init() != 0) {
    AUDIO_PRINTF("Failed to initialize audio HAL\n");
  } else {
    AUDIO_PRINTF("Audio HAL initialized\n");
  }

  if (hal_tts_init() != 0) {
    AUDIO_PRINTF("Failed to initialize TTS HAL\n");
  } else {
    AUDIO_PRINTF("TTS HAL initialized: %s\n", hal_tts_get_impl_name());
  }

  AUDIO_PRINTF("Creating input queue\n");

  Packet_queue *input_queue = create_packet_queue();

  AUDIO_PRINTF("Initializing queue mutex lock\n");

  if (pthread_mutex_init(&audio_queue_lock, NULL) != 0) {
    perror("pthread_mutex_init");
    // kill(controller_pid, SIGINT);
    exit(1);
  }
  AUDIO_PRINTF("Initializing queue availibility mutex lock\n");

  if (pthread_mutex_init(&audio_queue_available, NULL) != 0) {
    perror("pthread_mutex_init");
    // kill(controller_pid, SIGINT);
    exit(1);
  }

  pthread_t audio_io_buffer;
  audio_io_packet thread_input;
  thread_input.pipe_fd = input_pipe_fd;
  thread_input.output_pipe_fd = output_pipe_fd;
  thread_input.queue = input_queue;

  AUDIO_PRINTF("Launching IO thread\n");
  if (pthread_create(&audio_io_buffer, NULL, audio_io_thread,
                     (void *)&thread_input) != 0) {
    perror("Keypad IO thread failed");
    // kill(controller_pid, SIGINT);
    exit(1);
  }
  usleep(500000); // Half sec sleep to let child thread take control

  while (audio_running) {
    pthread_mutex_lock(&audio_queue_available);
    pthread_mutex_lock(&audio_queue_lock);
    if (is_empty(input_queue)) {
      pthread_mutex_unlock(&audio_queue_available);
      pthread_mutex_unlock(&audio_queue_lock);
      usleep(500);
      continue;
    }
    Inst_packet *received_packet = dequeue(input_queue);
    pthread_mutex_unlock(&audio_queue_lock);
    pthread_mutex_unlock(&audio_queue_available);
    char *requested_string = calloc(1, received_packet->data_len + 0x10);
    strcpy(requested_string, (char *)received_packet->data);
    char audio_type_byte = requested_string[0];
    char *remaining_string = requested_string + 1;
    int system_result;
    unsigned short packet_tag = received_packet->tag;
    char default_directory[0x100];
    getcwd(default_directory, sizeof(default_directory));
    if (audio_type_byte == 'd') {
      /* New TTS request - clear interrupt so this can play */
      hal_audio_clear_interrupt();
      AUDIO_PRINTF("TTS speak (direct): %s\n", remaining_string);
      system_result = hal_tts_speak(remaining_string, NULL);
    } else if (audio_type_byte == 's') {
      /* New TTS request - clear interrupt so this can play */
      hal_audio_clear_interrupt();
      AUDIO_PRINTF("TTS speak (save): %s\n", remaining_string);
      /* For Piper persistent mode, we ignore output_file and speak directly */
      system_result = hal_tts_speak(remaining_string, NULL);
    } else if (audio_type_byte == 'p') {
      /* Playing audio file - clear interrupt so this can play */
      hal_audio_clear_interrupt();
      // strcpy(buffer, "aplay '");
      // strcat(remaining_string, ".wav'");
      // strcat(buffer, remaining_string);
      sprintf(buffer, "%s.wav", remaining_string);
      AUDIO_PRINTF("Now playing %s with HAL\n", remaining_string);
      // system_result = system(buffer);
      system_result = hal_audio_play_file(buffer);
    } else if (audio_type_byte == 'b') {
      /* Beep request: remaining_string is beep type ('k'=keypress, 'h'=hold,
       * 'e'=error) */
      hal_audio_clear_interrupt(); /* Clear interrupt so beep can play */
      BeepType beep_type;
      switch (remaining_string[0]) {
      case 'k':
        beep_type = BEEP_KEYPRESS;
        break;
      case 'h':
        beep_type = BEEP_HOLD;
        break;
      case 'e':
        beep_type = BEEP_ERROR;
        break;
      default:
        AUDIO_PRINTF("Unknown beep type: %c\n", remaining_string[0]);
        beep_type = BEEP_KEYPRESS;
      }
      system_result = audio_play_beep(beep_type);
    } else if (audio_type_byte == 'i') {
      /* Interrupt request */
      AUDIO_PRINTF("Interrupting audio playback\n");
      hal_audio_interrupt();
      hal_tts_interrupt();
      system_result = 0;
    } else if (audio_type_byte == 'q') {
      /* Query audio device info - return card number */
      AUDIO_PRINTF("Querying audio device info\n");
      system_result = hal_audio_get_card_number();
      AUDIO_PRINTF("Returning card number: %d\n", system_result);
    } else {
      AUDIO_PRINTF("Audio error. Unrecognized packet data %s\n",
                   requested_string);
      system_result = -1;
    }
    Inst_packet *packet_to_send = create_inst_packet(
        AUDIO, sizeof(int), (unsigned char *)&system_result, packet_tag);
    AUDIO_PRINTF("Sending back value of %x\n", system_result);
    write(output_pipe_fd, packet_to_send,
          8); // Packet_to_send is a pointer, so would this not just be sending
              // back the address of it.
    write(
        output_pipe_fd, packet_to_send->data,
        sizeof(
            int)); // while earlier you did have this be sizeof(int) would it
                   // not be safer to just use the size value of packet_to_send
    free(requested_string);
    destroy_inst_packet(&packet_to_send);
  }

  pthread_join(audio_io_buffer, NULL);
  destroy_queue(input_queue);
  close(input_pipe_fd);
  close(output_pipe_fd); // Graceful closing is always nice :)

  hal_audio_cleanup();
  return;
}

void *audio_io_thread(void *arg) {
  AUDIO_IO_PRINTF("Audio IO thread created\n");

  audio_io_packet *io_args = (audio_io_packet *)arg;
  int i_pipe = io_args->pipe_fd;
  int o_pipe = io_args->output_pipe_fd;
  Packet_queue *queue = io_args->queue;
  unsigned char buffer[256];

  AUDIO_IO_PRINTF("Input pipe = %d, output pipe = %d, queue ptr = %p\n", i_pipe,
                  o_pipe, (void *)queue);

  while (audio_running) {
    pthread_mutex_lock(&audio_queue_lock);
    int queue_empty = is_empty(queue);
    pthread_mutex_unlock(&audio_queue_lock);
    if (queue_empty) {
      // KEYPAD_IO_PRINTF("Making queue inaccessible\n");
      pthread_mutex_lock(&audio_queue_available);
    }

    Packet_type type;
    unsigned short size;
    unsigned short tag;
    ssize_t bytes_read;

    /* Read packet header with error checking */
    bytes_read = read(i_pipe, &type, sizeof(Packet_type));
    if (bytes_read <= 0) {
      AUDIO_IO_PRINTF("Pipe closed or read error (type), exiting thread\n");
      break;
    }

    bytes_read = read(i_pipe, &size, sizeof(unsigned short));
    if (bytes_read <= 0) {
      AUDIO_IO_PRINTF("Pipe closed or read error (size), exiting thread\n");
      break;
    }

    bytes_read = read(i_pipe, &tag, sizeof(unsigned short));
    if (bytes_read <= 0) {
      AUDIO_IO_PRINTF("Pipe closed or read error (tag), exiting thread\n");
      break;
    }

    bytes_read = read(i_pipe, buffer, size);
    if (bytes_read <= 0) {
      AUDIO_IO_PRINTF("Pipe closed or read error (data), exiting thread\n");
      break;
    }

    AUDIO_IO_PRINTF("Found packet with type %d, size %d\n", type, size);
    AUDIO_IO_PRINTF("Buffer holds: %s: with size %d\n", buffer, size);

    if (type != AUDIO) {
      AUDIO_IO_PRINTF("Packet not supported for Audio firmware\n");
      continue;
    }

    /* ===== INTERRUPT BYPASS =====
     * Handle interrupt packets ('i') immediately without queueing.
     * This allows interrupts to take effect even when TTS is blocking.
     */
    if (size > 0 && buffer[0] == 'i') {
      AUDIO_IO_PRINTF("INTERRUPT BYPASS: Handling interrupt immediately\n");
      hal_audio_interrupt();
      hal_tts_interrupt();

      /* Clear any queued audio packets so they don't play after interrupt */
      pthread_mutex_lock(&audio_queue_lock);
      clear_queue(queue);
      pthread_mutex_unlock(&audio_queue_lock);
      AUDIO_IO_PRINTF("INTERRUPT BYPASS: Cleared audio queue\n");

      /* Send acknowledgment directly to output pipe */
      int ack_result = 0;
      Inst_packet *ack_packet = create_inst_packet(
          AUDIO, sizeof(int), (unsigned char *)&ack_result, tag);
      write(o_pipe, ack_packet, 8);
      write(o_pipe, ack_packet->data, sizeof(int));
      destroy_inst_packet(&ack_packet);

      /* Release queue lock if we held it, then skip normal queue processing */
      if (queue_empty) {
        pthread_mutex_unlock(&audio_queue_available);
      }
      continue;
    }

    /* ===== BEEP BYPASS =====
     * Handle beep packets ('b') immediately without queueing.
     * This ensures beeps play right away and aren't cleared by subsequent
     * interrupts that may come from the callback processing the key event.
     */
    if (size > 0 && buffer[0] == 'b') {
      AUDIO_IO_PRINTF("BEEP BYPASS: Playing beep immediately\n");

      /* Clear interrupt to allow beep to play */
      hal_audio_clear_interrupt();

      /* Determine beep type from second character */
      BeepType beep_type = BEEP_KEYPRESS;
      if (size > 1) {
        switch (buffer[1]) {
        case 'k':
          beep_type = BEEP_KEYPRESS;
          break;
        case 'h':
          beep_type = BEEP_HOLD;
          break;
        case 'e':
          beep_type = BEEP_ERROR;
          break;
        }
      }

      int beep_result = hal_audio_play_beep(beep_type);
      AUDIO_IO_PRINTF("BEEP BYPASS: Beep returned %d\n", beep_result);

      /* Send acknowledgment directly to output pipe */
      Inst_packet *ack_packet = create_inst_packet(
          AUDIO, sizeof(int), (unsigned char *)&beep_result, tag);
      write(o_pipe, ack_packet, 8);
      write(o_pipe, ack_packet->data, sizeof(int));
      destroy_inst_packet(&ack_packet);

      /* Release queue lock if we held it, then skip normal queue processing */
      if (queue_empty) {
        pthread_mutex_unlock(&audio_queue_available);
      }
      continue;
    }

    Inst_packet *queued_packet = create_inst_packet(type, size, buffer, tag);

    AUDIO_IO_PRINTF("Locking queue\n");
    pthread_mutex_lock(&audio_queue_lock);

    AUDIO_IO_PRINTF("Queueing packet\n");
    enqueue(queue, queued_packet);

    AUDIO_IO_PRINTF("Releasing queue & making it accessible\n");
    pthread_mutex_unlock(&audio_queue_lock);
    if (queue_empty) {
      pthread_mutex_unlock(&audio_queue_available);
    }
    usleep(100);
  }
  return NULL;
}

void firmwareStartAudio() {
  AUDIO_PRINTF("Initializing audio mutex lock\n");
  if (pthread_mutex_init(&audio_lock, NULL) != 0) {
    perror("pthread_mutex_init");
    // kill(controller_pid, SIGINT);
    exit(1);
  }

  // Initialize HAL
  if (hal_audio_init() != 0) {
    AUDIO_PRINTF("Failed to initialize audio HAL\n");
  } else {
    AUDIO_PRINTF("Audio HAL initialized: %s\n", hal_audio_get_impl_name());
  }
}

int firmwarePlayAudio(void *text) {
  char *incomingText = (char *)text;
  char *buffer = malloc(sizeof(char) * (2 * strlen(incomingText) + 100 + 1));
  char audio_type_byte = incomingText[0];
  char *remaining_string = incomingText + 1;
  int system_result;
  char default_directory[0x100];
  printf("I am here before trying to print incomingText\n");
  printf("DEBUG: incomingText = '%s'\n", incomingText);
  printf("DEBUG: audio_type_byte = '%c'\n", audio_type_byte);
  printf("DEBUG: remaining_string = '%s'\n", remaining_string);
  pthread_mutex_lock(&audio_lock);
  getcwd(default_directory, sizeof(default_directory));
  if (audio_type_byte == 'd') {
    AUDIO_PRINTF("Festival tts without saving file");
    chdir("/tmp");
    sprintf(buffer, "echo '%s' | text2wave -o output.wav", remaining_string);
    system_result = system(buffer);
    // system_result = system("aplay output.wav");
    system_result = hal_audio_play_file("output.wav");
    chdir(default_directory);
  } else if (audio_type_byte == 's') {
    AUDIO_PRINTF("Festival tts %s with saving file\n", remaining_string);
    chdir("../Firmware/pregen_audio");
    sprintf(buffer, "echo '%s' | text2wave -o '%s.wav'", remaining_string,
            remaining_string);
    system_result = system(buffer);
    // sprintf(buffer, "aplay '%s.wav'", remaining_string);
    // system(buffer);
    sprintf(buffer, "%s.wav", remaining_string);
    system_result = hal_audio_play_file(buffer);
    chdir(default_directory);
  } else if (audio_type_byte == 'p') {
    // strcpy(buffer, "aplay '");
    // strcat(buffer, remaining_string);
    // strcat(buffer, ".wav'");
    //  sprintf(buffer, "aplay '%s.wav'", remaining_string);
    sprintf(buffer, "%s.wav", remaining_string);
    AUDIO_PRINTF("Now playing %s with HAL\n", remaining_string);
    // system_result = system(buffer);
    system_result = hal_audio_play_file(buffer);
  } else {
    AUDIO_PRINTF("Audio error. Unrecognized packet data %s\n", incomingText);
    system_result = -1;
  }
  pthread_mutex_unlock(&audio_lock);
  free(buffer);
  free(incomingText);
  return system_result;
}

/**
 * @brief Play a beep sound directly using HAL (low-latency)
 *
 * Uses pre-generated WAV files for minimum latency.
 * Thread-safe via audio_lock mutex.
 */
int audio_play_beep(BeepType type) {
  AUDIO_PRINTF("audio_play_beep: Playing beep type %d\n", type);

  /* Use new low-latency HAL function (plays from RAM cache) */
  int result = hal_audio_play_beep(type);

  if (result != 0) {
    AUDIO_PRINTF("audio_play_beep: Failed to play beep type %d\n", type);
  }

  return result;
}
