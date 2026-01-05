/**
 * comm.c - Pipe Communication Module Implementation
 *
 * Implements communication with Firmware via named pipes using the
 * binary packet protocol defined in Firmware/hampod_firm_packet.h.
 *
 * Part of Phase 0: Core Infrastructure (Step 1.1, 1.2)
 * Updated for Router Thread architecture (comm_router_plan.md)
 */

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "comm.h"

// ============================================================================
// Pipe Paths (relative to Software2 directory)
// ============================================================================

// From Software2's perspective, Firmware is ../Firmware
#define FIRMWARE_INPUT_PIPE "../Firmware/Firmware_i"  // We write to this
#define FIRMWARE_OUTPUT_PIPE "../Firmware/Firmware_o" // We read from this

// ============================================================================
// Module State
// ============================================================================

static int fd_firmware_out = -1; // File descriptor for reading from Firmware
static int fd_firmware_in = -1;  // File descriptor for writing to Firmware
static unsigned short packet_tag = 0; // Incrementing tag for packet matching

// ============================================================================
// Response Queue Structure (Thread-safe circular buffer)
// ============================================================================

typedef struct {
  CommPacket packets[COMM_RESPONSE_QUEUE_SIZE];
  int head;                 // Index of first item
  int tail;                 // Index of next empty slot
  int count;                // Current number of items
  pthread_mutex_t mutex;    // Protects queue access
  pthread_cond_t not_empty; // Signaled when item added
} ResponseQueue;

// Response queues for each packet type
static ResponseQueue keypad_queue;
static ResponseQueue audio_queue;
static ResponseQueue config_queue;

// Router thread state
static pthread_t router_thread;
static volatile bool router_running = false;

// ============================================================================
// Response Queue Functions
// ============================================================================

static void response_queue_init(ResponseQueue *q) {
  q->head = 0;
  q->tail = 0;
  q->count = 0;
  pthread_mutex_init(&q->mutex, NULL);
  pthread_cond_init(&q->not_empty, NULL);
}

static void response_queue_destroy(ResponseQueue *q) {
  pthread_mutex_destroy(&q->mutex);
  pthread_cond_destroy(&q->not_empty);
}

static int response_queue_push(ResponseQueue *q, const CommPacket *packet) {
  pthread_mutex_lock(&q->mutex);

  if (q->count >= COMM_RESPONSE_QUEUE_SIZE) {
    // Queue full - drop oldest packet (overwrite head)
    LOG_ERROR("Response queue full, dropping oldest packet");
    q->head = (q->head + 1) % COMM_RESPONSE_QUEUE_SIZE;
    q->count--;
  }

  // Add packet to tail
  q->packets[q->tail] = *packet;
  q->tail = (q->tail + 1) % COMM_RESPONSE_QUEUE_SIZE;
  q->count++;

  // Signal that queue is not empty
  pthread_cond_signal(&q->not_empty);

  pthread_mutex_unlock(&q->mutex);
  return HAMPOD_OK;
}

static int response_queue_pop_timeout(ResponseQueue *q, CommPacket *packet,
                                      int timeout_ms) {
  pthread_mutex_lock(&q->mutex);

  // Wait for item with timeout
  while (q->count == 0 && router_running) {
    struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);

    // Add timeout_ms to current time
    timeout.tv_sec += timeout_ms / 1000;
    timeout.tv_nsec += (timeout_ms % 1000) * 1000000;
    if (timeout.tv_nsec >= 1000000000) {
      timeout.tv_sec += 1;
      timeout.tv_nsec -= 1000000000;
    }

    int result = pthread_cond_timedwait(&q->not_empty, &q->mutex, &timeout);
    if (result == ETIMEDOUT) {
      pthread_mutex_unlock(&q->mutex);
      return HAMPOD_TIMEOUT;
    }
  }

  if (!router_running && q->count == 0) {
    pthread_mutex_unlock(&q->mutex);
    return HAMPOD_ERROR;
  }

  if (q->count == 0) {
    pthread_mutex_unlock(&q->mutex);
    return HAMPOD_NOT_FOUND;
  }

  // Remove packet from head
  *packet = q->packets[q->head];
  q->head = (q->head + 1) % COMM_RESPONSE_QUEUE_SIZE;
  q->count--;

  pthread_mutex_unlock(&q->mutex);
  return HAMPOD_OK;
}

