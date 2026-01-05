# Startup Device Handling and Config Improvements

> **Goal**: Reliable audio/radio device selection at startup with multi-radio config support.

---

## Problem Summary

1. **Audio Device Selection**: USB card numbers change on reboot. Need deterministic selection:
   - Prefer "USB2.0 Device" external USB audio
   - Fall back to any external USB audio
   - Last resort: headphones

2. **Multi-Radio Config**: Support up to 10 radios in config, with only one active.

3. **Device Tracking**: Record physical USB port numbers for keypad/speaker to detect config changes.

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                      hampod.conf                            │
├─────────────────────────────────────────────────────────────┤
│ [audio]                                                     │
│ preferred_device = USB2.0 Device                            │
│ port = /sys/bus/usb/devices/1-2                              │
│ volume = 25                                                 │
│                                                             │
│ [keypad]                                                    │
│ port = /sys/bus/usb/devices/1-3                             │
│                                                             │
│ [radio.1]                                                   │
│ name = IC-7300                                              │
│ model = 3073                                                │
│ device = /dev/ttyUSB0                                       │
│ baud = 19200                                                │
│ port = /sys/bus/usb/devices/1-1                             │
│ enabled = true                                              │
│                                                             │
│ [radio.2]                                                   │
│ name = FT-991A                                              │
│ model = 1035                                                │
│ enabled = false                                             │
│ ...                                                         │
└─────────────────────────────────────────────────────────────┘
```

---

## Phase 1: Deterministic Audio Device Selection

### Chunk 1.1: USB Device Enumeration Utility

**File**: `Firmware/hal/hal_usb_util.c` (new), `Firmware/hal/hal_usb_util.h` (new)

**New Functions**:
```c
typedef struct {
    char card_name[64];      // e.g., "USB2.0 Device"
    int card_number;         // ALSA card number
    char usb_port[128];      // e.g., "/sys/bus/usb/devices/1-2"
    char device_path[64];    // e.g., "plughw:2,0"
} AudioDeviceInfo;

// Enumerate all audio devices
int hal_usb_enumerate_audio(AudioDeviceInfo* devices, int max_devices, int* found);

// Find device by name preference
int hal_usb_find_audio(const char* preferred_name, AudioDeviceInfo* result);

// Get USB port path for a device
int hal_usb_get_port_path(int card_number, char* port_path, size_t len);
```

**Implementation Details**:
1. Parse `/proc/asound/cards` for card names and numbers
2. For each USB card, read `/sys/class/sound/cardN/device` symlink to get USB port path
3. Priority order: "USB2.0 Device" > any USB > headphones > default

**Test**: `test_hal_usb_util.c`
- Enumerate devices on test system
- Verify port path resolution works

**Estimated Time**: 2-3 hours

---

### Chunk 1.2: Update Audio HAL to Use Device Selection

**File**: `Firmware/hal/hal_audio_usb.c`

**Changes**:
1. Replace `detect_usb_audio()` with `hal_usb_find_audio()` call
2. Store selected device info in static struct for reuse
3. Add `hal_audio_get_card_number()` for volume control
4. Ensure all audio paths (TTS, beeps, WAV) use same device

**New API**:
```c
// Get the selected card number (for volume control)
int hal_audio_get_card_number(void);

// Get the selected USB port path
const char* hal_audio_get_port_path(void);
```

**Test**: Verify audio plays through correct device on systems with multiple USB audio.

**Estimated Time**: 1-2 hours

---

### Chunk 1.3: Update Software2 Volume Control

**File**: `Software2/src/main.c`

**Changes**:
1. Add function to query Firmware for selected audio card
2. Use card number from Firmware response for amixer command
3. Remove hardcoded card number guessing

**Approach**: Add new packet type to query audio card from Firmware.

> [!IMPORTANT]
> The decision must be made once (in Firmware) and stored. Software2 queries the stored result—no duplicate logic that could diverge.

**New Packet Type**: `AUDIO_INFO` - returns selected card number, device path, port path.

**Test**: Volume setting applies to correct device regardless of card number.

**Estimated Time**: 1-2 hours

---

## Phase 2: Multi-Radio Configuration Support

### Chunk 2.1: Extend Config Parser for Radio Array

**File**: `Software2/src/config.c`, `Software2/include/config.h`

**Config Format**:
```ini
[radio.1]
enabled = true
name = IC-7300
model = 3073
device = /dev/ttyUSB0
baud = 19200
port = /sys/bus/usb/devices/1-1

