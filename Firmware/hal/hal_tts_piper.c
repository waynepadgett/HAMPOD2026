/**
 * @file hal_tts_piper.c
 * @brief Piper TTS implementation of the TTS HAL with persistent subprocess
 *
 * Routes Piper's raw PCM output through the audio HAL for playback.
 * Keeps Piper running as a persistent subprocess for lower latency.
 * This makes TTS interruptible and avoids audio device conflicts.
 *
 * Phase 2: Persistent Piper implementation
 */

#include "hal_audio.h"
#include "hal_tts.h"
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/wait.h>
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

/* Timeout for end-of-utterance detection (microseconds) */
#define TTS_READ_TIMEOUT_US 100000 /* 100ms */

/* Persistent Piper process state */
static int initialized = 0;
static volatile int tts_interrupted = 0;

/* Persistent Piper pipes and process ID */
static FILE *piper_stdin = NULL; /* Write text to Piper here */
static int piper_stdout_fd = -1; /* Read raw PCM from Piper here */
static pid_t piper_pid = -1;     /* Piper process ID for cleanup */

/**
 * @brief Start the persistent Piper subprocess
 *
 * Forks and execs Piper with stdin for text and stdout for raw PCM audio.
 *
 * @return 0 on success, -1 on failure
 */
static int start_persistent_piper(void) {
  int stdin_pipe[2];  /* Parent writes, child reads */
  int stdout_pipe[2]; /* Child writes, parent reads */

  /* Create pipes */
  if (pipe(stdin_pipe) != 0 || pipe(stdout_pipe) != 0) {
    perror("HAL TTS: pipe() failed");
    return -1;
  }

  /* Fork */
  piper_pid = fork();

  if (piper_pid < 0) {
    perror("HAL TTS: fork() failed");
    close(stdin_pipe[0]);
    close(stdin_pipe[1]);
    close(stdout_pipe[0]);
    close(stdout_pipe[1]);
    return -1;
  }

  if (piper_pid == 0) {
    /* ===== CHILD PROCESS ===== */

    /* Close ALL inherited file descriptors (except the pipes we need)
     * This is CRITICAL - inherited handles to USB devices, sockets, or
     * system files could cause unexpected behavior if kept open.
     */
    int max_fd = sysconf(_SC_OPEN_MAX);
    if (max_fd < 0)
      max_fd = 1024; /* Fallback */
    for (int fd = 3; fd < max_fd; fd++) {
      /* Skip the pipe fds we need */
      if (fd != stdin_pipe[0] && fd != stdout_pipe[1]) {
        close(fd);
      }
    }

    /* Redirect stdin to read end of stdin_pipe */
    dup2(stdin_pipe[0], STDIN_FILENO);
    close(stdin_pipe[0]);
    close(stdin_pipe[1]); /* Close write end in child */

    /* Redirect stdout to write end of stdout_pipe */
    dup2(stdout_pipe[1], STDOUT_FILENO);
    close(stdout_pipe[0]); /* Close read end in child */
    close(stdout_pipe[1]);

    /* Redirect stderr to /dev/null to suppress Piper's status messages */
    int devnull = open("/dev/null", O_WRONLY);
    if (devnull >= 0) {
      dup2(devnull, STDERR_FILENO);
      close(devnull);
    }

    /* Execute Piper with persistent read from stdin */
    execlp("piper", "piper", "--model", PIPER_MODEL_PATH, "--length_scale",
           PIPER_SPEED, "--output_raw", NULL);

    /* execlp only returns on error */
    perror("HAL TTS: execlp(piper) failed");
    _exit(127);
  }

  /* ===== PARENT PROCESS ===== */
  /* Close unused ends */
  close(stdin_pipe[0]);  /* Close read end of stdin pipe */
  close(stdout_pipe[1]); /* Close write end of stdout pipe */

  /* Wrap stdin pipe in FILE* for fprintf/fflush */
  piper_stdin = fdopen(stdin_pipe[1], "w");
  if (piper_stdin == NULL) {
    perror("HAL TTS: fdopen() failed");
    close(stdin_pipe[1]);
    close(stdout_pipe[0]);
    kill(piper_pid, SIGTERM);
    waitpid(piper_pid, NULL, 0);
    piper_pid = -1;
    return -1;
  }

  /* Set line buffering for stdin to ensure text is sent immediately */
  setvbuf(piper_stdin, NULL, _IOLBF, 0);

  /* Store stdout file descriptor (we use raw fd for select() timeout) */
  piper_stdout_fd = stdout_pipe[0];

  printf("HAL TTS: Started persistent Piper process (pid=%d)\n", piper_pid);
  return 0;
}

