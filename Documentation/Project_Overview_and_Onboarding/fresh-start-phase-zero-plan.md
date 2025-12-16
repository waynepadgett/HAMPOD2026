# Fresh Start: Phase Zero Plan (Core Infrastructure)

> **Parent Plan:** [Fresh Start: Big Picture Plan](fresh-start-big-plan.md)
> **Next Phase:** [Phase 1: Frequency Mode](fresh-start-first-freq-mode.md)

## Overview

This phase builds the foundation for the new Software layer (`Software2/`). The goal is to establish stable communication with the Firmware (via pipes) and provide clean APIs for subsequent phases (modes).

**Key insight:** The Firmware is already complete and handles all USB I/O. Our Software2 just needs to send/receive packets over named pipes.

Each step is a **compile-ready chunk** with specific verification steps.

---

## Shared Dependencies (Prerequisite)

We need a home for shared definitions used across modules.

### Step 0.1: Directory Structure & Shared Headers
**Goal:** Create clean project structure and common types.

1. **Create Directories:**
   ```
   Software2/
     ├── include/       # Public headers (hampod_core.h)
     ├── src/           # Source files
     ├── bin/           # Output executables
     └── tests/         # Integration tests
   ```

2. **Common Header (`include/hampod_core.h`)**:
   - `KeyPress` struct (reused from old code)
   - Boolean type (stdbool.h)
   - Common constants (DEBUG flags)

3. **Makefile Skeleton**:
   - Compiler flags (`-Wall -pthread -I./include`)
   - Target for `Software2` binary
   - Target for `test_*` binaries

**Verification:**
- Run `make` → creates directories, compiles empty main.c successfully.

---

## Module 1: Pipe Communication (`comm.c`)
**Goal:** Abstract low-level pipe I/O from the rest of the app.

### Step 1.1: Pipe Reader
- **Function:** `comm_init()` - Opens `Keypad_o` and `Firmware_o` for reading
- **Function:** `comm_read_keypad()` - Blocking read from keypad pipe
- **Function:** `comm_close()` - Cleanup

**Verification (Test 1.1):**
- Write `tests/test_comm_read.c`
- Prints "Waiting for key..."
- Run on Pi: Press keys on keypad → confirms characters appear in console.

### Step 1.2: Pipe Writer
- **Function:** `comm_send_audio(char type, char* payload)` - Writes to `Firmware_i`
- Formats packets correctly: `d<text>` or `p<path>` or `s<text>`

**Verification (Test 1.2):**
- Write `tests/test_comm_write.c`
- Sends "dHello World" packet
- Run on Pi: Should hear Festival speak "Hello World" (since Firmware is running).

---

## Module 2: Speech System (`speech.c`)
**Goal:** Non-blocking speech with queue and audio caching.

### Step 2.1: Audio Queue & Thread
- **Struct:** `AudioPacket` (type, payload)
- **Thread:** `speech_thread` - loops, pops from queue, calls `comm_send_audio`
- **API:** `speech_say_text(const char* text)` - pushes to queue

**Verification (Test 2.1):**
- Write `tests/test_speech_queue.c`
- Call `speech_say_text("One")`, `speech_say_text("Two")`, `speech_say_text("Three")` rapidly
- Run on Pi: Should hear "One", "Two", "Three" sequentially without overlapping/cutting off.

### Step 2.2: Audio Caching (Reused Logic)
- **Feature:** `check_cache(text)`
- Logic:
  1. Hash the text
  2. Check if cached WAV exists in `../Firmware/pregen_audio/`
  3. If yes: send `p<path>` (fast playback)
  4. If no: send `d<text>` (Festival TTS)

**Verification (Test 2.2):**
- Run `test_speech_queue` again.
- Second run of same text should be noticeably snappier if caching works (or check Firmware logs).

### Step 2.3: Dictionary Substitution
- **Feature:** Replace "MHz" → "Megahertz", "." → "point"
- **Implementation:** Simple string replacement before queuing

**Verification (Test 2.3):**
- `speech_say_text("14.250 MHz")`
- Should hear: "fourteen point two five zero megahertz"

---

## Module 3: Keypad Input (`keypad.c`)
**Goal:** Turn raw pipe characters into rich `KeyPress` events.

### Step 3.1: Polling Loop & Event Logic
- **Thread:** `keypad_thread` - polls `comm_read_keypad()`
- **Logic:** Implements the "Hold" detection logic (reused from old code)
- **Callback System:** `keypad_register_callback(void (*cb)(KeyPress*))`

