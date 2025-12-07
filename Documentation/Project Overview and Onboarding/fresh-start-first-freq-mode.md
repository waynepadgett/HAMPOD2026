# Fresh Start: Minimal Frequency Mode Implementation Plan

## Goal
Rewrite the Software layer from scratch while keeping the working Firmware/HAL. Focus on implementing a minimal **Frequency Mode** that:
- Reads keypad input from Firmware
- Speaks feedback to the blind user
- Uses Hamlib to set radio frequency
- Is easily adaptable for different radios (Kenwood, Elecraft, etc.)

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────┐
│                    NEW SOFTWARE LAYER                    │
├─────────────────────────────────────────────────────────┤
│  main.c              │  Entry point, initialization     │
│  speech.c/.h         │  Wrapper for Firmware audio API  │
│  keypad.c/.h         │  Reads keys from Firmware        │
│  radio.c/.h          │  Hamlib wrapper (radio-agnostic) │
│  frequency_mode.c/.h │  Frequency entry state machine   │
│  config.h            │  Radio model, device paths       │
└─────────────────────────────────────────────────────────┘
                          │
                          ▼
┌─────────────────────────────────────────────────────────┐
│              EXISTING FIRMWARE LAYER (KEEP)             │
├─────────────────────────────────────────────────────────┤
│  firmwarePlayAudio(text)  │  Speaks text via USB audio  │
│  hal_keypad_read()        │  Reads USB keypad events    │
└─────────────────────────────────────────────────────────┘
```

---

## How Firmware Integration Works (IMPORTANT)

The existing Firmware (`firmware.elf`) handles all USB hardware. Software communicates with it via **named pipes (FIFOs)**:

```
┌──────────────┐         Named Pipes          ┌──────────────────────────────┐
│  NEW         │  Keypad_i ──────────────────► │  EXISTING FIRMWARE           │
│  SOFTWARE    │  ◄────────────────── Keypad_o │  (firmware.elf - DO NOT TOUCH)│
│              │                               │                               │
│              │  Speaker_i ─────────────────► │  ┌─────────────────────────┐  │
│              │  ◄───────────────── Speaker_o │  │ USB Keypad HAL (working)│  │
│              │                               │  │ USB Audio HAL (working) │  │
│              │  Firmware_i ────────────────► │  └─────────────────────────┘  │
│              │  ◄──────────────── Firmware_o │                               │
└──────────────┘                               └──────────────────────────────┘
```

### Keypad Request/Response (via pipes)

To read a key, Software sends a **KEYPAD packet** on `Firmware_i`:
```c
// Packet structure (from hampod_firm_packet.h)
typedef struct Inst_packet {
    Packet_type type;        // KEYPAD = 0
    unsigned short data_len; // 1
    unsigned short tag;      // sequence number
    unsigned char *data;     // "r" for read request
} Inst_packet;
```

Firmware reads USB keypad (via HAL) and returns the key character on `Firmware_o`.

### Audio Request (via pipes)

To speak text, Software sends an **AUDIO packet** on `Firmware_i`:
```c
// data field format:
// "d<text>"     - dynamic TTS via Festival
// "p<filename>" - play pre-recorded .wav file
// "s<text>"     - generate and save TTS file
```

### Running Both Together

**Terminal 1:** Start Firmware first
```bash
cd ~/HAMPOD2026/Firmware && ./firmware.elf
```

**Terminal 2:** Start Software (connects to Firmware via pipes)
```bash
cd ~/HAMPOD2026/Software2 && ./software.elf
```

The Firmware handles ALL USB keypad reading and audio playback - your new Software just sends/receives packets.

---

## Implementation Phases

### Phase 1: Minimal Infrastructure (Test: Compile and run)

**Goal:** Create skeleton that compiles and starts

**Files to create:**
1. `Software2/Makefile` - Simple makefile linking to `-lhamlib`
2. `Software2/main.c` - Entry point
3. `Software2/config.h` - Configuration constants

**Test checkpoint:**
- [ ] `make` produces `software.elf`
- [ ] `./software.elf` runs and exits cleanly

---

### Phase 2: Speech Module (Test: Speak "hello")

**Goal:** Wrap Firmware pipe communication for speech

**Files to create:**
1. `Software2/speech.h` - API declarations
2. `Software2/speech.c` - Implementation (sends AUDIO packets to Firmware via pipe)

**speech.h API:**
```c
int speech_init(void);              // Open pipes to Firmware
void speech_say(const char* text);  // Send "d<text>" AUDIO packet
void speech_say_digit(char digit);  // Speak single digit
void speech_say_frequency(double freq_hz);  // "14 point 250 megahertz"
void speech_cleanup(void);          // Close pipes
```

**Implementation notes:**
- `speech_init()` opens `Firmware_i` pipe for writing
- `speech_say()` sends AUDIO packet with type=AUDIO(1), data="d<text>"
- Waits for response on `Firmware_o` before returning

**Test checkpoint:**
- [ ] Call `speech_say("hello")` - hear "hello" from speaker
- [ ] Call `speech_say_digit('5')` - hear "5"

---

### Phase 3: Keypad Module (Test: Press key, see output)

**Goal:** Wrap Firmware pipe communication for keypad

**Files to create:**
1. `Software2/keypad.h` - API declarations
2. `Software2/keypad.c` - Implementation (sends KEYPAD packets to Firmware via pipe)

**keypad.h API:**
```c
int keypad_init(void);        // Open pipes to Firmware
char keypad_read(void);       // Send KEYPAD packet, blocking wait for response
void keypad_cleanup(void);    // Close pipes
```

**Implementation notes:**
- `keypad_init()` opens `Firmware_i` for writing, `Firmware_o` for reading
- `keypad_read()` sends KEYPAD packet with data="r", then waits for key char response
- Returns the key character ('0'-'9', '#', '*', etc.)

---

### Phase 4: Keypad + Speech Integration (Test: Press key, hear it)

**Goal:** Combine keypad and speech

**Test checkpoint:**
- [ ] Press '7' → hear "7" spoken
- [ ] Press '#' → hear "hash" spoken
- [ ] Press '*' → hear "star" spoken

---

### Phase 5: Radio Module - Connection Only (Test: Connect to radio)

**Goal:** Establish Hamlib connection to radio

**Files to create:**
1. `Software2/radio.h` - API declarations
2. `Software2/radio.c` - Implementation

**radio.h API:**
```c
// Radio-agnostic interface
int radio_init(int model_id, const char* device_path);
double radio_get_frequency(void);
int radio_set_frequency(double freq_hz);
void radio_cleanup(void);
```

**config.h radio constants:**
```c
// Change these for different radios:
#define RADIO_MODEL     RIG_MODEL_IC7300   // or RIG_MODEL_K3, etc.
#define RADIO_DEVICE    "/dev/ttyUSB0"
#define RADIO_BAUD      19200
```

**Test checkpoint:**
- [ ] Program connects to radio (or prints error if not connected)
- [ ] `radio_get_frequency()` returns current frequency

---

### Phase 6: Frequency Mode State Machine (Test: Enter frequency)

**Goal:** Implement frequency entry logic

**Files to create:**
1. `Software2/frequency_mode.h` - API declarations
2. `Software2/frequency_mode.c` - State machine

**State machine:**
```
┌─────────────┐
│   IDLE      │◄─────────────────────────────┐
│ (waiting)   │                              │
└──────┬──────┘                              │
       │ digit pressed                       │
       ▼                                     │
