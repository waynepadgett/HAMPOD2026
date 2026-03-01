/**
 * comm.h - Pipe Communication Module
 *
 * Abstracts the low-level pipe I/O for communication with Firmware.
 *
 * The Firmware creates and manages named pipes:
 * - Firmware_o: Firmware -> Software (we read keypad events, audio acks)
 * - Firmware_i: Software -> Firmware (we write audio requests)
 * - Keypad_o: (Legacy) Direct keypad output - NOT USED in packet mode
 *
 * Communication uses binary packets (see Firmware/hampod_firm_packet.h):
 * - Packet_type (4 bytes): KEYPAD=0, AUDIO=1, SERIAL=2, CONFIG=3
 * - data_len (2 bytes): Length of payload
 * - tag (2 bytes): Sequence tag for matching requests/responses
 * - data (variable): Payload bytes
 *
 * Part of Phase 0: Core Infrastructure (Step 1.1, 1.2)
 */

#ifndef COMM_H
#define COMM_H

#include "hampod_core.h"
#include <stdint.h>

// ============================================================================
// Packet Types (mirrored from Firmware/hampod_firm_packet.h)
// ============================================================================

typedef enum {
  PACKET_KEYPAD = 0,
  PACKET_AUDIO = 1,
  PACKET_SERIAL = 2,
  PACKET_CONFIG = 3
} PacketType;

// ============================================================================
// Packet Structure
// ============================================================================

#define COMM_MAX_DATA_LEN 256

typedef struct {
  PacketType type;
  unsigned short data_len;
  unsigned short tag;
  unsigned char data[COMM_MAX_DATA_LEN];
} CommPacket;

// ============================================================================
// Initialization & Cleanup
// ============================================================================

/**
 * Initialize communication with Firmware.
 * Opens Firmware_o (for reading) and Firmware_i (for writing).
 *
 * @return HAMPOD_OK on success, HAMPOD_ERROR on failure
 *
 * Note: Firmware must be running and pipes must exist before calling this.
 */
int comm_init(void);

/**
 * Close all open pipes and cleanup resources.
 */
void comm_close(void);

/**
 * Check if communication is initialized.
 * @return true if connected to Firmware, false otherwise
 */
bool comm_is_connected(void);

/**
 * Wait for Firmware "ready" signal.
 *
 * After comm_init(), the Firmware sends a CONFIG packet with 'R' to indicate
 * it's ready for commands. Call this before sending any requests.
 *
 * @return HAMPOD_OK if ready signal received, HAMPOD_ERROR otherwise
 */
int comm_wait_ready(void);

// ============================================================================
// Reading from Firmware
// ============================================================================

/**
 * Read a packet from Firmware (blocking).
 *
 * @param packet Pointer to packet structure to fill
 * @return HAMPOD_OK on success, HAMPOD_ERROR on read failure
 *
 * This blocks until a complete packet is received.
 */
int comm_read_packet(CommPacket *packet);

/**
 * Request a keypad read from Firmware and wait for response.
 *
 * Sends a KEYPAD request packet, waits for response.
 *
 * @param key_out Pointer to store the key character (e.g., '5', 'A', '#')
 *                Returns '-' if no key pressed
 * @return HAMPOD_OK on success, HAMPOD_ERROR on failure
 */
int comm_read_keypad(char *key_out);

// ============================================================================
// Writing to Firmware
// ============================================================================

/**
 * Send a packet to Firmware.
 *
 * @param packet Packet to send
 * @return HAMPOD_OK on success, HAMPOD_ERROR on write failure
 */
int comm_send_packet(const CommPacket *packet);

/**
 * Send an audio request to Firmware.
 *
 * @param audio_type One of: AUDIO_TYPE_TTS ('d'), AUDIO_TYPE_FILE ('p'),
 *                   AUDIO_TYPE_SPELL ('s')
 * @param payload The text to speak or file path to play
 * @return HAMPOD_OK on success, HAMPOD_ERROR on failure
 *
 * Example:
 *   comm_send_audio(AUDIO_TYPE_TTS, "Hello World");
 *   comm_send_audio(AUDIO_TYPE_FILE, "pregen_audio/5");
 */
int comm_send_audio(char audio_type, const char *payload);

/**
 * Send audio and wait for Firmware acknowledgment.
 *
 * Same as comm_send_audio() but blocks until Firmware confirms playback
 * started.
 *
 * @param audio_type Audio type character
 * @param payload Text or file path
 * @return HAMPOD_OK on success and ack received, HAMPOD_ERROR on failure
 */