**Verification (Test 3.1):**
- Write `tests/test_keypad_events.c`
- Register callback that prints: `[Key: 5, Shift: 0, Hold: YES/NO]`
- Run on Pi:
  - Press '5' → Output: `Key: 5, Hold: NO`
  - Hold '5' → Output: `Key: 5, Hold: YES`

### Step 3.2: Key Beeps (Zero Latency)
- **Optimization:** If Firmware modification was done (Phase 0.a), enable it.
- **Alternative:** Call `aplay beep.wav` in background process on key press event.
- **Note:** This is purely audio feedback; logic remains the same.

**Verification (Test 3.2):**
- Run `test_keypad_events`
- Ensure beep happens *immediately* on press, even while speech is playing.

---

## Module 4: Configuration (`config.c`)
**Goal:** Load/Save settings.

### Step 4.1: Key-Value Storage
- **Struct:** `ConfigState` (volume, speed, radio_model, etc.)
- **Function:** `config_load(path)`, `config_save(path)`
- **Format:** Simple text file `key=value`

**Verification (Test 4.1):**
- Write `tests/test_config.c`
- Save settings, restart app, load settings
- Verify values persist.

---

## Final Phase Zero Integration

**Goal:** All modules running together.

### Step 0.8: Firmware Bug Fixes (Pre-Integration)

Before integration testing, fix known Firmware issues:

1. **Audio IO Thread Repeat Bug** (`Firmware/audio_firmware.c`)
   - **Problem:** `audio_io_thread` continues reading stale data after Software disconnects
   - **Symptom:** Last speech item repeats multiple times
   - **Fix:** Add error checking to `read()` calls (lines 161-164) to detect disconnection

2. **Stale Pipes Block Startup** (`Firmware/firmware.c`)
   - **Problem:** `mkfifo()` fails if pipes from previous run weren't cleaned up
   - **Symptom:** Firmware exits with `mkfifo: File exists` error
   - **Fix:** Add `unlink()` calls before `mkfifo()` to remove stale pipes

3. **Keypad Hold Detection** (`Firmware/hal/hal_keypad_usb.c`)
   - **Problem:** HAL only reports `ev.value == 1` (key press), ignores `ev.value == 2` (key repeat)
   - **Root Cause:** Line 145 filters for `ev.value == 1` only, so held keys won't report repeat events
   - **Symptom:** Software cannot detect held keys - only sees initial press, then `-` continuously
   - **Fix:** Either:
     1. Also handle `ev.value == 2` (auto-repeat) events, OR
     2. Track key state in HAL - report key as held until `ev.value == 0` (release)

**Verification:**
- Run speech queue test - speech should NOT repeat
- Firmware should start cleanly after unclean shutdown
- Keypad hold detection should work (key held >500ms triggers hold event)

### Step 0.9: Integration Test (`main_phase0.c`)
- Init Config
- Init Comm
- Init Speech
- Init Keypad
- **Logic:**
  - When key pressed → Speak "You pressed <key>"
  - If held → Speak "You held <key>"

**Success Criteria:**
1. App compiles clean
2. Key presses trigger speech
3. Key holds trigger different speech
4. Speech is queued correctly (no cutoffs)
5. Beeps happen instantly
6. No speech repeats after disconnect

---

## Execution Checklist

- [x] **Step 0.1** Directories & Headers ✅ (2025-12-14)
- [x] **Step 1.1** Comm Reader (Keypad) ✅ (2025-12-14)
- [x] **Step 1.2** Comm Writer (Audio) ✅ (2025-12-14)
- [x] **Step 2.1** Speech Queue ✅ (2025-12-14)
- [ ] **Step 2.2** Audio Caching
- [ ] **Step 2.3** Dictionary Sub
- [x] **Step 3.1** Keypad Events ⚠️ (2025-12-14) - Press detection works; Hold detection blocked by Firmware (see 0.8)
- [ ] **Step 4.1** Config Load/Save
- [ ] **Step 0.8** Firmware Bug Fixes
- [ ] **Step 0.9** Integration Test

### Notes

**Step 3.1 Hold Detection Issue:** The Firmware's keypad process reports each key press only ONCE, then immediately returns '-' (no key). It does not continuously report the key while it's being held. This means the Software cannot detect holds via polling. Options to fix:
1. **Firmware Enhancement:** Modify `keypad_firmware.c` to continuously report the held key until it's released
2. **Alternative:** Count rapid consecutive presses as a "double-tap" feature instead of holds
