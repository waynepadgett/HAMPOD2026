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

// ============================================================================
// Packet Types (mirrored from Firmware/hampod_firm_packet.h)
// ============================================================================

typedef enum {
    PACKET_KEYPAD = 0,
    PACKET_AUDIO  = 1,
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
int comm_read_packet(CommPacket* packet);

/**
 * Request a keypad read from Firmware and wait for response.
 * 
 * Sends a KEYPAD request packet, waits for response.
 * 
 * @param key_out Pointer to store the key character (e.g., '5', 'A', '#')
 *                Returns '-' if no key pressed
 * @return HAMPOD_OK on success, HAMPOD_ERROR on failure
 */
int comm_read_keypad(char* key_out);

// ============================================================================
// Writing to Firmware
// ============================================================================

/**
 * Send a packet to Firmware.
 * 
 * @param packet Packet to send
 * @return HAMPOD_OK on success, HAMPOD_ERROR on write failure
 */
int comm_send_packet(const CommPacket* packet);

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
int comm_send_audio(char audio_type, const char* payload);

/**
 * Send audio and wait for Firmware acknowledgment.
 * 
 * Same as comm_send_audio() but blocks until Firmware confirms playback started.
 * 
 * @param audio_type Audio type character
 * @param payload Text or file path
 * @return HAMPOD_OK on success and ack received, HAMPOD_ERROR on failure
 */
int comm_send_audio_sync(char audio_type, const char* payload);

#endif // COMM_H