// ============================================================================
// Router Thread
// ============================================================================

static void *router_thread_func(void *arg) {
  (void)arg;

  LOG_INFO("Router thread started");

  while (router_running) {
    CommPacket packet;
    int result = comm_read_packet(&packet);

    if (result != HAMPOD_OK) {
      if (!router_running)
        break; // Expected shutdown
      LOG_ERROR("Router: Pipe read failed, retrying...");
      usleep(100000); // 100ms before retry
      continue;
    }

    switch (packet.type) {
    case PACKET_KEYPAD:
      if (response_queue_push(&keypad_queue, &packet) != HAMPOD_OK) {
        LOG_ERROR("Router: Keypad queue full, dropping packet");
      }
      break;
    case PACKET_AUDIO:
      if (response_queue_push(&audio_queue, &packet) != HAMPOD_OK) {
        LOG_ERROR("Router: Audio queue full, dropping packet");
      }
      break;
    case PACKET_CONFIG:
      if (response_queue_push(&config_queue, &packet) != HAMPOD_OK) {
        LOG_ERROR("Router: Config queue full, dropping packet");
      }
      break;
    default:
      LOG_ERROR("Router: Unknown packet type %d", packet.type);
      break;
    }
  }

  LOG_INFO("Router thread exiting");
  return NULL;
}

int comm_start_router(void) {
  if (router_running) {
    LOG_ERROR("Router already running");
    return HAMPOD_ERROR;
  }

  LOG_INFO("Starting router thread...");

  // Initialize response queues
  response_queue_init(&keypad_queue);
  response_queue_init(&audio_queue);
  response_queue_init(&config_queue);

  // Start router thread
  router_running = true;
  if (pthread_create(&router_thread, NULL, router_thread_func, NULL) != 0) {
    LOG_ERROR("Failed to create router thread");
    router_running = false;
    return HAMPOD_ERROR;
  }

  LOG_INFO("Router thread started");
  return HAMPOD_OK;
}

void comm_stop_router(void) {
  if (!router_running) {
    return;
  }

  LOG_INFO("Stopping router thread...");

  // Signal thread to stop
  router_running = false;

  // Wake up any threads waiting on queues
  pthread_cond_broadcast(&keypad_queue.not_empty);
  pthread_cond_broadcast(&audio_queue.not_empty);
  pthread_cond_broadcast(&config_queue.not_empty);

  // Wait for thread to finish
  // Note: pthread_join is blocking, but router_running=false should cause
  // the thread to exit quickly after its next read attempt
  pthread_join(router_thread, NULL);

  // Destroy queues
  response_queue_destroy(&keypad_queue);
  response_queue_destroy(&audio_queue);
  response_queue_destroy(&config_queue);

  LOG_INFO("Router thread stopped");
}

bool comm_router_is_running(void) { return router_running; }

int comm_wait_keypad_response(CommPacket *packet, int timeout_ms) {
  return response_queue_pop_timeout(&keypad_queue, packet, timeout_ms);
}

int comm_wait_audio_response(CommPacket *packet, int timeout_ms) {
  return response_queue_pop_timeout(&audio_queue, packet, timeout_ms);
}

// ============================================================================
// Initialization & Cleanup
// ============================================================================

