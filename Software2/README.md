# Software2 - HAMPOD Fresh Start Implementation

This is the new Software layer for HAMPOD2026, built with clean architecture following the "Fresh Start" approach.

## Status: Phase 0 (Core Infrastructure)

See [Phase Zero Plan](../Documentation/Project_Overview_and_Onboarding/fresh-start-phase-zero-plan.md) for details.

## Directory Structure

```
Software2/
├── include/                    # Public headers
│   ├── hampod_core.h           # Core types and constants
│   ├── comm.h                  # Pipe communication API
│   ├── config.h                # Configuration management
│   ├── frequency_mode.h        # Frequency entry mode
│   ├── keypad.h                # Keypad event handling
│   ├── normal_mode.h           # Normal operating mode
│   ├── radio.h                 # Radio control (Hamlib)
│   ├── radio_queries.h         # Radio query functions
│   ├── radio_setters.h         # Radio setter functions
│   ├── set_mode.h              # Set mode (parameter adjustment)
│   └── speech.h                # Speech queue
├── src/                        # Source files
│   ├── main.c                  # Main entry point
│   ├── comm.c                  # Pipe communication + router thread
│   ├── config.c                # Load/save INI config, undo support
│   ├── frequency_mode.c        # Frequency entry state machine
│   ├── keypad.c                # Keypad polling + hold detection
│   ├── normal_mode.c           # Normal mode key dispatch
│   ├── radio.c                 # Hamlib radio connection
│   ├── radio_queries.c         # Read radio state (VFO, AGC, etc.)
│   ├── radio_setters.c         # Set radio parameters
│   ├── set_mode.c              # Set mode parameter adjustment
│   └── speech.c                # Non-blocking speech queue
├── tests/                      # Test programs
│   ├── test_compile.c          # Build smoke test
│   ├── test_comm_queue.c       # Unit: response queue logic
│   ├── test_config.c           # Unit: config load/save/undo
│   ├── test_frequency_mode.c   # Unit: frequency mode (mock-based)
│   ├── test_radio.c            # Radio: Hamlib connection (needs radio)
│   └── deprecated/             # Old integration tests (pipe deadlock)
│       ├── test_comm_read.c
│       ├── test_comm_write.c
│       ├── test_keypad_events.c
│       └── test_speech_queue.c
├── config/                     # Configuration files
│   └── hampod.conf             # Runtime configuration
├── bin/                        # Output binaries (auto-created)
├── obj/                        # Object files (auto-created)
├── Makefile                    # Build system
└── README.md                   # This file
```

## Building

```bash
# Build main executable
make

# Build tests
make tests

# Clean build artifacts
make clean
```

## Modules

| Module | File | Status | Description |
|--------|------|--------|-------------|
| Core | `hampod_core.h` | ✅ Done | Types, constants, debug macros |
| Comm | `comm.c` | ✅ Done | Pipe communication with Firmware, router thread |
| Speech | `speech.c` | ✅ Done | Non-blocking speech queue |
| Keypad | `keypad.c` | ✅ Done | Key event handling with hold detection |
| Config | `config.c` | ✅ Done | INI config load/save, 10-deep undo |
| Radio | `radio.c` | ✅ Done | Hamlib radio connection and polling |
| Radio Queries | `radio_queries.c` | ✅ Done | VFO, AGC, preamp, attenuation queries |
| Radio Setters | `radio_setters.c` | ✅ Done | Power, mic gain, NB, NR, etc. |
| Normal Mode | `normal_mode.c` | ✅ Done | Normal mode key dispatch |
| Frequency Mode | `frequency_mode.c` | ✅ Done | Frequency entry state machine |
| Set Mode | `set_mode.c` | ✅ Done | Parameter adjustment mode |

## Communication with Firmware

Software2 communicates with the Firmware via named pipes:

| Pipe | Direction | Purpose |
|------|-----------|---------|
| `Keypad_o` | Firmware → Software | Key press events |
| `Firmware_i` | Software → Firmware | Audio requests |
| `Firmware_o` | Firmware → Software | Status messages |

### Audio Packet Format

| Type | Example | Description |
|------|---------|-------------|
| `d` | `dHello World` | Speak text via TTS (Festival/Piper) |
| `p` | `p/path/file.wav` | Play WAV file |
| `s` | `sABC123` | Spell out characters |

## Dependencies

- GCC with pthread support
- ALSA libraries (on Pi)
- Firmware must be running first

## Testing

Use the test runner script or run tests individually:

```bash
make tests                             # Build all tests
./run_all_unit_tests.sh                # Run all 5 tests (prompts for radio)
./run_all_unit_tests.sh --unit-only    # Just 4 unit tests (no hardware)
./run_all_unit_tests.sh --all          # Run everything automatically
```

### Active Tests

| Test | Type | Dependencies |
|------|------|--------------|
| `test_compile` | Smoke test | None |
| `test_comm_queue` | Unit test | None |
| `test_config` | Unit test | None |
| `test_frequency_mode` | Unit test (mock-based) | None |
| `test_radio` | Hardware test | Radio connected via USB |

### Deprecated Tests (tests/deprecated/)

The following integration tests were written during Phase 0 and no longer
connect to the current Firmware due to pipe deadlock issues. Their
functionality is covered by the regression scripts and manual SOP.

- `test_comm_read`, `test_comm_write`, `test_keypad_events`, `test_speech_queue`

### Makefile Notes

`test_frequency_mode` uses a **mock-based** approach — it defines its own stub
implementations of `speech_say_text()`, `radio_init()`, `config_init()`, etc.
The Makefile has a dedicated rule that links it with only `frequency_mode.o`
to avoid "multiple definition" linker errors with the real implementations.

## Running on Raspberry Pi

1. Ensure Firmware is running: `cd ../Firmware && ./firmware.elf`
2. Build: `make`
3. Run: `./bin/hampod`