[radio.2]
enabled = false
name = FT-991A
model = 1035
device = /dev/ttyUSB1
baud = 38400
port = /sys/bus/usb/devices/1-4
```

**New Types**:
```c
#define MAX_RADIOS 10

typedef struct {
    char name[32];
    int model;
    char device[64];
    int baud;
    char port[128];
    bool enabled;
} RadioConfig;

typedef struct {
    // existing fields...
    RadioConfig radios[MAX_RADIOS];
    int num_radios;
    int active_radio_index;  // -1 if none enabled
} HampodConfig;
```

**New API**:
```c
const RadioConfig* config_get_active_radio(void);
int config_get_radio_count(void);
const RadioConfig* config_get_radio(int index);
int config_set_radio_enabled(int index, bool enabled);
```

**Test**: Parse config with multiple radios, verify only one enabled.

**Estimated Time**: 2-3 hours

---

### Chunk 2.2: Update Radio Module to Use Config

**File**: `Software2/src/radio.c`

**Changes**:
1. Modify `radio_init()` to use `config_get_active_radio()`
2. Log which radio was selected
3. Handle case where no radio is enabled

**Test**: System starts with configured radio, logs selection.

**Estimated Time**: 1 hour

---

### Chunk 2.3: Add Keypad/Speaker Port Tracking

**File**: `Software2/include/config.h`, `Software2/src/config.c`

**Config Format**:
```ini
[keypad]
port =                  # Auto-populated: USB port path
device_name =           # Auto-populated: e.g., "Usb KeyBoard"
```

**New API**:
```c
const char* config_get_keypad_port(void);
const char* config_get_speaker_port(void);
int config_set_keypad_port(const char* port);
int config_set_speaker_port(const char* port);
```

**Test**: Ports are saved and loaded correctly.

**Estimated Time**: 1 hour

---

## Phase 3: Startup Configuration Validation

### Chunk 3.1: Detect Configuration Changes

**File**: `Software2/src/config.c`

**New Function**:
```c
typedef struct {
    bool keypad_port_changed;
    bool speaker_port_changed;
    bool active_radio_port_changed;
} ConfigChangeStatus;

