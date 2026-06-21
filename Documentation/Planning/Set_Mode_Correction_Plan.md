> **Status:** 🟡 Partially Complete — most corrections implemented, 2 items remain
> **Last Updated:** 2026-06-21

# Set Mode Correction Plan

> **Goal**: Correct current system behavior to match [ICOMReaderManual2.md](../Original_Hampod_Docs/ICOMReaderManual2.md) specifications for features already implemented. No new features.

---

## Executive Summary

Most behavioral differences between the current implementation and the specification have been corrected. The remaining work is tracked below.

### Verified Status

| Area | Spec Says | Current Behavior | Status |
|------|-----------|------------------|--------|
| Key beep on press | Short beep when key pressed (configurable) | Implemented (`Software2/src/keypad.c:84` sends IPC beep) | ✅ Done |
| Hold indicator beep | Lower-pitch beep at 500ms | Implemented (`Software2/src/keypad.c:81` sends IPC beep) | ✅ Done |
| Error beep | Low-frequency beep on invalid key | Implemented (`set_mode.c`, `frequency_mode.c`, `config_mode.c` via `comm_play_beep`) | ✅ Done |
| Set Mode announcement | Says "Set" | Says "Set" (`set_mode.c:454` — matches spec) | ✅ Done |
| [*] in Set Mode | Cancel/exit Set Mode | Calls `set_mode_exit()` (`set_mode.c:652` — matches spec) | ✅ Done |
| Frequency announcement | "dot" separator for sub-kHz digits | Always uses "point" | ❌ Needs fix |

---

## Phase 1: Audio Feedback System ✅ COMPLETE

Beep audio files, HAL API, and Software-side beep triggering are all implemented. Beeps are triggered from `Software2/src/keypad.c` (via IPC → Firmware beep bypass) rather than directly in `Firmware/keypad_firmware.c`, but the user experience is identical.

### Step 1.1: Create Beep Audio Files ✅

**Files exist** in `Firmware/pregen_audio/`:

| File | Description | Size |
|------|-------------|------|
| `beep_keypress.wav` | Short beep on key press (50ms, 1000Hz) | 1.6 KB |
| `beep_hold.wav` | Lower-pitch hold indicator (50ms, 700Hz) | 1.6 KB |
| `beep_error.wav` | Low-frequency error beep (100ms, 400Hz) | 3.2 KB |

`Firmware/pregen_audio/generate_beeps.sh` contains the `sox` commands to regenerate if needed.

### Step 1.2: Add Beep API to Audio Firmware ✅

- `BeepType` enum defined in `Firmware/hal/hal_audio.h` (lines 131-134)
- `hal_audio_play_beep(BeepType)` declared in `Firmware/hal/hal_audio.h` (line 145)
- `audio_play_beep(BeepType)` wrapper in `Firmware/audio_firmware.c` (line 435)
- HAL implementation in `Firmware/hal/hal_audio_usb.c` with WAV paths defined
- Has unit tests in `Firmware/hal/tests/test_hal_audio.c` and `test_tts_cache.c`

### Step 1.3: Wire Beeps to Keypad Handling ✅ (via Software IPC)

Beeps are triggered from `Software2/src/keypad.c` (lines 78-86), which sends beep requests to Firmware via IPC:
- **Key press** → `comm_play_beep(COMM_BEEP_KEYPRESS)` on every key event
- **Key hold** → `comm_play_beep(COMM_BEEP_HOLD)` when a key is held
- Both guarded by `config_get_key_beep_enabled()`

Firmware receives beep requests in its audio processing loop via a **beep bypass** mechanism (`audio_firmware.c` lines 275-302) that plays them immediately without queuing.

**Note:** Beeps go through the IPC round-trip (Software → Firmware) rather than being handled directly in `Firmware/keypad_firmware.c`. This adds a small latency. If zero-lag beeps are needed, a future enhancement could wire beeps into `keypad_process()` directly.

### Step 1.4: Firmware Integration Test ✅

HAL-level beep tests exist in `Firmware/hal/tests/test_hal_audio.c` (`test_audio_play_beep` function). The manual test procedure below can be used for RPi hardware validation.

**Manual test**:
```bash
# RPI: Build and run full system
cd Documentation/scripts && sudo ./Regression_Phase_Three_Manual_Test.sh
```

**Verification checklist**:
- [x] Key press beep plays on key down (via keypad.c IPC)
- [x] Hold beep plays at 500ms mark (via keypad.c IPC)
- [x] Beeps work when speech is playing (ALSA dmix)
- [x] Beep volume is appropriate
- [x] `config key_beep = 0` disables all beeps

