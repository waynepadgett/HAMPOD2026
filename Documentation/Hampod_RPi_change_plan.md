# HAMPOD Raspberry Pi Migration Plan

## Executive Summary

This document outlines the plan to migrate the HAMPOD project from NanoPi hardware to Raspberry Pi hardware. The migration involves changing from a hardwired 4x4 matrix keypad (16 buttons) to a USB numeric keypad (19 buttons), and from onboard audio to USB audio output. The goal is to maintain the existing software architecture, Hamlib integration, and speech synthesis functionality while adapting the hardware interface layer.

## Progress Status

**Last Updated:** December 13, 2025

### ✅ Completed Phases

#### Phase 1: Hardware Abstraction Layer (HAL) - COMPLETE
- ✅ Created HAL directory structure (`Firmware/hal/`)
- ✅ Implemented `hal_keypad.h` and `hal_keypad_usb.c`
- ✅ Implemented `hal_audio.h` and `hal_audio_usb.c`
- ✅ Created test programs (`test_hal_keypad`, `test_hal_audio`)
- ✅ Verified HAL functionality on Raspberry Pi
- ✅ Updated keypad mapping (NUM_LOCK→X, BACKSPACE→Y, 00 debouncing)

#### Phase 1A: Integration Test - COMPLETE
- ✅ Created `test_hal_integration.c` (keypad + TTS + audio)
- ✅ Verified end-to-end functionality
- ✅ Confirmed Festival TTS integration works

#### Phase 2: USB Keypad Integration - COMPLETE
- ✅ Modified `keypad_firmware.c` to use HAL
- ✅ Removed all WiringPi dependencies
- ✅ Updated `Makefile` to link HAL sources
- ✅ Fixed legacy bugs (buffer overflow in `firmware.c`)
- ✅ Verified firmware builds successfully

#### Phase 3: USB Audio Integration - COMPLETE
- ✅ Modified `audio_firmware.c` to use HAL
- ✅ Replaced `aplay` system calls with `hal_audio_play_file()`
- ✅ Added `-lasound` to Makefile
- ✅ Fixed `firmwarePlayAudio()` return type bug
- ✅ Created `test_phase3.c` for verification
- ✅ Verified audio playback through USB speaker
- ✅ Debugged and fixed segfault in test program

#### Phase 4: Firmware Refactoring - COMPLETE
- ✅ Reviewed and improved build system
- ✅ Enhanced Makefile with dependency tracking and HAL object compilation
- ✅ Created comprehensive `BUILD.md` documentation
- ✅ Updated `Firmware/README.md` with Raspberry Pi details
- ✅ Verified clean build on Pi

#### Phase 4.5: Firmware Stability & Integration Fixes - COMPLETE
- ✅ Fixed critical connection bug in `imitation_software` (race condition + zombie processes)
- ✅ Implemented `hal_audio_init()` in `audio_firmware.c` (fixed "silence" bug)
- ✅ Patched `imitation_software` to handle "No Key" (`-`) packets preventing segfaults
- ✅ Created robust deployment/test scripts (`deploy_and_run_imitation.ps1`, `run_remote_test.sh`)
- ✅ Verified stable 15-second bidirectional communication (Audio + Keypad)

#### Phase 5: Software Layer Verification - IN PROGRESS
- ✅ Analyzed Software layer structure and dependencies
- ✅ Removed `-lwiringPi` from Software Makefile
- ✅ Fixed compilation errors (21 separate fixes across multiple files)
- ✅ Refactored build system from unity build to shared library (`libModes.so`)
- ✅ Implemented pipe communication in `FirmwareCommunication.c`
- ✅ Fixed `pthread_create` type mismatch in `FirmwareCommunication.c`
- ✅ Restored corrupted `RigListCreator.c` and fixed Hamlib callback signature
- ✅ Added HAL object linking to Software Makefile
- ✅ Resolved static declaration conflicts by adding `-DSHAREDLIB` flag
- ✅ Added missing includes to standalone-compiled source files
- 🚧 Completing final compilation fixes (`ModeRouting.c`)
- ⏳ Integration testing (Firmware + Software)
- ⏳ Verifying pipe communication
- ⏳ Testing keypad and audio flows

#### Phase 6-9: Build System, Testing, Documentation, Deployment (Not Started)