int config_detect_changes(ConfigChangeStatus* status);
```

**Implementation**:
1. At startup, enumerate current USB devices
2. Compare stored values with actual connected devices:
   - Port paths (USB topology)
   - Device names ("USB2.0 Device", etc.)
   - Radio model numbers (if detectable)
3. Report any mismatches
4. Check for radios/devices that were present but are now missing

**Test**: Detect when USB device has moved to a different port or model changed.

**Estimated Time**: 1-2 hours

---

### Chunk 3.2: Announce Config Changes and Update Config

**File**: `Software2/src/main.c`

**Changes**:
1. After `config_init()`, call `config_detect_changes()`
2. If changes detected:
   - Speak a warning: "Configuration changed. Keypad on new port."
   - **Always update config with new port paths** (required for accurate future detection)
3. Call `config_save()` after any updates

> [!IMPORTANT]
> Config must be updated every time a change is detected. Otherwise, the previous state becomes unknown and future change detection is unreliable.

**Test**: System announces when devices have moved and saves new state.

**Estimated Time**: 1 hour

---

## Phase 4: Integration Testing

### Chunk 4.1: Create Integration Test Script

**File**: `Documentation/scripts/test_device_handling.sh`

**Tests**:
1. Audio plays through preferred device
2. Volume applies to correct card
3. Config with multiple radios loads correctly
4. Only enabled radio is used
5. Port change detection works

**Estimated Time**: 1-2 hours

---

### Chunk 4.2: Run Regression Suite

Run all existing regression tests:
```bash
./Documentation/scripts/Regression_Frequency_Mode.sh
./Documentation/scripts/Regression_Normal_Mode.sh
./Documentation/scripts/Regression_Phase0_Integration.sh
```

**Acceptance Criteria**: All tests pass.

---

## Verification Checklist

### Phase 1: Audio Device Selection
- [ ] USB device enumeration works on Pi 3/4/5
- [ ] "USB2.0 Device" is prioritized
- [ ] Fallback to any USB audio works
- [ ] All audio functions use same device
- [ ] Volume control uses correct card

### Phase 2: Multi-Radio Config
- [ ] Config parses up to 10 radios
- [ ] Only one radio can be enabled
- [ ] Radio module uses active radio
- [ ] Changing enabled radio in config switches device

### Phase 3: Configuration Validation
- [ ] Port paths are stored in config
- [ ] Port changes are detected at startup
- [ ] Warning is spoken on config change

### Phase 4: Integration
- [ ] All regression tests pass
- [ ] System works on Pi 3, 4, and 5

---

## Git Strategy

```bash
# Create feature branch
git checkout main
git pull origin main
git checkout -b startup-device-handling
```

### Commit Plan

| Chunk | Commit Message |
|-------|----------------|
| 1.1 | `hal: Add USB device enumeration utility` |
| 1.2 | `hal: Update audio HAL to use deterministic device selection` |
| 1.3 | `software2: Use detected card number for volume control` |
| 2.1 | `config: Add multi-radio configuration support` |
| 2.2 | `radio: Use config for radio device selection` |
| 2.3 | `config: Add keypad/speaker port tracking` |
| 3.1 | `config: Detect configuration changes at startup` |
| 3.2 | `main: Announce config changes at startup` |
| 4.1-4.2 | `test: Add device handling integration tests` |

### Merge Strategy

```bash
# After all tests pass
git checkout main
git merge startup-device-handling
git push origin main
git branch -d startup-device-handling
```

---

## Estimated Effort

| Phase | Chunks | Estimated Time |
|-------|--------|----------------|
| Phase 1: Audio Selection | 3 | 4-6 hours |
| Phase 2: Multi-Radio Config | 3 | 4-5 hours |
| Phase 3: Config Validation | 2 | 2-3 hours |
| Phase 4: Integration | 2 | 2-3 hours |
| **Total** | **10** | **12-17 hours** |

---

## Sample hampod.conf (New Format)

```ini
# HAMPOD Configuration
# Edit settings below and restart software

[audio]
preferred_device = USB2.0 Device
device_name =           # Auto-populated: actual device name detected
port =                  # Auto-populated: USB port path
card_number =           # Auto-populated: ALSA card number
volume = 25
speech_speed = 1.0
key_beep = 1

[keypad]
port =                  # Auto-populated: USB port path
device_name =           # Auto-populated: e.g., "Usb KeyBoard"

[radio.1]
enabled = true
name = IC-7300
model = 3073
device = /dev/ttyUSB0
baud = 19200
port =                  # Auto-populated: USB port path
detected_model =        # Auto-populated: model number from device query

[radio.2]
enabled = false
name = FT-991A  
model = 1035
device = /dev/ttyUSB1
baud = 38400
port =                  # Auto-populated: USB port path
detected_model =        # Auto-populated: model number from device query
```

> [!NOTE]
> **Auto-populated fields** (port, device_name, card_number, detected_model) are:
> - Set when a device is first detected and actively used
> - Updated whenever changes are detected
> - Used for change detection on subsequent startups
> 
> Unique identifiers like model number and device name help detect when a different device is on the same port.