---

## Phase 2: Key Beep Integration ✅ COMPLETE

All software-level beep integration is implemented.

### Step 2.1: Add Error Beep Request to Comm Protocol ✅

- `CommBeepType` enum and `comm_play_beep()` declared in `Software2/include/comm.h` (lines 158-175)
- `comm_play_beep()` implemented in `Software2/src/comm.c` (line 569) — sends beep request packet to Firmware IPC

### Step 2.2: Fire Error Beeps on Invalid Keys ✅

Error beeps are wired in all mode handlers:

| File | Usage | Count |
|------|-------|-------|
| `Software2/src/set_mode.c` | Error beep on failed set operations, invalid keys | 9 calls |
| `Software2/src/frequency_mode.c` | Error beep on invalid key sequences | 3 calls |
| `Software2/src/config_mode.c` | Error beep on invalid operations | 3 calls |
| `Software2/src/keypad.c` | Key press and hold beeps (guarded by config) | 2 calls |

All calls are guarded by `config_get_key_beep_enabled()`.

### Step 2.3: Test Key Beep Enable/Disable

**Manual test**: Modify `hampod.conf`:
```ini
key_beep = 0
```
Verify no beeps play. Change to `key_beep = 1`, verify beeps resume.

---

## Phase 3: Set Mode Behavior Corrections (Software)

### Step 3.1: Verify [*] Key Behavior ✅

**Already fixed.** The current implementation at `Software2/src/set_mode.c:652` calls `set_mode_exit()` directly:

```c
// [*] - Cancel and exit Set Mode (per spec: not just clear buffer)
if (key == '*' && !is_hold) {
    set_mode_exit();
    return true;
}
```

The comment confirms this was intentionally aligned with the spec. No changes needed.

**Note**: The current implementation exits Set Mode immediately rather than first clearing the value buffer. If a more nuanced behavior is desired (clear digits first when actively editing, then exit from idle), this can be revisited. For now, the behavior matches the spec.

### Step 3.2: Verify [B] Cycling Behavior

**Current**: `[B]` toggles between Set Mode and Off only.

**Spec**: `[B]` should cycle: Set → Band → Off (if band stacking available), or Set → Off otherwise.

For now (no band stacking implemented), current behavior is acceptable. Document this as intentional limitation.

### Step 3.3: Unit Test for Set Mode Keys

**File**: `Software2/tests/test_set_mode.c` (new file)

Create focused tests for:
- `[B]` enters Set Mode
- `[B]` again exits Set Mode (from idle)
- `[*]` cancels/exits in various states
- `[D]` cancels edit
- `[#]` confirms value

**Run tests**:
```bash
# DEV or RPI: Build and run
cd Software2 && make tests && ./bin/test_set_mode
```

---

## Phase 4: Frequency Announcement Format (Software)

### Step 4.1: Fix "dot" vs "point" Separator

**Spec**: 
- Primary decimal (MHz) = "point"
- Sub-kHz separator = "dot"

**Current**: Always uses "point"

**File**: `Software2/src/normal_mode.c` - `announce_frequency()` function
**File**: `Software2/src/frequency_mode.c` - `announce_frequency()` function

Modify announcement logic:
1. Speak digits down to 1 kHz with "point"
2. If sub-kHz digits are non-zero, add "dot" + remaining digits
3. If all sub-kHz zeros, omit them

**Example**:
- 14.250000 → "fourteen point two five zero"
- 14.250500 → "fourteen point two five zero dot five"

### Step 4.2: Verify Special Frequencies

**Test**: Enter frequency "777" and verify firmware version is announced.

**Test**: Enter frequency "999" and verify factory reset occurs (use with caution).

---

## Phase 5: Testing and Documentation

### Step 5.1: Update Existing Tests

Review and update if needed:
- `test_frequency_mode.c` - Verify frequency format
- `test_keypad_events.c` - Note about beep behavior

### Step 5.2: Integration Test Script

Create `Documentation/scripts/test_set_mode_integration.sh`:
```bash
#!/bin/bash
# RPI: Run from home directory
# Prerequisites: Firmware running, radio connected

echo "=== Set Mode Integration Test ==="
echo "1. Press [B] - Should hear 'Set'"
echo "2. Press [9] Hold - Should hear power level"
echo "3. Enter '50' then [#] - Should set power to 50%"
echo "4. Press [B] - Should hear 'Set Off'"
echo ""
echo "Press Ctrl+C when done"

cd ~/HAMPOD2026/Software2
./bin/hampod
```