/**
 * @brief Stop the persistent Piper subprocess
 */
static void stop_persistent_piper(void) {
  if (piper_stdin != NULL) {
    fclose(piper_stdin);
    piper_stdin = NULL;
  }

  if (piper_stdout_fd >= 0) {
    close(piper_stdout_fd);
    piper_stdout_fd = -1;
  }

  if (piper_pid > 0) {
    /* Send SIGTERM and wait */
    kill(piper_pid, SIGTERM);
    int status;
    waitpid(piper_pid, &status, 0);
    printf("HAL TTS: Stopped Piper process (pid=%d, status=%d)\n", piper_pid,
           WEXITSTATUS(status));
    piper_pid = -1;
  }
}

/**
 * @brief Check if persistent Piper is running
 */
static int is_piper_running(void) {
  if (piper_pid <= 0) {
    return 0;
  }
  /* Check if process is still alive */
  int status;
  pid_t result = waitpid(piper_pid, &status, WNOHANG);
  if (result == 0) {
    return 1; /* Still running */
  }
  /* Process exited */
  piper_pid = -1;
  return 0;
}

/* ============================================================================
 * Public API
 * ============================================================================
 */

int hal_tts_init(void) {
  if (initialized) {
    return 0;
  }

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

  /* 3. Start persistent Piper subprocess */
  if (start_persistent_piper() != 0) {
    fprintf(stderr, "HAL TTS: Failed to start persistent Piper\n");
    return -1;
  }

  printf("HAL TTS: Piper initialized (model=%s, speed=%s, persistent=yes)\n",
         PIPER_MODEL_PATH, PIPER_SPEED);
  initialized = 1;
  return 0;
}

int hal_tts_speak(const char *text, const char *output_file) {
  (void)output_file; /* Ignored - we stream directly */

  int16_t chunk_buffer[TTS_CHUNK_SAMPLES];
  ssize_t bytes_read;
  int received_any_audio = 0;

  if (!initialized) {
    if (hal_tts_init() != 0) {
      return -1;
    }
  }

  /* Ensure Piper is still running, restart if needed */
  if (!is_piper_running()) {
    printf("HAL TTS: Piper process died, restarting...\n");
    stop_persistent_piper();
    if (start_persistent_piper() != 0) {
      fprintf(stderr, "HAL TTS: Failed to restart Piper\n");
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

  /* Send text to Piper via stdin (with newline to trigger processing) */
  if (fprintf(piper_stdin, "%s\n", text) < 0 || fflush(piper_stdin) != 0) {
    fprintf(stderr, "HAL TTS: Failed to write to Piper stdin\n");
    return -1;
  }

  /* Stream Piper output through audio HAL in chunks.
   * Use select() with timeout to detect end of utterance. */
  while (!tts_interrupted) {
    fd_set read_fds;
    struct timeval timeout;

    FD_ZERO(&read_fds);
    FD_SET(piper_stdout_fd, &read_fds);

    /* Timeout: 100ms - if no data within this period after receiving some
     * audio, we assume the utterance is complete */
    timeout.tv_sec = 0;
    timeout.tv_usec = TTS_READ_TIMEOUT_US;

    int select_result =
        select(piper_stdout_fd + 1, &read_fds, NULL, NULL, &timeout);

    if (select_result < 0) {
      if (errno == EINTR) {
        continue; /* Interrupted by signal, retry */
      }
      perror("HAL TTS: select() error");
      break;
    }

    if (select_result == 0) {
      /* Timeout - no data available */
      if (received_any_audio) {
        /* We've received audio and now timed out - utterance complete */
        break;
      }
      /* Still waiting for first audio data, continue waiting */
      continue;
    }

    /* Data available - read it */
    bytes_read = read(piper_stdout_fd, chunk_buffer, TTS_CHUNK_BYTES);

    if (bytes_read <= 0) {
      if (bytes_read < 0 && errno == EINTR) {
        continue; /* Interrupted by signal, retry */
      }
      /* EOF or error - this shouldn't happen with persistent Piper */
      fprintf(stderr, "HAL TTS: Read returned %zd (Piper may have crashed)\n",
              bytes_read);
      break;
    }

    received_any_audio = 1;

    /* Write chunk to audio HAL */
    if (hal_audio_write_raw(chunk_buffer, bytes_read / 2) != 0) {
      fprintf(stderr, "HAL TTS: Audio write failed\n");
      break;
    }
  }

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
  stop_persistent_piper();
  initialized = 0;
  printf("HAL TTS: Piper cleaned up\n");
}

const char *hal_tts_get_impl_name(void) {
  return "Piper (Persistent Subprocess)";
}
