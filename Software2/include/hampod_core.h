/**
 * hampod_core.h - Core definitions for HAMPOD2026 Software
 *
 * This header provides:
 * - Debug logging macros (LOG_INFO, LOG_ERROR, LOG_DEBUG)
 * - Named pipe paths for Firmware communication
 * - Common type definitions (KeyPressEvent)
 * - Configuration constants
 *
 * Part of Phase 0: Core Infrastructure
 * See:
 * Documentation/Project_Overview_and_Onboarding/fresh-start-phase-zero-plan.md
 */

#ifndef HAMPOD_CORE_H
#define HAMPOD_CORE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Debug Configuration
// ============================================================================

// Debug levels
#define DEBUG_NONE 0
#define DEBUG_ERROR 1
#define DEBUG_INFO 2
#define DEBUG_DEBUG 3

// Current debug level (can override at compile time: -DDEBUG_LEVEL=3)
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL DEBUG_INFO
#endif

// Debug macros
#if DEBUG_LEVEL >= DEBUG_ERROR
#define LOG_ERROR(fmt, ...) fprintf(stderr, "[ERROR] " fmt "\n", ##__VA_ARGS__)
#else
#define LOG_ERROR(fmt, ...)
#endif

#if DEBUG_LEVEL >= DEBUG_INFO
#define LOG_INFO(fmt, ...) printf("[INFO] " fmt "\n", ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)
#endif

#if DEBUG_LEVEL >= DEBUG_DEBUG
#define LOG_DEBUG(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)
#define DEBUG_PRINT(fmt, ...) printf("[DEBUG] " fmt, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...)
#define DEBUG_PRINT(fmt, ...)
#endif

// ============================================================================
// Named Pipe Paths (Communication with Firmware)
// ============================================================================

// These pipes are created and managed by the Firmware process
// Software2 connects to them after Firmware is running

// Keypad events: Firmware → Software (read from this)
#define PIPE_KEYPAD_OUT "../Firmware/Keypad_o"

// Audio requests: Software → Firmware (write to this)
#define PIPE_FIRMWARE_IN "../Firmware/Firmware_i"

// Firmware status: Firmware → Software (read from this, optional)
#define PIPE_FIRMWARE_OUT "../Firmware/Firmware_o"

// ============================================================================
// Key Press Event Structure
// ============================================================================

/**
 * KeyPressEvent - Represents a single keypad event
 *
 * Reused concept from old Software/GeneralFunctions.h
 * Original struct: { keyPressed, shiftAmount, isHold }
 */
typedef struct {
  char key;        // The character pressed (e.g., '1', 'A', '#', '-' for none)
  int shiftAmount; // 0 = normal, 1+ = shifted (reserved for future use)
  bool isHold;     // true if this is a long press (held > HOLD_THRESHOLD_MS)
} KeyPressEvent;

// Hold detection threshold (milliseconds)
#define HOLD_THRESHOLD_MS 500

// ============================================================================
// Audio Packet Types (for Firmware communication)
// ============================================================================

// Audio packet format: <type><payload>
// Example: "dHello World" = speak "Hello World" via TTS
// Example: "p/path/to/file.wav" = play WAV file

#define AUDIO_TYPE_TTS 'd'       // Dynamic TTS: speak text
#define AUDIO_TYPE_FILE 'p'      // Play pre-recorded WAV file
#define AUDIO_TYPE_SPELL 's'     // Spell out text character by character
#define AUDIO_TYPE_BEEP 'b'      // Play pre-cached beep
#define AUDIO_TYPE_INTERRUPT 'i' // Interrupt current playback
#define AUDIO_TYPE_INFO 'q' // Query audio device info (returns card number)

// ============================================================================
// Common Return Codes
// ============================================================================

#define HAMPOD_OK 0
#define HAMPOD_ERROR -1
#define HAMPOD_TIMEOUT -2
#define HAMPOD_NOT_FOUND -3

#endif // HAMPOD_CORE_H