int comm_init(void) {
  LOG_INFO("Initializing Firmware communication...");

  // Open Firmware_o for reading (Firmware -> Software)
  LOG_DEBUG("Opening %s for reading...", FIRMWARE_OUTPUT_PIPE);
  fd_firmware_out = open(FIRMWARE_OUTPUT_PIPE, O_RDONLY);
  if (fd_firmware_out == -1) {
    LOG_ERROR("Failed to open %s: %s", FIRMWARE_OUTPUT_PIPE, strerror(errno));
    LOG_ERROR(
        "Is Firmware running? Start with: cd ../Firmware && ./firmware.elf");
    return HAMPOD_ERROR;
  }
  LOG_DEBUG("Opened %s (fd=%d)", FIRMWARE_OUTPUT_PIPE, fd_firmware_out);

  // Open Firmware_i for writing (Software -> Firmware)
  // Retry loop since Firmware may not have opened its read end yet
  LOG_DEBUG("Opening %s for writing...", FIRMWARE_INPUT_PIPE);
  for (int attempt = 0; attempt < 100; attempt++) {
    fd_firmware_in = open(FIRMWARE_INPUT_PIPE, O_WRONLY | O_NONBLOCK);
    if (fd_firmware_in != -1) {
      // Success - switch back to blocking mode
      int flags = fcntl(fd_firmware_in, F_GETFL);
      fcntl(fd_firmware_in, F_SETFL, flags & ~O_NONBLOCK);
      break;
    }
    usleep(10000); // Wait 10ms between attempts
  }

  if (fd_firmware_in == -1) {
    LOG_ERROR("Failed to open %s: %s", FIRMWARE_INPUT_PIPE, strerror(errno));
    close(fd_firmware_out);
    fd_firmware_out = -1;
    return HAMPOD_ERROR;
  }
  LOG_DEBUG("Opened %s (fd=%d)", FIRMWARE_INPUT_PIPE, fd_firmware_in);

  LOG_INFO("Firmware communication initialized successfully");

  // Note: Router is NOT started here - it's started after comm_wait_ready()
  // because we need to read the ready packet directly first
  return HAMPOD_OK;
}

void comm_close(void) {
  LOG_INFO("Closing Firmware communication...");

  // Stop router thread first (if running)
  if (router_running) {
    comm_stop_router();
  }

  if (fd_firmware_out != -1) {
    close(fd_firmware_out);
    fd_firmware_out = -1;
  }

  if (fd_firmware_in != -1) {
    close(fd_firmware_in);
    fd_firmware_in = -1;
  }

  LOG_INFO("Firmware communication closed");
}

bool comm_is_connected(void) {
  return (fd_firmware_out != -1 && fd_firmware_in != -1);
}

int comm_wait_ready(void) {
  LOG_INFO("Waiting for Firmware ready signal...");

  // Read the ready packet directly (router not started yet)
  CommPacket packet;
  if (comm_read_packet(&packet) != HAMPOD_OK) {
    LOG_ERROR("Failed to read ready packet");
    return HAMPOD_ERROR;
  }

  // Verify it's the expected CONFIG packet with 'R'
  if (packet.type != PACKET_CONFIG) {
    LOG_ERROR("Expected CONFIG packet, got type=%d", packet.type);
    return HAMPOD_ERROR;
  }

  if (packet.data_len > 0 && packet.data[0] == 'R') {
    LOG_INFO("Firmware ready!");

    // NOW start the router thread (after ready signal received)
    if (comm_start_router() != HAMPOD_OK) {
      LOG_ERROR("Failed to start router thread");
      return HAMPOD_ERROR;
    }

    return HAMPOD_OK;
  }

  LOG_ERROR("Unexpected ready packet data: 0x%02X", packet.data[0]);
  return HAMPOD_ERROR;
}

// ============================================================================
// Reading from Firmware
// ============================================================================

int comm_read_packet(CommPacket *packet) {
  if (!comm_is_connected()) {
    LOG_ERROR("comm_read_packet: Not connected");
    return HAMPOD_ERROR;
  }

  if (packet == NULL) {
    LOG_ERROR("comm_read_packet: NULL packet pointer");
    return HAMPOD_ERROR;
  }

  // Read packet header fields (matching Firmware's write order)
  // 1. Packet_type (enum, typically 4 bytes on 32/64-bit systems)
  ssize_t n = read(fd_firmware_out, &packet->type, sizeof(PacketType));
  if (n != sizeof(PacketType)) {
    LOG_ERROR("comm_read_packet: Failed to read type (%zd bytes)", n);
    return HAMPOD_ERROR;
  }

  // 2. data_len (unsigned short, 2 bytes)
  n = read(fd_firmware_out, &packet->data_len, sizeof(unsigned short));
  if (n != sizeof(unsigned short)) {
    LOG_ERROR("comm_read_packet: Failed to read data_len");
    return HAMPOD_ERROR;
  }

  // 3. tag (unsigned short, 2 bytes)
  n = read(fd_firmware_out, &packet->tag, sizeof(unsigned short));
  if (n != sizeof(unsigned short)) {
    LOG_ERROR("comm_read_packet: Failed to read tag");
    return HAMPOD_ERROR;
  }

  // 4. data (variable length)
  if (packet->data_len > 0) {
    if (packet->data_len > COMM_MAX_DATA_LEN) {
      LOG_ERROR("comm_read_packet: data_len %u exceeds max %d",
                packet->data_len, COMM_MAX_DATA_LEN);
      return HAMPOD_ERROR;
    }

    n = read(fd_firmware_out, packet->data, packet->data_len);
    if (n != packet->data_len) {
      LOG_ERROR("comm_read_packet: Failed to read data (expected %u, got %zd)",
                packet->data_len, n);
      return HAMPOD_ERROR;
    }
  }

  LOG_DEBUG("comm_read_packet: type=%d, len=%u, tag=%u", packet->type,
            packet->data_len, packet->tag);

  return HAMPOD_OK;
}

