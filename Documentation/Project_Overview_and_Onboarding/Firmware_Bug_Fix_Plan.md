# Firmware Bug Fix Plan

This plan addresses the known Firmware bugs identified during Phase Zero Software development. Each bug was fixed separately with incremental verification to ensure no regressions.

> [!NOTE]
> **Status:** All bugs fixed and verified. See commit history for details.

---

## Bug 1: Stale Pipes Block Startup ✅ FIXED

**Commit:** `39c890f`

### Problem
`mkfifo()` fails with "File exists" error if named pipes from a previous run weren't cleaned up.

### Solution
Added `unlink()` before each `mkfifo()` call in `firmware.c`.

---

## Bug 2: Audio IO Thread Repeat Bug ✅ FIXED

**Commits:** `68cd723`, `58a8e6d`

### Problem
IO threads continue reading garbage data after Software disconnects.

### Solution
Added `read()` error checking to all IO threads:
- `firmware.c` - `io_buffer_thread()`
- `audio_firmware.c` - `audio_io_thread()`
- `keypad_firmware.c` - `keypad_io_thread()`

Also fixed `test_comm_read.c` to call `comm_wait_ready()` before polling keypad.

---

## Bug 3: Keypad Hold Detection ✅ FIXED

**Commit:** `53baaaa`

### Problem
HAL only reported key press events (`ev.value == 1`), ignoring hold/repeat and release events.

### Solution
- Added `hold_state` struct to track held key in `hal_keypad_usb.c`
- Handle `ev.value == 0` (release), `ev.value == 1` (press), `ev.value == 2` (repeat)
- Updated `test_hal_integration.c` to use TTS HAL and demonstrate hold detection

---

## Bug 4: imitation_software Segfault ✅ FIXED

**Commit:** `0606425`

### Problem
`imitation_software` would segfault when any key was pressed. The keypad input character (e.g., '5' = 0x35) was used directly as an array index into `keypad_names[]` (16 elements), causing out-of-bounds access.

### Solution
1. **imitation_software.c:**
   - Rewrote `index_getter()` with correct switch-case mapping
   - Changed mode toggle from `== 12` to `== '*'` (character)
   - Added bounds checking before array access
   - Added NULL checks after `read_from_pipe()` calls
   - Added graceful exit with cleanup label

2. **hampod_firm_packet.c:**
   - Added NULL checks to `destroy_inst_packet()` to prevent segfault

---

## Verification Summary

| Bug | Status | Verification |
|-----|--------|--------------|
| 1: Stale Pipes | ✅ Fixed | Firmware starts with existing pipes |
| 2: IO Thread Repeat | ✅ Fixed | Threads exit cleanly on disconnect |
| 3: Keypad Hold | ✅ Fixed | test_hal_integration shows hold detection |
| 4: imitation_software | ✅ Fixed | All keys work, graceful exit |

---

## Related Documentation

- [fresh-start-phase-zero-plan.md](file:///c:/Users/wayne/github/hampod/HAMPOD2026/Documentation/Project_Overview_and_Onboarding/fresh-start-phase-zero-plan.md) - Step 0.8

