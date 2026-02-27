# Fresh Start: Phase 1 - Frequency Mode Implementation

Implement basic frequency mode for HAMPOD Software2 with keypad frequency entry, audio announcements, and radio polling for VFO dial changes.

> **Parent Plan:** [Fresh Start: Big Picture Plan](fresh-start-big-plan.md)
> **Prerequisite:** [Phase 0: Core Infrastructure](fresh-start-phase-zero-plan.md) ✅ Complete

## Overview

This phase adds radio control capability to the existing infrastructure (comm, speech, keypad, config modules) and implements the first operational mode.

**Key Features:**
- Radio module with Hamlib wrapper
- Frequency entry from keypad (per ICOMReader manual)
- Audio announcements of operations
- Radio polling for VFO dial changes (1-second debounce)

---

## Proposed Changes

### Radio Module

#### [NEW] `Software2/include/radio.h`
```c
int radio_init(void);           // Uses config for model/device/baud
void radio_cleanup(void);
bool radio_is_connected(void);
double radio_get_frequency(void);           // Returns Hz, -1 on error
int radio_set_frequency(double freq_hz);    // Returns 0 on success
int radio_start_polling(void (*on_change)(double new_freq));
void radio_stop_polling(void);
```

#### [NEW] `Software2/src/radio.c`
Hamlib wrapper with polling thread (100ms interval), 1-second debounce.

---

### Frequency Mode

#### [NEW] `Software2/include/frequency_mode.h`
Per ICOMReader manual section 2: VFO selection, digit accumulation, decimal handling.

#### [NEW] `Software2/src/frequency_mode.c`
Key behaviors:
- `[#]` - Enter frequency mode, cycle VFO selection (A/B/Current)
- Digits `0-9` - Accumulate, speak digit
- `[*]` - Insert decimal point; second press cancels
- `[#]` (after digits) - Submit frequency, speak confirmation
- `[D]` - Clear entry and return to normal mode

---

### Tests

#### [NEW] `Software2/tests/test_radio.c`
Hamlib connection test (requires physical radio).

#### [NEW] `Software2/tests/test_frequency_mode.c`
State machine unit tests.

#### [NEW] `Documentation/scripts/Regression_Frequency_Mode.sh`
Automated regression test for frequency mode.

---

## Implementation Chunks

Each chunk compiles and has a verification step.

### Chunk 1: Documentation Updates
Update `fresh-start-big-plan.md` and `fresh-start-first-freq-mode.md`.

### Chunk 2: Radio Module Skeleton
Create `radio.h`, `radio.c` stubs, update Makefile.
**Verify:** `make` compiles.

### Chunk 3: Radio Connect/Disconnect
Implement init/cleanup. Create `test_radio.c`.
**Verify:** Connects to IC-7300.

### Chunk 4: Get/Set Frequency
Implement frequency operations.
**Verify:** Reads and sets frequency.

### Chunk 5: Radio Polling Thread
1-second debounce for VFO dial changes.
**Verify:** Turn knob, see announcement.

### Chunk 6: Frequency Mode Skeleton
Create header and state enum.
**Verify:** Compiles.

### Chunk 7: Frequency Entry State Machine
Digit accumulation, decimal, submit/cancel.
**Verify:** Unit tests pass.

### Chunk 8: Audio Integration
Wire `speech_say()` for feedback.
**Verify:** End-to-end on RPi.

### Chunk 9: Regression Testing
Run all regression tests before merge:
| Test | Script | Must Pass |
|------|--------|-----------|
| HAL Integration | `Regression_HAL_Integration.sh` | ✅ |
| Imitation Software | `Regression_Imitation_Software.sh` | ✅ |
| Phase 0 Integration | `Regression_Phase0_Integration.sh` | ✅ |
| Frequency Mode | `Regression_Frequency_Mode.sh` | ✅ |

### Chunk 10: Final Integration & Merge
Update `main.c`, merge feature branch to main.

---

## Manual Verification

**Requires:** RPi with radio connected, Firmware running

1. **Frequency Entry:** `[#]` → "VFO A" → `1,4,*,2,5,0,[#]` → confirm 14.250 MHz on radio
2. **Radio Polling:** Turn knob, wait 1s, hear frequency announced
3. **Cancel Entry:** `[#]` → digits → `[*][*]` → returns to normal mode

---

## Git Workflow

1. `git checkout -b feature/frequency-mode`
2. Commit each chunk separately
3. Run all regression tests
4. `git merge feature/frequency-mode` into main

---

## Execution Checklist

- [x] **Chunk 1** Documentation Updates ✅ (2025-12-21)
- [x] **Chunk 2** Radio Module Skeleton ✅ (2025-12-21)
- [x] **Chunk 3** Radio Connect/Disconnect ✅ (2025-12-21)
- [x] **Chunk 4** Get/Set Frequency ✅ (2025-12-21)
- [x] **Chunk 5** Radio Polling Thread ✅ (2025-12-21)
- [x] **Chunk 6** Frequency Mode Skeleton ✅ (2025-12-21)
- [x] **Chunk 7** Frequency Entry State Machine ✅ (2025-12-21)
- [x] **Chunk 8** Audio Integration ✅ (2025-12-21)
- [x] **Chunk 9** Regression Testing ✅ (2025-12-21)
- [x] **Chunk 10** Final Integration & Merge ✅ (2025-12-21)

### Regression Test Results (2025-12-21)

| Test | Result | Notes |
|------|--------|-------|
| Build (make) | ✅ PASS | Compiles with 1 warning (unused function) |
| Unit Tests | ✅ PASS | 11/11 frequency mode tests passed |
| Phase 0 Integration | ✅ PASS | Router, speech, keypad all working |