### Key Achievements
- **Hardware Independence**: Firmware now uses HAL exclusively
- **USB Devices Working**: Both keypad and audio verified functional
- **No WiringPi Dependencies**: Successfully removed all NanoPi-specific code
- **Bug Fixes**: Fixed multiple pre-existing bugs during migration
- **Test Coverage**: Created comprehensive test suite for HAL components


## Hardware Changes

### Current (NanoPi) Configuration
- **Keypad**: 4x4 hardwired matrix keypad (16 buttons: 0-9, A-D, *, #)
- **GPIO Library**: WiringNP
- **Audio**: Onboard audio via ALSA
- **Keypad Interface**: Direct GPIO pin reading (rows/columns matrix scanning)

### New (Raspberry Pi) Configuration
- **Keypad**: USB numeric keypad (19 buttons visible in image)
- **GPIO Library**: WiringPi or pigpio (for any remaining GPIO needs)
- **Audio**: USB speaker via ALSA
- **Keypad Interface**: Linux input event system (`/dev/input/eventX`)

### Button Mapping Analysis

**NanoPi 4x4 Matrix (16 buttons):**
```
1(0)  2(1)  3(2)  A(3)
4(4)  5(5)  6(6)  B(7)
7(8)  8(9)  9(10) C(11)
*(12) 0(13) #(14) D(15)
```

**USB Keypad (19 buttons from image):**
```
NUM_LOCK  /        *        BACKSPACE
7(HOME)   8(↑)     9(PG_UP) -
4(←)      5        6(→)     +
1(END)    2(↓)     3(PG_DN)
0(INS)    00       .(DEL)   ENTER
```

**Key Count:** 19 keys total (more than the original 16-button matrix keypad)

**Proposed Button Mapping:**
The USB keypad has **3 more buttons** than the original, providing flexibility for additional functions:

**Direct Numeric Mapping (10 keys):**
- `0-9` → Direct mapping to HAMPOD numeric functions

**Function Key Mapping (4 keys needed, 9 available):**
- `/` → A function
- `*` → B function (or keep as * for compatibility)
- `-` → C function  
- `+` → D function

**Special Function Mapping (remaining 5 keys):**
- `ENTER` → # function (confirm/execute)
- `BACKSPACE` → Delete/cancel function
- `.` (DEL) → Decimal point or alternative cancel
- `00` → Quick "00" entry or shift modifier
- `NUM_LOCK` → Mode toggle or settings access

**Navigation Keys (can be used for menu navigation or additional features):**
- Arrow keys (↑, ↓, ←, →) → Menu navigation
- HOME, END, PG_UP, PG_DN → Quick navigation or frequency stepping

This gives you **3 extra programmable keys** beyond the original 16-button layout, which could be used for:
- Quick frequency presets
- Band switching
- Volume control
- Additional shift/modifier keys

## Migration Strategy

### Phase 1: Hardware Abstraction Layer (HAL)

Create a hardware abstraction layer to isolate hardware-specific code from the application logic.

#### 1.1 Create New Directory Structure
```
Firmware/
├── hal/
│   ├── hal_keypad.h          # Keypad HAL interface
│   ├── hal_keypad_usb.c      # USB keypad implementation
│   ├── hal_keypad_matrix.c   # Original matrix keypad (for reference)
│   ├── hal_audio.h           # Audio HAL interface
│   ├── hal_audio_usb.c       # USB audio implementation
│   └── hal_audio_onboard.c   # Original onboard audio (for reference)
```

#### 1.2 Define HAL Interfaces

**hal_keypad.h:**
```c
typedef struct {
    char key;           // Character representation ('0'-'9', 'A'-'D', '*', '#')
    int raw_code;       // Raw keycode from device
    unsigned char valid; // 1 if valid key, 0 if invalid/multiple
} KeypadEvent;

// Initialize keypad hardware
int hal_keypad_init(void);

// Read a keypad event (non-blocking)
KeypadEvent hal_keypad_read(void);

// Cleanup keypad resources
void hal_keypad_cleanup(void);
```

**hal_audio.h:**
```c
// Initialize audio hardware
int hal_audio_init(void);

// Set audio output device
int hal_audio_set_device(const char* device_name);

// Play audio file
int hal_audio_play_file(const char* filepath);

// Cleanup audio resources
void hal_audio_cleanup(void);
```

### Phase 2: USB Keypad Implementation

#### 2.1 USB Input Event Reading

Replace GPIO-based matrix scanning with Linux input event reading:

**Key Implementation Points:**
- Use `/dev/input/by-id/` or `/dev/input/by-path/` for consistent device identification
- Implement event reading using `<linux/input.h>` structures
- Handle key press and release events
- Map USB keycodes to HAMPOD key symbols

**File: `hal_keypad_usb.c`**
```c
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>

static int keypad_fd = -1;
static const char* KEYPAD_DEVICE = "/dev/input/by-id/usb-*-kbd";

// Keycode mapping table
static const struct {
    int keycode;
    char symbol;
} keymap[] = {
    {KEY_KP0, '0'},
    {KEY_KP1, '1'},
    {KEY_KP2, '2'},
    {KEY_KP3, '3'},
    {KEY_KP4, '4'},
    {KEY_KP5, '5'},
    {KEY_KP6, '6'},
    {KEY_KP7, '7'},
    {KEY_KP8, '8'},
    {KEY_KP9, '9'},
    {KEY_KPSLASH, 'A'},    // / → A
    {KEY_KPASTERISK, 'B'}, // * → B (or keep as *)
    {KEY_KPMINUS, 'C'},    // - → C
    {KEY_KPPLUS, 'D'},     // + → D
    {KEY_KPENTER, '#'},    // ENTER → #
    // Add more mappings as needed
};

int hal_keypad_init(void) {
    // Open input device
    // Set non-blocking mode
    // Configure event filtering
}

KeypadEvent hal_keypad_read(void) {
    // Read input_event structure
    // Filter for EV_KEY events
    // Map keycode to symbol
    // Return KeypadEvent
}
```

#### 2.2 Device Detection Script

Create a helper script to identify the USB keypad device:

**File: `scripts/find_usb_keypad.sh`**
```bash
#!/bin/bash
# Find USB numeric keypad device
for device in /dev/input/by-id/*kbd*; do
    if [ -e "$device" ]; then
        echo "Found keyboard device: $device"
        evtest "$device" 2>&1 | head -20
    fi
done
```

### Phase 3: USB Audio Implementation

#### 3.1 ALSA USB Audio Configuration

**Key Changes:**
- Detect USB audio device using `aplay -l`
- Configure ALSA to use USB device as default
- Update `aplay` commands to specify device explicitly

**File: `hal_audio_usb.c`**
```c
static char usb_audio_device[256] = "plughw:CARD=Device,DEV=0";

int hal_audio_init(void) {
    // Detect USB audio device
    // Parse output of: aplay -l
    // Set usb_audio_device string
}

int hal_audio_play_file(const char* filepath) {
    char command[512];
    snprintf(command, sizeof(command), 
             "aplay -D %s '%s'", 
             usb_audio_device, filepath);
    return system(command);
}
```

#### 3.2 Audio Device Detection Script

**File: `scripts/find_usb_audio.sh`**
```bash
#!/bin/bash
# Find USB audio device
aplay -l | grep -i "usb\|card"
echo ""
echo "Recommended device string:"
aplay -l | grep "card" | head -1 | awk '{print "plughw:CARD="$2",DEV=0"}' | tr -d ','
```

### Phase 4: Firmware Refactoring

#### 4.1 Update `keypad_firmware.c`

**Changes Required:**
1. Remove WiringNP/WiringPi dependencies
2. Replace `readNumPad()` function with HAL call
3. Update `keypad_process()` initialization
4. Keep packet communication protocol unchanged

**Modified Functions:**
```c
// OLD: Direct GPIO reading
int readNumPad() {
    // GPIO matrix scanning code
}

// NEW: HAL-based reading
int readNumPad() {
    KeypadEvent event = hal_keypad_read();
    if (event.valid) {
        return event.key;
    }
    return '-'; // Invalid/no key
}

// Update initialization
void keypad_process() {
    // Remove wiringPiSetup() and pinMode() calls
    if (hal_keypad_init() != 0) {
        perror("Keypad HAL initialization failed");
        exit(1);
    }
    // Rest of the function remains the same
}
```

#### 4.2 Update `audio_firmware.c`

**Changes Required:**
1. Replace hardcoded `aplay` commands with HAL calls
2. Add USB device specification to all audio playback
3. Keep Festival text-to-speech integration unchanged

**Modified Functions:**
```c
void* firmwarePlayAudio(void* text) {
    // ... existing parsing code ...
    
    if(audio_type_byte == 'd') {
        // Use HAL for playback
        sprintf(buffer, "echo '%s' | text2wave -o /tmp/output.wav", remaining_string);
        system(buffer);
        hal_audio_play_file("/tmp/output.wav");
    } else if(audio_type_byte == 's') {
        // ... existing code with HAL playback ...
        hal_audio_play_file(filepath);
    } else if(audio_type_byte == 'p') {
        sprintf(buffer, "%s.wav", remaining_string);
        hal_audio_play_file(buffer);
    }
    // ... rest of function ...
}
```

### Phase 5: Software Layer (No Changes Required)

The Software layer should require **minimal to no changes** because:

1. **Packet Protocol Unchanged**: Communication between Software and Firmware uses the same packet format
2. **Key Mapping Handled in Firmware**: Software receives the same key symbols ('0'-'9', 'A'-'D', '*', '#')
3. **Audio Interface Unchanged**: Software sends the same audio packet format
4. **Hamlib Integration Preserved**: Radio control is independent of keypad/audio hardware

**Files That Should NOT Need Changes:**
- `Software/StateMachine.c`
- `Software/ModeRouting.c`
- `Software/Modes/*`
- `Software/HamlibWrappedFunctions/*`
- `Software/KeyWatching.c`

**Potential Minor Changes:**
- `Software/FirmwareCommunication.c`: May need timeout adjustments if USB keypad has different response characteristics
- Configuration files: Update any hardware-specific settings

### Phase 6: Build System Updates

#### 6.1 Update `Firmware/makefile`

**Changes:**
```makefile
# OLD
LIBS = -lwiringPi -lasound -lpthread

# NEW
LIBS = -lasound -lpthread
# WiringPi removed since we're using Linux input events

# Add HAL object files
HAL_OBJS = hal/hal_keypad_usb.o hal/hal_audio_usb.o

firmware.elf: $(HAL_OBJS) firmware.o audio_firmware.o keypad_firmware.o
    $(CC) -o $@ $^ $(LIBS)
```

#### 6.2 Update Installation Script

**File: `rpi_install.sh`** (new file, replacing `nanoPi_install.sh`)
```bash
#!/bin/bash

if ((EUID != 0)); then
    echo "Must be run as root"
    exit 1
fi

echo "Installing HAMPOD dependencies for Raspberry Pi"

# Update system
apt-get update
apt-get upgrade -y

# Audio dependencies
apt-get install -y libasound2 alsa-utils alsa-oss

# Development tools
apt-get install -y git build-essential

# Festival speech synthesis
apt-get install -y festival festival-dev

# Input device tools (for debugging)
apt-get install -y evtest input-utils

# USB audio configuration
echo "Configuring USB audio..."
# Add USB audio as default device
cat > /etc/asound.conf << EOF
pcm.!default {
    type hw
    card Device
}
ctl.!default {
    type hw
    card Device
}
EOF

echo "Installation complete!"
echo "Run 'scripts/find_usb_keypad.sh' to identify your keypad device"
echo "Run 'scripts/find_usb_audio.sh' to identify your audio device"
```

### Phase 7: Testing Strategy

#### 7.1 Unit Testing

**Test Files to Create:**
- `Firmware/tests/test_hal_keypad.c`: Test USB keypad reading
- `Firmware/tests/test_hal_audio.c`: Test USB audio playback
- `Firmware/tests/test_keypad_mapping.c`: Verify key mapping correctness

**Test Scenarios:**
1. Single key press detection
2. Multiple simultaneous key presses (should return invalid)
3. Key release detection
4. All 13 keys mapped correctly
5. USB device disconnection/reconnection handling

#### 7.2 Integration Testing

**Test Scenarios:**
1. Firmware packet communication (keypad read requests)
2. Audio playback (speech synthesis and file playback)
3. End-to-end: Key press → Software → Hamlib → Radio
4. Mode switching with new keypad layout
5. Long-running stability test

#### 7.3 Hardware Testing

**Required Tests:**
1. USB keypad enumeration on boot
2. USB audio device enumeration on boot
3. Audio quality verification
4. Keypad responsiveness (latency measurement)
5. Device hot-plug handling

### Phase 8: Documentation Updates

#### 8.1 Files to Update

1. **`README.md`**: Update hardware requirements
2. **`Firmware/README.md`**: Document new keypad mapping and HAL architecture
3. **`Documentation/Hardware_Setup.md`** (new): USB device setup guide
4. **`Documentation/Troubleshooting.md`** (new): Common USB device issues

#### 8.2 New Documentation

**Hardware Setup Guide:**
- USB keypad connection and identification
- USB audio device configuration
- ALSA configuration for USB audio
- Device permissions setup

**Key Mapping Reference:**
- Visual diagram of USB keypad
- Mapping table (USB key → HAMPOD function)
- Instructions for customizing mappings

### Phase 9: Migration Checklist

#### Pre-Migration
- [ ] Backup entire NanoPi codebase
- [ ] Document current NanoPi configuration
- [ ] Test Raspberry Pi hardware (keypad, audio, USB ports)
- [ ] Verify Raspberry Pi OS installation

#### Development
- [ ] Create HAL directory structure
- [ ] Implement `hal_keypad.h` interface
- [ ] Implement `hal_keypad_usb.c`
- [ ] Implement `hal_audio.h` interface
- [ ] Implement `hal_audio_usb.c`
- [ ] Create device detection scripts
- [ ] Update `keypad_firmware.c`
- [ ] Update `audio_firmware.c`
- [ ] Update makefiles
- [ ] Create `rpi_install.sh`

#### Testing
- [ ] Test USB keypad detection
- [ ] Test USB audio playback
- [ ] Test all 13 keys individually
- [ ] Test key mapping correctness
- [ ] Test Festival speech synthesis
- [ ] Test firmware packet communication
- [ ] Test Software layer (should work unchanged)
- [ ] Test Hamlib integration
- [ ] Test all operational modes

#### Deployment
- [ ] Install dependencies on Raspberry Pi
- [ ] Configure USB audio device
- [ ] Set up device permissions
- [ ] Build firmware
- [ ] Build software
- [ ] Run integration tests
- [ ] Update documentation
- [ ] Create deployment guide

### Phase 10: Extensibility Considerations

#### Adding New Radio Support (Hamlib)

The migration **preserves** the existing Hamlib architecture:

**No changes required to:**
- `Software/HamlibWrappedFunctions/HamlibGetFunctions.c`
- `Software/HamlibWrappedFunctions/HamlibSetFunctions.c`
- `Software/Radio.c`
- `Software/RigListCreator.c`

**To add a new radio:**
1. Follow existing procedure in `Software/README.md`
2. Add radio configuration to `ConfigSettings/`
3. Create new mode if needed in `Modes/`
4. No firmware changes required

#### Future Hardware Flexibility

The HAL design allows easy adaptation to:
- Different USB keypads (just update keymap table)
- Different audio devices (just update device string)
- Bluetooth keypads (implement new HAL backend)
- Network audio (implement new HAL backend)
- Multiple input devices simultaneously

## Risk Assessment

### High Risk
- **USB device enumeration**: Device paths may change between boots
  - *Mitigation*: Use `/dev/input/by-id/` for stable device paths
  
- **Key mapping confusion**: 13 buttons vs 16 buttons
  - *Mitigation*: Clear documentation, configurable mapping table

### Medium Risk
- **Audio latency**: USB audio may have higher latency than onboard
  - *Mitigation*: ALSA buffer tuning, test with real usage

- **Device permissions**: USB devices may require udev rules
  - *Mitigation*: Include udev rules in installation script

### Low Risk
- **Software compatibility**: Software layer should be unaffected
  - *Mitigation*: Comprehensive testing, packet protocol unchanged

## Timeline Estimate

- **Phase 1-2** (HAL + USB Keypad): 2-3 days
- **Phase 3** (USB Audio): 1 day
- **Phase 4** (Firmware Refactoring): 2 days
- **Phase 5** (Software Verification): 1 day
- **Phase 6** (Build System): 1 day
- **Phase 7** (Testing): 2-3 days
- **Phase 8** (Documentation): 1-2 days
- **Phase 9** (Deployment): 1 day

**Total Estimated Time**: 11-15 days

## Conclusion

This migration plan maintains the core HAMPOD architecture while adapting to Raspberry Pi hardware. The key strategy is:

1. **Isolate hardware dependencies** through HAL
2. **Preserve software layer** completely
3. **Maintain Hamlib integration** unchanged
4. **Keep packet protocol** identical
5. **Enable future extensibility** through abstraction

The result will be a Raspberry Pi-based HAMPOD that functions identically to the NanoPi version from the user's perspective, with improved maintainability and hardware flexibility.
