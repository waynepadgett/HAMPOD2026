# Firmware Bug Fix Plan

This plan addresses the three known Firmware bugs identified during Phase Zero Software development. Each bug is fixed separately with incremental verification to ensure no regressions.

> [!IMPORTANT]
> **Approach:** Fix each bug in isolation, verify after each change, and compile at every step.

---

## Bug 1: Stale Pipes Block Startup

### Problem
`mkfifo()` fails with "File exists" error if named pipes from a previous run weren't cleaned up (e.g., unclean shutdown, Ctrl+C).

### Affected File
[firmware.c](file:///c:/Users/wayne/github/hampod/HAMPOD2026/Firmware/firmware.c) - Lines 120-180

### Root Cause
The code calls `mkfifo()` without first checking/removing existing pipe files.

### Fix Steps

#### Step 1.1: Add unlink calls before each mkfifo
Add `unlink(PIPE_NAME)` before each `mkfifo()` to remove stale pipes. `unlink()` silently succeeds if file doesn't exist.

```c
// Before each mkfifo, add:
unlink(OUTPUT_PIPE);
if (mkfifo(OUTPUT_PIPE, 0666) == -1) { ... }
```

Apply to all 6 pipes: Firmware_o, Firmware_i, Keypad_i, Keypad_o, Speaker_i, Speaker_o

#### Step 1.2: Compile and verify
```bash
cd ~/HAMPOD2026/Firmware && make clean && make
```

### Verification

**Test 1.1 - Automated:**
```bash
# Create stale pipes manually
cd ~/HAMPOD2026/Firmware
mkfifo Firmware_i Firmware_o Keypad_i Keypad_o Speaker_i Speaker_o 2>/dev/null
# Start Firmware (should NOT fail with "File exists")
timeout 5s ./firmware.elf
# Expected: Firmware starts successfully, no mkfifo errors
```

**Test 1.2 - Normal startup:**
```bash
# Clean slate
cd ~/HAMPOD2026/Firmware && rm -f *_i *_o
# Start normally
./firmware.elf &
sleep 2
# Verify pipes created
ls -la *_i *_o | wc -l  # Should show 6 pipes
pkill -9 firmware
```

---

## Bug 2: Audio IO Thread Repeat Bug

### Problem
`audio_io_thread` continues reading stale data after Software disconnects, causing last speech to repeat.

### Affected File
[audio_firmware.c](file:///c:/Users/wayne/github/hampod/HAMPOD2026/Firmware/audio_firmware.c) - Lines 161-164

### Root Cause
The `read()` calls don't check return values. When the pipe is closed by Software, `read()` returns 0 or -1, but the code proceeds to process garbage data.

### Fix Steps

#### Step 2.1: Add error checking to read calls
Wrap reads with error checking and break loop on EOF/error:

```c
ssize_t bytes;
bytes = read(i_pipe, &type, sizeof(Packet_type));
if (bytes <= 0) {
    AUDIO_IO_PRINTF("Pipe closed or error, exiting thread\n");
    break;
}
// ... similar for other reads
```

#### Step 2.2: Compile and verify
```bash
cd ~/HAMPOD2026/Firmware && make clean && make
```

### Verification

**Test 2.1 - Speech doesn't repeat (Manual):**
1. Start Firmware: `cd ~/HAMPOD2026/Firmware && ./firmware.elf`
2. In another terminal, run imitation_software or Software2 test
3. Send a speech command (e.g., "Hello")
4. Disconnect Software (Ctrl+C)
5. Observe Firmware output - should NOT repeat the last audio

**Test 2.2 - Normal operation (use existing test):**
```bash
cd ~/HAMPOD2026/Software2
make tests
./bin/test_comm_write  # Should play "Hello World" once
# Verify no repeated audio after test exits
```

---

## Bug 3: Keypad Hold Detection

### Problem
HAL only reports `ev.value == 1` (key press down), ignoring `ev.value == 2` (auto-repeat) and `ev.value == 0` (release). Software cannot detect held keys.

### Affected File
[hal_keypad_usb.c](file:///c:/Users/wayne/github/hampod/HAMPOD2026/Firmware/hal/hal_keypad_usb.c) - Line 145

### Root Cause
The condition `ev.value == 1` filters out repeat and release events. Software only sees the initial press, then continuous '-' (no key).

### Fix Approach
**Option A (Recommended):** Track key state - report the held key continuously until release event (`ev.value == 0`).

### Fix Steps

#### Step 3.1: Add static state to track currently held key
Add a static variable to remember the last pressed key:

```c
static char held_key = '-';  // Currently held key, or '-' for none
```

#### Step 3.2: Update logic to handle press, hold, and release
Modify `hal_keypad_read()`:
- On `ev.value == 1` (press): Set `held_key` to the pressed key, return event
- On `ev.value == 0` (release): Clear `held_key` to '-', return release event
- On `ev.value == 2` (repeat): Return the held key as still pressed
- If no event read but key is held: Return held key

#### Step 3.3: Compile and verify
```bash
cd ~/HAMPOD2026/Firmware && make clean && make
```

### Verification

**Test 3.1 - Hold detection (use existing HAL test):**
```bash
cd ~/HAMPOD2026/Firmware/hal/tests
make test_hal_keypad
./test_hal_keypad
# Press and HOLD a key for 2+ seconds
# Expected: Should see continuous reports of same key while held
```

**Test 3.2 - Normal key presses still work:**
```bash
cd ~/HAMPOD2026/Software2
./bin/test_keypad_events
# Press keys quickly (not holding)
# Expected: Each key detected as single press
```

---

## Execution Order

1. **Fix Bug 1** (Stale Pipes) first - easiest, least risk
2. **Fix Bug 2** (Audio Repeat) next - isolated to audio thread
3. **Fix Bug 3** (Keypad Hold) last - requires careful testing

After all bugs fixed, run full integration test.

---

## Verification Summary

| Bug | Test Command | Expected Result |
|-----|--------------|-----------------|
| 1 | Create stale pipes, start Firmware | Starts without error |
| 2 | Run speech test, disconnect Software | No repeated audio |
| 3 | Hold key >2s in test_hal_keypad | Continuous key reports |

---

## Related Documentation

- [fresh-start-phase-zero-plan.md](file:///c:/Users/wayne/github/hampod/HAMPOD2026/Documentation/Project_Overview_and_Onboarding/fresh-start-phase-zero-plan.md) - Step 0.8