int comm_read_keypad(char *key_out) {
  if (key_out == NULL) {
    LOG_ERROR("comm_read_keypad: NULL key_out pointer");
    return HAMPOD_ERROR;
  }

  // Send a keypad request to Firmware
  CommPacket request = {
      .type = PACKET_KEYPAD, .data_len = 1, .tag = packet_tag++, .data = {'r'}
      // 'r' = read request
  };

  if (comm_send_packet(&request) != HAMPOD_OK) {
    return HAMPOD_ERROR;
  }

  // Wait for response from router queue
  CommPacket response;
  int result = comm_wait_keypad_response(&response, COMM_KEYPAD_TIMEOUT_MS);

  if (result == HAMPOD_TIMEOUT) {
    LOG_ERROR("comm_read_keypad: Timeout waiting for response");
    return HAMPOD_TIMEOUT;
  }

  if (result != HAMPOD_OK) {
    LOG_ERROR("comm_read_keypad: Failed to get response");
    return HAMPOD_ERROR;
  }

  // Extract key from response
  if (response.data_len > 0) {
    *key_out = (char)response.data[0];
  } else {
    *key_out = '-'; // No key
  }

  LOG_DEBUG("comm_read_keypad: key='%c' (0x%02X)", *key_out,
            (unsigned char)*key_out);
  return HAMPOD_OK;
}

// ============================================================================
// Writing to Firmware
// ============================================================================

int comm_send_packet(const CommPacket *packet) {
  if (!comm_is_connected()) {
    LOG_ERROR("comm_send_packet: Not connected");
    return HAMPOD_ERROR;
  }

  if (packet == NULL) {
    LOG_ERROR("comm_send_packet: NULL packet pointer");
    return HAMPOD_ERROR;
  }

  LOG_DEBUG("comm_send_packet: type=%d, len=%u, tag=%u", packet->type,
            packet->data_len, packet->tag);

  // Write packet header fields (matching Firmware's read order)
  // 1. Packet_type
  ssize_t n = write(fd_firmware_in, &packet->type, sizeof(PacketType));
  if (n != sizeof(PacketType)) {
    LOG_ERROR("comm_send_packet: Failed to write type");
    return HAMPOD_ERROR;
  }

  // 2. data_len
  n = write(fd_firmware_in, &packet->data_len, sizeof(unsigned short));
  if (n != sizeof(unsigned short)) {
    LOG_ERROR("comm_send_packet: Failed to write data_len");
    return HAMPOD_ERROR;
  }

  // 3. tag
  n = write(fd_firmware_in, &packet->tag, sizeof(unsigned short));
  if (n != sizeof(unsigned short)) {
    LOG_ERROR("comm_send_packet: Failed to write tag");
    return HAMPOD_ERROR;
  }

  // 4. data
  if (packet->data_len > 0) {
    n = write(fd_firmware_in, packet->data, packet->data_len);
    if (n != packet->data_len) {
      LOG_ERROR("comm_send_packet: Failed to write data");
      return HAMPOD_ERROR;
    }
  }

  return HAMPOD_OK;
}

