# Configuration Module Implementation Plan

> **Branch:** `feature/config-module`  
> **Parent Plan:** [Phase 0: Core Infrastructure](fresh-start-phase-zero-plan.md)

## Overview

Configuration storage for Software2 with:
- Load/save to INI file (auto-save on changes)
- 10-deep undo history
- Thread-safe getter/setter API
- Easy extension for future modes

---

## Settings

| Setting | Type | Default | Description |
|---------|------|---------|-------------|
| `radio_model` | int | 3073 (IC-7300) | Hamlib radio model ID |
| `radio_device` | string | `/dev/ttyUSB0` | Serial device path |
| `radio_baud` | int | 19200 | Baud rate |
| `output_volume` | int (0-100) | 80 | Audio volume % |
| `speech_speed` | float (0.5-2.0) | 1.0 | TTS speed |
| `key_beep_enabled` | bool | true | Key beep toggle |

---

## Design

### Undo System (10-deep circular buffer)

```c
#define CONFIG_UNDO_DEPTH 10

typedef struct {
    HampodConfig snapshots[CONFIG_UNDO_DEPTH];
    int head;      // Next write position
    int count;     // Number of valid snapshots
} ConfigHistory;

// Before each set operation:
// 1. Push current config to history
// 2. Apply change
// 3. Auto-save to file
```

### API

```c
// Core
int config_init(const char* path);
void config_cleanup(void);

// Undo
int config_undo(void);           // Restore previous state
int config_get_undo_count(void); // How many undos available

// Getters
int config_get_radio_model(void);
const char* config_get_radio_device(void);
int config_get_volume(void);
float config_get_speech_speed(void);

// Setters (auto-save after each)
void config_set_radio_model(int model);
void config_set_volume(int volume);
void config_set_speech_speed(float speed);
```

---

## Files

| File | Action | Purpose |
|------|--------|---------|
| `include/config.h` | NEW | API declarations |
| `src/config.c` | NEW | Implementation with undo |
| `config/hampod.conf` | NEW | Default config file |
| `tests/test_config.c` | NEW | Unit tests |
| `Makefile` | MODIFY | Add targets |

---

## Steps

### Step 1: Header with struct and API
Define `HampodConfig`, `ConfigHistory`, all function declarations.

### Step 2: Getters with hardcoded defaults
Implement `config_init()` setting defaults, all getters.

### Step 3: INI file parsing
Load settings from file in `config_init()`.

### Step 4: INI file writing
Implement `config_save()` to write INI format.

### Step 5: Undo history buffer
Add circular buffer for 10 config snapshots.

### Step 6: Setters with auto-save and undo
Each setter: push to history → update → save.

### Step 7: Undo function
Implement `config_undo()` to restore previous state.

### Step 8: Default config file
Create `config/hampod.conf` with documented defaults.

### Step 9: Unit tests
Test load, save, set, undo operations.

### Step 10: Integration with phase0_test
Use config in main_phase0.c.

### Step 11: Merge to main
Run regression tests, merge branch.

---

## Verification

```bash
# Unit tests
cd ~/HAMPOD2026/Software2
make test_config && ./bin/test_config

# Regression
./Documentation/scripts/Regression_Phase0_Integration.sh
```

---

## Checklist

- [x] Step 1: Header ✅ (2025-12-21)
- [x] Step 2: Getters ✅ (2025-12-21)
- [x] Step 3: INI parsing ✅ (2025-12-21)
- [x] Step 4: File writing ✅ (2025-12-21)
- [x] Step 5: Undo buffer ✅ (2025-12-21)
- [x] Step 6: Setters + auto-save ✅ (2025-12-21)
- [x] Step 7: Undo function ✅ (2025-12-21)
- [x] Step 8: Default config ✅ (2025-12-21)
- [x] Step 9: Unit tests ✅ (2025-12-21) - 8/8 passed
- [x] Step 10: Integration ✅ (2025-12-21) - Regression tests pass
- [x] Step 11: Merge ✅ (2025-12-21)
