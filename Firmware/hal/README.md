# HAMPOD Hardware Abstraction Layer (HAL)

## Overview

The HAL provides a unified interface for different hardware implementations, allowing the firmware to remain hardware-agnostic. This makes it easy to port HAMPOD to different hardware platforms without modifying the core firmware logic.

## Architecture

```
┌─────────────────────────────────────┐
│     Firmware Core (firmware.c)     │
│  (Hardware-independent logic)       │
└──────────────┬──────────────────────┘
               │
               ├── Keypad Interface (hal_keypad.h)
               │   ├── USB Implementation (hal_keypad_usb.c)
               │   └── Matrix Implementation (hal_keypad_matrix.c)
               │
               └── Audio Interface (hal_audio.h)
                   ├── USB Implementation (hal_audio_usb.c)
                   └── Onboard Implementation (hal_audio_onboard.c)
```

## Keypad HAL

**Interface**: `hal_keypad.h`

Provides:
- `KeypadEvent` structure for key events
- `hal_keypad_init()` - Initialize hardware
- `hal_keypad_read()` - Read key (non-blocking)
- `hal_keypad_cleanup()` - Release resources

**USB Implementation**: `hal_keypad_usb.c`
- Reads from `/dev/input/eventX` using Linux input subsystem
- Maps USB keycodes to HAMPOD symbols (0-9, A-D, *, #)
- Auto-detects USB numeric keypad

**Key Mapping** (19-key USB keypad):
- 0-9: Direct numeric mapping
- `/` → A, `*` → B, `-` → C, `+` → D
- `ENTER` → #
- Arrow keys and other special keys can be extended

## Audio HAL

**Interface**: `hal_audio.h`

Provides:
- `hal_audio_init()` - Initialize and detect audio device
- `hal_audio_set_device()` - Manually set ALSA device
- `hal_audio_play_file()` - Play WAV file
- `hal_audio_cleanup()` - Release resources

**USB Implementation**: `hal_audio_usb.c`
- Detects USB audio device using `aplay -l`
- Uses ALSA (`aplay`) for playback
- Supports manual device configuration

## Usage in Firmware

```c
#include "hal/hal_keypad.h"
#include "hal/hal_audio.h"

int main() {
    // Initialize HAL
    hal_keypad_init();
    hal_audio_init();
    
    // Read keypad
    KeypadEvent event = hal_keypad_read();
    if (event.valid) {
        printf("Key pressed: %c\n", event.key);
    }
    
    // Play audio
    hal_audio_play_file("/path/to/file.wav");
    
    // Cleanup
    hal_keypad_cleanup();
    hal_audio_cleanup();
    
    return 0;
}
```

## Adding New Implementations

To add support for new hardware:

1. Create new implementation file (e.g., `hal_keypad_bluetooth.c`)
2. Implement all functions from the interface header
3. Update makefile to compile desired implementation
4. No changes needed to firmware core!

## Build Configuration

Select implementation at compile time by linking the appropriate `.c` file:

```makefile
# For USB hardware (Raspberry Pi)
HAL_OBJS = hal/hal_keypad_usb.o hal/hal_audio_usb.o

# For matrix keypad (NanoPi)
HAL_OBJS = hal/hal_keypad_matrix.o hal/hal_audio_onboard.o
```

## Testing

Each HAL implementation can be tested independently:

```bash
# Test USB keypad detection
evtest /dev/input/by-id/*kbd*

# Test audio device
aplay -l
aplay -D plughw:CARD=Device,DEV=0 test.wav
```

## Benefits

- **Portability**: Easy to port to new hardware
- **Maintainability**: Clear separation of concerns
- **Testability**: HAL can be mocked for unit testing
- **Flexibility**: Swap implementations without touching firmware