### Step 5.3: Update Documentation

**File**: `Documentation/Reference/Currently_Implemented_Keys.md`

Review and update to reflect:
- Key beep behavior (already implemented — verify documented)
- [*] exits Set Mode (already fixed — verify documented)
- Any other changes since last review

---

## Git Strategy

### Branch Structure
```
main
  └── feature/set-mode (merged — most corrections already applied)
```

### Commit History (Completed)

1. **Firmware commits** (Phase 1) ✅:
   - `firmware: Add beep audio files` — beep .wav files in `Firmware/pregen_audio/`
   - `firmware: Add beep playback API` — `hal_audio_play_beep()`, `audio_play_beep()`
   - `firmware: Wire key press and hold beeps` — via IPC beep bypass in `audio_firmware.c`

2. **Software commits** (Phases 2-4) ✅:
   - `software: Add comm_play_beep for error beeps` — `CommBeepType` enum + `comm_play_beep()`
   - `software: Fix [*] to exit Set Mode per spec` — `set_mode_exit()` call at `set_mode.c:652`
   - `software: Wire beeps across mode handlers` — 15+ `comm_play_beep()` calls across all modes

3. **Remaining to implement**:
   - `software: Fix frequency announcement format` — "dot" vs "point" (Phase 4)
   - `software: Add test_set_mode unit tests` — missing unit test file (Phase 3.3)
   - `docs: Update Currently_Implemented_Keys.md` — verify beep/[*/] behavior documented
   - `docs: Add Set Mode integration test script` — `test_set_mode_integration.sh`

### Merge Strategy

Most corrections are already committed. Remaining items can be committed individually to the current branch.

---

## Verification Checklist

### Firmware (Phase 1) ✅ Complete
- [x] Beep files created and play correctly (`Firmware/pregen_audio/`)
- [x] Beep API added to audio_firmware (`hal_audio_play_beep`, `audio_play_beep`)
- [x] Key press beep fires on key down (via keypad.c → IPC)
- [x] Hold beep fires at 500ms (via keypad.c → IPC)
- [x] Beeps mix correctly with speech (ALSA dmix, beep bypass mechanism)
- [x] `config key_beep = 0` disables all beeps

### Software (Phases 2-4) 🟡 Partial
- [x] `comm_play_beep()` API implemented and wired across all modes
- [x] Error beep plays on invalid key in Frequency Mode
- [x] Error beep plays on failed operations in Set Mode
- [x] Error beep plays on invalid operations in Config Mode
- [x] [*] exits Set Mode (calls `set_mode_exit()`)
- [ ] Frequency announcement uses "dot" for sub-kHz (still uses "point")
- [ ] `test_set_mode.c` unit tests created and passing

### Integration (Phase 5) 🟡 Partial
- [x] Full workflow: Normal → Set → adjust power → exit
- [ ] Full workflow: Normal → Frequency → enter freq → confirm (with "dot" format)
- [ ] `test_set_mode_integration.sh` script created
- [ ] `Currently_Implemented_Keys.md` updated and verified

---

## Dependencies and Prerequisites

| Dependency | Location | Status |
|------------|----------|--------|
| `sox` for audio generation | RPi system package | Added to RPi_Setup_Guide.md |
| ALSA dmix configuration | `~/.asoundrc` | Added to RPi_Setup_Guide.md |
| Firmware running | RPi | Required for all tests |
| Radio connected | RPi USB | Required for integration |

---

## Risk Mitigation

| Risk | Mitigation |
|------|------------|
| Beep audio quality poor | Test on actual hardware early; adjust frequency/duration |
| ALSA dmix not working | Fallback: interrupt speech for beeps (acceptable lag) |
| Firmware changes break existing functionality | Run regression tests after each Firmware change |
| Radio communication affected | Test radio queries after Firmware changes |

---

## Estimated Effort (Remaining)

| Phase | Status | Remaining Effort |
|-------|--------|------------------|
| Phase 1: Firmware beeps | ✅ Complete | 0 |
| Phase 2: Software integration | ✅ Complete | 0 |
| Phase 3: Set Mode fixes | 🟡 Mostly done | ~15 min (test_set_mode.c) |
| Phase 4: Frequency format | ❌ Not started | ~1 hour |
| Phase 5: Testing & docs | 🟡 Partial | ~1 hour |
| **Total remaining** | — | **~2 hours** |
