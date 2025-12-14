# HAMPOD Firmware Build Guide

## Overview

The HAMPOD firmware runs on Raspberry Pi and provides hardware abstraction for USB keypad input and USB audio output. It communicates with the Software layer via named pipes.

## Architecture

```
Firmware/
├── firmware.c              # Main firmware controller
├── keypad_firmware.c/h     # Keypad process (uses HAL)
├── audio_firmware.c/h      # Audio process (uses HAL)
├── hal/                    # Hardware Abstraction Layer
│   ├── hal_keypad.h        # Keypad HAL interface
│   ├── hal_keypad_usb.c    # USB keypad implementation
│   ├── hal_audio.h         # Audio HAL interface
│   ├── hal_audio_usb.c     # USB audio implementation (ALSA)
│   ├── hal_tts.h           # TTS HAL interface
│   ├── hal_tts_piper.c     # Piper TTS implementation (default)
│   └── hal_tts_festival.c  # Festival TTS implementation (legacy)
├── models/                 # Voice models (downloaded by install script)
└── tests/                  # HAL test programs
```

## Dependencies

### Required Packages
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    libasound2-dev
```

### Optional: Festival TTS (Legacy)
```bash
sudo apt-get install -y festival festvox-kallpc16k
```

### Optional: Piper TTS (Recommended)
```bash
./Documentation/scripts/install_piper.sh
```

## TTS Engine Selection

The firmware supports two text-to-speech engines:

### Piper (Default) - Recommended
- Zero-latency persistent pipeline
- High-quality neural voice
- Requires one-time installation

**Install Piper:**
```bash
./Documentation/scripts/install_piper.sh
```

**Build with Piper (default):**
```bash
make
```

**Build with custom speed (1.0 = normal, 0.8 = faster):**
```bash
make TTS_SPEED=0.8
```

### Festival (Legacy)
- Uses Festival `text2wave` command
- Lower quality robotic voice
- Pre-installed on most Linux systems

**Build with Festival:**
```bash
make TTS_ENGINE=festival
```

## Building

### Standard Build
```bash
cd Firmware
make
```

This produces `firmware.elf` which can be run directly.

### Debug Build
```bash
cd Firmware
make debug
```

Builds with:
- `-DDEBUG` flag (enables debug printf statements)
- `-g` flag (includes debugging symbols)

### Clean Build
```bash
cd Firmware
make clean
make
```

## Running

### Prerequisites
1. USB keypad must be connected
2. USB audio device (speaker) must be connected
3. Software layer must be running (creates named pipes)

### Execution
```bash
cd Firmware
./firmware.elf
```

The firmware will:
1. Initialize HAL for keypad and audio
2. Create named pipes for IPC
3. Fork keypad and audio processes
4. Wait for commands from Software layer

### Debug Mode
When built with `make debug`, the firmware prints detailed status messages:
- Keypad events (key presses)
- Audio playback commands
- HAL initialization status
- Pipe communication

## Hardware Abstraction Layer (HAL)

### Keypad HAL
- **Interface**: `hal/hal_keypad.h`
- **Implementation**: `hal/hal_keypad_usb.c`
- **Device**: Reads from `/dev/input/by-id/*kbd*`
- **Key Mapping**: 19-key USB numeric keypad → HAMPOD key codes

### Audio HAL
- **Interface**: `hal/hal_audio.h`
- **Implementation**: `hal/hal_audio_usb.c`
- **Device**: Auto-detects USB audio via `aplay -l`
- **Playback**: Uses ALSA (`aplay`) with device specification

## Testing

### HAL Tests
```bash
cd Firmware/hal/tests
make
./test_hal_keypad    # Test keypad input
./test_hal_audio     # Test audio output
./test_integration   # Test keypad + TTS + audio
```

### Integration Test
```bash
cd Firmware/tests
./test_phase3        # Test audio firmware with HAL
```

## Troubleshooting

### Build Errors

**Error: `cannot find -lasound`**
```bash
sudo apt-get install libasound2-dev
```

**Error: `hal_keypad.h: No such file`**
- Ensure you're building from the `Firmware/` directory
- Check that `hal/` subdirectory exists

### Runtime Errors

**Error: `Failed to initialize keypad HAL`**
- Check USB keypad is connected: `ls /dev/input/by-id/*kbd*`
- Check permissions: `sudo chmod a+r /dev/input/event*`

**Error: `No audio devices found`**
- Check USB audio is connected: `aplay -l`
- Verify USB device is recognized: `lsusb`

**Error: `mkfifo: File exists`**
- Clean up old pipes: `rm -f Firmware/*_o Firmware/*_i`

## Development

### Adding New HAL Implementations

1. Create new implementation file (e.g., `hal/hal_keypad_bluetooth.c`)
2. Implement all functions from `hal/hal_keypad.h`
3. Update `Makefile` to use new implementation:
   ```makefile
   HAL_SRCS = hal/hal_keypad_bluetooth.c hal/hal_audio_usb.c
   ```
4. Rebuild: `make clean && make`

### Modifying Key Mappings

Edit `hal/hal_keypad_usb.c`:
```c
static const KeyMapping key_map[] = {
    {KEY_KP0, '0'},
    {KEY_KP1, '1'},
    // Add or modify mappings here
};
```

## Performance Notes

- **Keypad polling**: 100ms sleep between reads
- **Audio latency**: Depends on ALSA buffer size (typically <100ms)
- **IPC overhead**: Named pipes add minimal latency (<1ms)

## Migration from NanoPi

The firmware has been migrated from NanoPi (WiringPi + GPIO matrix keypad) to Raspberry Pi (HAL + USB devices):

- ✅ Removed all WiringPi dependencies
- ✅ Replaced GPIO matrix scanning with USB input events
- ✅ Replaced hardcoded audio with ALSA device detection
- ✅ Maintained identical packet protocol for Software layer compatibility