int comm_send_audio_sync(char audio_type, const char *payload);

/**
 * Send a configuration packet to Firmware.
 *
 * @param sub_cmd Configuration sub-command (e.g., 0x01 for Keypad Layout)
 * @param value Configuration value
 * @return HAMPOD_OK on success, HAMPOD_ERROR on write failure
 */
int comm_send_config_packet(uint8_t sub_cmd, uint8_t value);

// ============================================================================
// Beep Audio Feedback
// ============================================================================

/**
 * Beep types for audio feedback.
 *
 * These correspond to the BeepType enum in Firmware/audio_firmware.h.
 */
typedef enum {
  COMM_BEEP_KEYPRESS = 0, /**< Short beep on key press (1000Hz, 50ms) */
  COMM_BEEP_HOLD = 1,     /**< Lower-pitch hold indicator (700Hz, 50ms) */
  COMM_BEEP_ERROR = 2     /**< Low-frequency error beep (400Hz, 100ms) */
} CommBeepType;

/**
 * Request a beep from Firmware (non-blocking).
 *
 * This sends a beep request to Firmware which plays a pre-generated
 * beep audio file. Used for keypad feedback.
 *
 * @param beep_type The type of beep to play
 * @return HAMPOD_OK on success, HAMPOD_ERROR on failure
 *
 * @note Beep audio files must exist on RPi. Generate with:
 *       cd ~/HAMPOD2026/Firmware/pregen_audio && ./generate_beeps.sh
 */
int comm_play_beep(CommBeepType beep_type);

/**
 * Set speech speed for TTS.
 *
 * Sends a speed setting to Firmware which restarts Piper with the new
 * --length_scale value. Lower values = faster speech.
 *
 * @param speed Speed multiplier (0.5 = faster, 1.0 = normal, 2.0 = slower)
 * @return HAMPOD_OK on success, HAMPOD_ERROR on failure
 */
int comm_set_speech_speed(float speed);

/**
 * Query the audio card number from Firmware.
 *
 * Sends an AUDIO_TYPE_INFO request and waits for the response
 * containing the ALSA card number of the selected audio device.
 *
 * @param card_number_out Pointer to store the card number (0-255)
 * @return HAMPOD_OK on success, HAMPOD_ERROR on failure
 */
int comm_query_audio_card_number(int *card_number_out);

// ============================================================================
// Router Thread (dispatches responses to type-specific queues)
// ============================================================================

// Response queue size for each packet type
#define COMM_RESPONSE_QUEUE_SIZE 16

// Timeout values for waiting on responses (in milliseconds)
#define COMM_KEYPAD_TIMEOUT_MS 5000 // 5 seconds for keypad
#define COMM_AUDIO_TIMEOUT_MS 30000 // 30 seconds for audio (speech can be long)

/**
 * Start the router thread.
 *
 * The router thread reads ALL packets from Firmware_o and dispatches them
 * to type-specific queues. This allows keypad and speech threads to operate
 * concurrently without packet conflicts.
 *
 * Called automatically by comm_init().
 *
 * @return HAMPOD_OK on success, HAMPOD_ERROR on failure
 */
int comm_start_router(void);

/**
 * Stop the router thread.
 *
 * Called automatically by comm_close().
 */
void comm_stop_router(void);

/**
 * Check if the router thread is running.
 * @return true if router is active, false otherwise
 */
bool comm_router_is_running(void);

/**
 * Wait for a KEYPAD response packet from the router queue.
 *
 * @param packet Pointer to packet structure to fill
 * @param timeout_ms Maximum time to wait (use COMM_KEYPAD_TIMEOUT_MS)
 * @return HAMPOD_OK on success, HAMPOD_TIMEOUT on timeout, HAMPOD_ERROR on
 * failure
 */
int comm_wait_keypad_response(CommPacket *packet, int timeout_ms);

/**
 * Wait for an AUDIO response packet from the router queue.
 *
 * @param packet Pointer to packet structure to fill
 * @param timeout_ms Maximum time to wait (use COMM_AUDIO_TIMEOUT_MS)
 * @return HAMPOD_OK on success, HAMPOD_TIMEOUT on timeout, HAMPOD_ERROR on
 * failure
 */
int comm_wait_audio_response(CommPacket *packet, int timeout_ms);

#endif // COMM_H
