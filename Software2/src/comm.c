/**
 * comm.c - Pipe Communication Module Implementation
 * 
 * Implements communication with Firmware via named pipes using the
 * binary packet protocol defined in Firmware/hampod_firm_packet.h.
 * 
 * Part of Phase 0: Core Infrastructure (Step 1.1, 1.2)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "comm.h"

// ============================================================================
// Pipe Paths (relative to Software2 directory)
// ============================================================================

// From Software2's perspective, Firmware is ../Firmware
#define FIRMWARE_INPUT_PIPE  "../Firmware/Firmware_i"   // We write to this
#define FIRMWARE_OUTPUT_PIPE "../Firmware/Firmware_o"   // We read from this

// ============================================================================
// Module State
// ============================================================================

static int fd_firmware_out = -1;   // File descriptor for reading from Firmware
static int fd_firmware_in = -1;    // File descriptor for writing to Firmware
static unsigned short packet_tag = 0;  // Incrementing tag for packet matching

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
        LOG_ERROR("Is Firmware running? Start with: cd ../Firmware && ./firmware.elf");
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
        usleep(10000);  // Wait 10ms between attempts
    }
    
    if (fd_firmware_in == -1) {
        LOG_ERROR("Failed to open %s: %s", FIRMWARE_INPUT_PIPE, strerror(errno));
        close(fd_firmware_out);
        fd_firmware_out = -1;
        return HAMPOD_ERROR;
    }
    LOG_DEBUG("Opened %s (fd=%d)", FIRMWARE_INPUT_PIPE, fd_firmware_in);
    
    LOG_INFO("Firmware communication initialized successfully");
    return HAMPOD_OK;
}

void comm_close(void) {
    LOG_INFO("Closing Firmware communication...");
    
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
        return HAMPOD_OK;
    }
    
    LOG_ERROR("Unexpected ready packet data: 0x%02X", packet.data[0]);
    return HAMPOD_ERROR;
}

// ============================================================================
// Reading from Firmware
// ============================================================================

int comm_read_packet(CommPacket* packet) {
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
    
    LOG_DEBUG("comm_read_packet: type=%d, len=%u, tag=%u", 
              packet->type, packet->data_len, packet->tag);
    
    return HAMPOD_OK;
}

int comm_read_keypad(char* key_out) {
    if (key_out == NULL) {
        LOG_ERROR("comm_read_keypad: NULL key_out pointer");
        return HAMPOD_ERROR;
    }
    
    // Send a keypad request to Firmware
    CommPacket request = {
        .type = PACKET_KEYPAD,
        .data_len = 1,
        .tag = packet_tag++,
        .data = {'r'}  // 'r' = read request
    };
    
    if (comm_send_packet(&request) != HAMPOD_OK) {
        return HAMPOD_ERROR;
    }
    
    // Wait for response
    CommPacket response;
    if (comm_read_packet(&response) != HAMPOD_OK) {
        return HAMPOD_ERROR;
    }
    
    // Verify response type
    if (response.type != PACKET_KEYPAD) {
        LOG_ERROR("comm_read_keypad: Expected KEYPAD response, got type=%d", 
                  response.type);
        return HAMPOD_ERROR;
    }
    
    // Extract key from response
    if (response.data_len > 0) {
        *key_out = (char)response.data[0];
    } else {
        *key_out = '-';  // No key
    }
    
    LOG_DEBUG("comm_read_keypad: key='%c' (0x%02X)", *key_out, (unsigned char)*key_out);
    
    return HAMPOD_OK;
}

// ============================================================================
// Writing to Firmware
// ============================================================================

int comm_send_packet(const CommPacket* packet) {
    if (!comm_is_connected()) {
        LOG_ERROR("comm_send_packet: Not connected");
        return HAMPOD_ERROR;
    }
    
    if (packet == NULL) {
        LOG_ERROR("comm_send_packet: NULL packet pointer");
        return HAMPOD_ERROR;
    }
    
    LOG_DEBUG("comm_send_packet: type=%d, len=%u, tag=%u", 
              packet->type, packet->data_len, packet->tag);
    
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

int comm_send_audio(char audio_type, const char* payload) {
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
        .data_len = (unsigned short)(payload_len + 2),  // +1 for type, +1 for null
        .tag = packet_tag++
    };
    
    // First byte is audio type, then payload, then null terminator
    packet.data[0] = (unsigned char)audio_type;
    memcpy(packet.data + 1, payload, payload_len);
    packet.data[payload_len + 1] = '\0';  // Null terminate
    
    LOG_DEBUG("comm_send_audio: type='%c', payload='%s', len=%u", 
              audio_type, payload, packet.data_len);
    
    return comm_send_packet(&packet);
}

int comm_send_audio_sync(char audio_type, const char* payload) {
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