int comm_send_audio(char audio_type, const char *payload) {
  if (payload == NULL) {
    LOG_ERROR("comm_send_audio: NULL payload");
    return HAMPOD_ERROR;
  }

  // Build audio packet: first byte is type, rest is payload + null terminator
  // Format: <type><payload>\0
  // Example: "dHello World\0" for TTS
  size_t payload_len = strlen(payload);

  // +1 for audio_type byte, +1 for payload, +1 for null terminator
  if (payload_len + 2 > COMM_MAX_DATA_LEN) {
    LOG_ERROR("comm_send_audio: Payload too long (%zu bytes)", payload_len);
    return HAMPOD_ERROR;
  }

  CommPacket packet = {
      .type = PACKET_AUDIO,
      .data_len = (unsigned short)(payload_len + 2), // +1 for type, +1 for null
      .tag = packet_tag++};

  // First byte is audio type, then payload, then null terminator
  packet.data[0] = (unsigned char)audio_type;
  memcpy(packet.data + 1, payload, payload_len);
  packet.data[payload_len + 1] = '\0'; // Null terminate

  LOG_DEBUG("comm_send_audio: type='%c', payload='%s', len=%u", audio_type,
            payload, packet.data_len);

  return comm_send_packet(&packet);
}

int comm_send_audio_sync(char audio_type, const char *payload) {
  // Send the audio request
  if (comm_send_audio(audio_type, payload) != HAMPOD_OK) {
    return HAMPOD_ERROR;
  }

  // Wait for acknowledgment
  CommPacket response;
  if (comm_read_packet(&response) != HAMPOD_OK) {
    return HAMPOD_ERROR;
  }

  // Verify response type
  if (response.type != PACKET_AUDIO) {
    LOG_ERROR("comm_send_audio_sync: Expected AUDIO response, got type=%d",
              response.type);
    return HAMPOD_ERROR;
  }

  LOG_DEBUG("comm_send_audio_sync: Got acknowledgment");
  return HAMPOD_OK;
}

// ============================================================================
// Beep Audio Feedback
// ============================================================================

int comm_play_beep(CommBeepType beep_type) {
  /*
   * Sends a beep request to Firmware using the 'b' audio type.
   * Protocol: 'b' + beep_type_char where:
   *   'k' = keypress beep
   *   'h' = hold indicator beep
   *   'e' = error beep
   */
  char beep_char;

  switch (beep_type) {
  case COMM_BEEP_KEYPRESS:
    beep_char = 'k';
    break;
  case COMM_BEEP_HOLD:
    beep_char = 'h';
    break;
  case COMM_BEEP_ERROR:
    beep_char = 'e';
    break;
  default:
    LOG_ERROR("comm_play_beep: Unknown beep type %d", beep_type);
    return HAMPOD_ERROR;
  }

  // Create payload with just the beep type character
  char payload[2] = {beep_char, '\0'};

  LOG_DEBUG("comm_play_beep: Sending beep type='%c'", beep_char);

  // Send beep request (non-blocking - don't wait for ack)
  return comm_send_audio('b', payload);
}

// ============================================================================
// Audio Device Query
// ============================================================================

int comm_query_audio_card_number(int *card_number_out) {
  if (card_number_out == NULL) {
    LOG_ERROR("comm_query_audio_card_number: NULL output pointer");
    return HAMPOD_ERROR;
  }

  // Send audio info query (non-blocking)
  LOG_DEBUG("comm_query_audio_card_number: Querying Firmware...");

  if (comm_send_audio(AUDIO_TYPE_INFO, "") != HAMPOD_OK) {
    LOG_ERROR("comm_query_audio_card_number: Send failed");
    *card_number_out = 2; // Fallback
    return HAMPOD_ERROR;
  }

  // Wait for response from router
  CommPacket response;
  if (comm_wait_audio_response(&response, 5000) == HAMPOD_OK) {
    if (response.data_len >= sizeof(int)) {
      *card_number_out = *(int *)response.data;
      LOG_DEBUG("comm_query_audio_card_number: Got card number %d",
                *card_number_out);
      return HAMPOD_OK;
    }
  }

  // Fallback - use a reasonable default
  LOG_ERROR("comm_query_audio_card_number: No response, defaulting to card 2");
  *card_number_out = 2;
  return HAMPOD_OK;
}