┌─────────────┐                              │
│  ENTERING   │ ←─ more digits              │
│  DIGITS     │                              │
└──────┬──────┘                              │
       │ '#' pressed                  '*' pressed (cancel)
       ▼                                     │
┌─────────────┐                              │
│   SUBMIT    │──── set frequency ──────────►│
│   (apply)   │     speak confirmation       │
└─────────────┘
```

**frequency_mode.h API:**
```c
void frequency_mode_init(void);
void frequency_mode_handle_key(char key);  // Called for each keypress
```

**Behavior:**
- Digit (0-9): Accumulate, speak the digit
- '*' (star): Toggle decimal mode OR cancel if already in decimal
- '#' (hash): Submit frequency to radio, speak confirmation

**Test checkpoint:**
- [ ] Type "14*250#" → radio set to 14.250 MHz, hear "frequency set 14 point 250 megahertz"
- [ ] Type "7*125#" → radio set to 7.125 MHz
- [ ] Type "145*52#" → radio set to 145.520 MHz

---

### Phase 7: Main Loop Integration (Test: Full workflow)

**Goal:** Complete main loop

**main.c structure:**
```c
int main() {
    speech_init();
    keypad_init();
    radio_init(RADIO_MODEL, RADIO_DEVICE);
    frequency_mode_init();
    
    speech_say("frequency mode ready");
    
    while (1) {
        char key = keypad_read();
        frequency_mode_handle_key(key);
    }
    
    // Cleanup on exit
    radio_cleanup();
    keypad_cleanup();
    speech_cleanup();
    return 0;
}
```

**Test checkpoint:**
- [ ] Startup → hear "frequency mode ready"
- [ ] Full frequency entry workflow works
- [ ] Ctrl+C exits cleanly

---

## File Structure Summary

```
Software2/
├── Makefile           # Build configuration
├── config.h           # Radio model, device paths (edit for different radios)
├── main.c             # Entry point
├── speech.h/.c        # Text-to-speech wrapper
├── keypad.h/.c        # Keypad input wrapper
├── radio.h/.c         # Hamlib wrapper (radio-agnostic)
└── frequency_mode.h/.c # Frequency entry state machine
```

---

## Adapting for Different Radios

To use a different radio, only ONE file needs to change: `config.h`

**Icom IC-7300:**
```c
#define RADIO_MODEL   RIG_MODEL_IC7300
#define RADIO_DEVICE  "/dev/ttyUSB0"
#define RADIO_BAUD    19200
```

**Kenwood TS-590:**
```c
#define RADIO_MODEL   RIG_MODEL_TS590S
#define RADIO_DEVICE  "/dev/ttyUSB0"
#define RADIO_BAUD    57600
```

**Elecraft K3:**
```c
#define RADIO_MODEL   RIG_MODEL_K3
#define RADIO_DEVICE  "/dev/ttyUSB0"
#define RADIO_BAUD    38400
```

Find your radio's model ID: `rigctl -l | grep -i "your radio"`

---

## Testing Strategy

Each phase has a **concrete test checkpoint** before moving to the next:

| Phase | Test | Pass Criteria |
|-------|------|---------------|
| 1 | Compile | `make` succeeds |
| 2 | Speech | Speak "hello" audibly |
| 3 | Keypad | Print key to console |
| 4 | Key+Speech | Press key, hear it |
| 5 | Radio | Read current frequency |
| 6 | Freq Mode | Enter "14*250#", radio changes |
| 7 | Full | Complete startup-to-entry flow |

---

## Dependencies

**Required on Raspberry Pi:**
```bash
sudo apt install libhamlib-dev libasound2-dev festival
```

**Makefile linking:**
```makefile
LDFLAGS = -lhamlib -lasound -lpthread
```

---

## Estimated Time

| Phase | Estimated Time |
|-------|----------------|
| Phase 1 | 30 min |
| Phase 2 | 1 hour |
| Phase 3 | 1 hour |
| Phase 4 | 30 min |
| Phase 5 | 1 hour |
| Phase 6 | 2 hours |
| Phase 7 | 30 min |
| **Total** | **~6-7 hours** |

---

## Future Expansion

After frequency mode works, the same pattern can add:
- **Mode selection** (USB, LSB, CW, etc.)
- **VFO switching** (A/B)
- **Memory presets** (store/recall frequencies)
- **Signal strength** (read S-meter, speak it)

Each would be a new `*_mode.c` file following the same state machine pattern.
