# Set Mode Correction Plan

> **Goal**: Correct current system behavior to match [ICOMReaderManual2.md](../Original_Hampod_Docs/ICOMReaderManual2.md) specifications for features already implemented. No new features.

---

## Executive Summary

Manual testing revealed behavioral differences between the current implementation and the specification. This plan addresses corrections in priority order, with Firmware and Software changes kept separate.

### Key Discrepancies Found

| Area | Spec Says | Current Behavior | Priority |
|------|-----------|------------------|----------|
| Key beep on press | Short beep when key pressed (configurable) | No beep implemented | High |
| Hold indicator beep | Lower-pitch beep at 500ms | No beep implemented | High |
| Error beep | Low-frequency beep on invalid key | No beep implemented | High |
| Set Mode announcement | Says "Set" | Says "Set Mode" | High |
| [*] in Set Mode | Cancel/exit Set Mode | Clears value buffer only | Medium |
| Frequency announcement | "dot" separator for sub-kHz digits | Always uses "point" | Medium |

---

## Phase 1: Audio Feedback System (Firmware)

Beeps require very low latency. Per [fresh-start-big-plan.md](./fresh-start-big-plan.md#problem-2-key-beeps-with-minimal-lag), the recommended approach is to handle beeps in Firmware for zero-lag response.

### Step 1.1: Create Beep Audio Files

**Files to create** in `Firmware/pregen_audio/`:

| File | Description | Spec |
|------|-------------|------|
| `beep_keypress.wav` | Short beep on key press | 50ms, 1000Hz, medium volume |
| `beep_hold.wav` | Lower-pitch hold indicator | 50ms, 700Hz, medium volume |
| `beep_error.wav` | Low-frequency error beep | 100ms, 400Hz, medium volume |

**How to generate** (on dev machine or RPi):
```bash
cd Firmware/pregen_audio

# Key press beep: 50ms, 1000Hz sine wave
sox -n beep_keypress.wav synth 0.05 sine 1000 vol 0.5

# Hold indicator: 50ms, 700Hz (lower pitch)  
sox -n beep_hold.wav synth 0.05 sine 700 vol 0.5

# Error beep: 100ms, 400Hz (even lower)
sox -n beep_error.wav synth 0.1 sine 400 vol 0.5
```

**Verification**:
```bash
# RPI: Test audio files play correctly
aplay Firmware/pregen_audio/beep_keypress.wav
aplay Firmware/pregen_audio/beep_hold.wav
aplay Firmware/pregen_audio/beep_error.wav
```

### Step 1.2: Add Beep API to Audio Firmware

**File**: `Firmware/audio_firmware.h`

Add new beep type enum and function declaration:
```c
typedef enum {
    BEEP_KEYPRESS,
    BEEP_HOLD,
    BEEP_ERROR
} BeepType;

void audio_play_beep(BeepType type);
```

**File**: `Firmware/audio_firmware.c`

Implement beep playback using pre-generated audio files. Beeps should be able to play immediately, potentially interrupting or mixing with speech.

### Step 1.3: Wire Beeps to Keypad Process

**File**: `Firmware/keypad_firmware.c`

Modify `keypad_process()` to:
1. Play `BEEP_KEYPRESS` immediately when a new key is detected
2. Play `BEEP_HOLD` when hold threshold (500ms) is reached

This keeps beep latency minimal since it happens before sending key to Software.

### Step 1.4: Firmware Integration Test

**Test**: Manual test on RPi
```bash
# RPI: Build and run firmware
cd Firmware && make clean && make && ./firmware.elf

# Press a key quickly - should hear key press beep
# Hold a key >500ms - should hear key press beep, then hold beep
# Verify beeps don't interfere with speech playback
```

**Verification checklist**:
- [ ] Key press beep plays immediately on key down
- [ ] Hold beep plays at 500ms mark
- [ ] Beeps work when speech is playing (ALSA dmix)
- [ ] Beep volume is appropriate

---

## Phase 2: Key Beep Integration (Software → Firmware)

After Firmware supports beeps, Software needs to:
1. Respect `config_get_key_beep_enabled()` setting
2. Request error beeps for invalid keys

### Step 2.1: Add Error Beep Request to Comm Protocol

**Files**: 
- `Software2/include/comm.h` - Add `comm_play_beep()` declaration
- `Software2/src/comm.c` - Implement beep request to Firmware

This allows Software to request an error beep when detecting an invalid key for the current mode.

### Step 2.2: Fire Error Beeps on Invalid Keys

**Files to modify**:
- `Software2/src/frequency_mode.c` - Error beep for [A], [B], [C] keys
- `Software2/src/set_mode.c` - Error beep for invalid keys in idle state
- `Software2/src/normal_mode.c` - Error beep for unhandled keys (if applicable)

### Step 2.3: Test Key Beep Enable/Disable

**Test**: Modify `hampod.conf`:
```ini
key_beep = 0
```

Verify no beeps play. Change to `key_beep = 1`, verify beeps resume.

---

## Phase 3: Set Mode Behavior Corrections (Software)

### Step 3.1: Fix [*] Key to Exit Set Mode

**Current behavior**: `[*]` clears value buffer but stays in Set Mode.

**Spec**: `[*]` should cancel and exit Set Mode (return to Normal Mode).

**File**: `Software2/src/set_mode.c`

Modify the `[*]` key handler:
```diff
 // [*] - Clear value
 if (key == '*' && !is_hold) {
-    clear_value_buffer();
-    speech_say_text("Cleared");
+    if (g_state == SET_MODE_EDITING) {
+        // If actively editing with digits, clear first
+        if (g_value_len > 0) {
+            clear_value_buffer();
+            speech_say_text("Cleared");
+        } else {
+            // No value entered, exit like [D]
+            set_mode_cancel_edit();
+        }
+    } else if (g_state == SET_MODE_IDLE) {
+        // Exit Set Mode entirely
+        set_mode_exit();
+    }
     return true;
 }
```

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

**File**: `Documentation/Project Overview and Onboarding/Currently_Implemented_Keys.md`

Update to reflect:
- Key beep behavior (when implemented)
- Corrected [*] behavior in Set Mode
- Any other changes

---

## Git Strategy

### Branch Structure
```
main
  └── feature/set-mode (current)
        ├── (Firmware changes - commits 1-4)
        └── (Software changes - commits 5-10)
```

### Commit Strategy

1. **Firmware commits** (Phase 1):
   - `firmware: Add beep audio files`
   - `firmware: Add beep playback API`
   - `firmware: Wire key press and hold beeps`

2. **Software commits** (Phases 2-4):
   - `software: Add comm_play_beep for error beeps`
   - `software: Fix [*] to exit Set Mode per spec`
   - `software: Fix frequency announcement format`
   - `software: Add test_set_mode unit tests`

3. **Documentation commits** (Phase 5):
   - `docs: Update Currently_Implemented_Keys.md`
   - `docs: Add integration test script`

### Merge Strategy

Once all changes verified:
```bash
git checkout main
git pull origin main
git merge feature/set-mode
git push origin main
```

---

## Verification Checklist

### Firmware (Phase 1)
- [ ] Beep files created and play correctly
- [ ] Beep API added to audio_firmware
- [ ] Key press beep fires on key down
- [ ] Hold beep fires at 500ms
- [ ] Beeps mix correctly with speech (dmix)

### Software (Phases 2-4)
- [ ] config `key_beep = 0` disables beeps
- [ ] Error beep plays on invalid key in Frequency Mode
- [ ] [*] exits Set Mode (not just clears)
- [ ] Frequency announcement uses "dot" for sub-kHz
- [ ] test_set_mode.c passes all tests

### Integration (Phase 5)
- [ ] Full workflow: Normal → Set → adjust power → exit
- [ ] Full workflow: Normal → Frequency → enter freq → confirm
- [ ] Documentation updated

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

## Estimated Effort

| Phase | Estimated Time | Confidence |
|-------|----------------|------------|
| Phase 1: Firmware beeps | 2-3 hours | High |
| Phase 2: Software integration | 1-2 hours | High |
| Phase 3: Set Mode fixes | 1-2 hours | High |
| Phase 4: Frequency format | 1 hour | Medium |
| Phase 5: Testing & docs | 1-2 hours | High |
| **Total** | **6-10 hours** | — |
