# HAMPOD2026 Regression Testing Plan

**Created:** December 19, 2025  
**Last Updated:** December 19, 2025  
**Status:** Active

---

## 📋 Overview

This document outlines the regression testing strategy for the HAMPOD2026 project. The goal is to ensure that changes to the codebase do not introduce regressions in existing functionality, particularly for hardware integration (keypad, audio) and inter-layer communication (Firmware ↔ Software).

### When to Run Tests

- ✅ **Required:** Before any significant code commit (Firmware, HAL, Software)
- ✅ **Required:** After modifying communication protocols or packet formats
- ✅ **Required:** After hardware-related changes
- ⏩ **Optional:** Documentation-only commits (no testing required)
- ⏩ **Optional:** Config file or comment-only changes

---

## 🧪 Core Regression Tests

### Test 1: HAL Integration Test

**Purpose:** Verifies that the Hardware Abstraction Layer components (Keypad, Audio, TTS) are functioning correctly together without requiring the full Firmware/Software stack.

**Location:** `Firmware/hal/tests/test_hal_integration`

#### Prerequisites
- SSH connection to Raspberry Pi
- USB keypad connected
- USB audio device connected
- Piper TTS model installed

#### Execution Steps

```bash
# 1. Build the test (on RPi)
cd ~/HAMPOD2026/Firmware/hal/tests
make clean
make test_hal_integration

# 2. Run from Firmware directory (required for model paths)
cd ~/HAMPOD2026/Firmware
sudo hal/tests/test_hal_integration
```

#### Expected Behavior
1. **Startup:** System should announce "System Ready" or similar
2. **Keypad:** Pressing keys should announce the key name (e.g., "One", "Enter")
3. **Hold Detection:** Holding a key for >1 second should announce "key held"
4. **Exit:** Use `Ctrl+C` to exit cleanly

#### Verification Checklist

| Check | Expected | Actual | Pass/Fail |
|-------|----------|--------|-----------|
| Test starts without errors | No error messages | | |
| Keypad HAL initializes | "Initializing Keypad HAL..." message | | |
| Audio HAL initializes | "Initializing Audio HAL..." message | | |
| TTS HAL initializes | "Initializing TTS HAL..." message | | |
| Key press detected | Key character printed to console | | |
| Audio heard on key press | User confirms audio output | | |
| Hold detection works | "held" announcement after 1s hold | | |
| Clean exit on Ctrl+C | "Cleaning up..." message | | |

#### ⚠️ Critical Verification Rules

1. **Key Press Detection:**
   - If the test is running and NO key presses are being detected:
     - **DO NOT** assume the test is working
     - **ASK:** "Are you pressing keys on the USB keypad? Please confirm the key presses are being logged to the console."
   - If logs show only `-` (no key) events, the keypad may not be connected or recognized

2. **Audio Verification:**
   - If logs show audio commands being processed but no confirmation of audio output:
     - **DO NOT** assume audio worked
     - **ASK:** "Did you hear speech output from the speaker? Please confirm audio was audible."
   - Check USB speaker is connected: `aplay -l`

---

### Test 2: Imitation Software Test (Firmware Integration)

**Purpose:** Verifies bidirectional communication between Firmware and a Software emulator, testing the complete packet protocol, keypad polling, and audio playback pipeline.

**Location:** `Firmware/imitation_software`

#### Prerequisites
- SSH connection to Raspberry Pi
- USB keypad connected
- USB audio device connected
- Firmware built and ready

#### Execution Steps

**Option A: Manual Execution**
```bash
# Terminal 1 - Start Firmware
cd ~/HAMPOD2026/Firmware
make clean && make
./firmware.elf

# Terminal 2 - Start Imitation Software
cd ~/HAMPOD2026/Firmware
./imitation_software
```

**Option B: Automated Script**
```bash
# Run from RPi
cd ~/HAMPOD2026
./Documentation/scripts/run_remote_test.sh
```

#### Expected Behavior
1. **Startup:** Initial TTS announcement: "This is a keypad and audio integration test..."
2. **Keypad Polling:** Console shows `keypad says XX (X)` for each poll cycle
3. **Key Press:** Pressing a key shows the key value and plays pre-generated audio
4. **Mode Toggle:** Pressing `*` toggles between Normal and DTMF mode
5. **Audio Playback:** Pre-generated audio files play for each key press

#### Verification Checklist

| Check | Expected | Actual | Pass/Fail |
|-------|----------|--------|-----------|
| Firmware starts | No crash, pipes created | | |
| Imitation Software connects | "Successful connection" message | | |
| Initial TTS plays | User hears announcement | | |
| Keypad polling works | Continuous `keypad says 2d (-)` logs | | |
| Key press detected | Key value logged (e.g., `keypad says 35 (5)`) | | |
| Audio plays on key press | User confirms audio heard | | |
| Mode toggle works | "Mode toggled to DTMF/Normal" message | | |
| Clean shutdown | "Closing pipes..." message | | |

#### ⚠️ Critical Verification Rules

1. **No Key Detection:**
   - If logs only show `keypad says 2d (-)` (hex 0x2D = '-') for extended periods:
     - This means NO keys are being pressed
     - **ASK:** "Please press a key on the USB keypad. Are key presses showing in the log?"

2. **Audio Verification:**
   - If logs show `Playing: X` messages but no audio confirmation:
     - **DO NOT** assume audio worked
     - **ASK:** "Did you hear the audio file play for the key you pressed?"

3. **Pipe Errors:**
   - If you see "Pipe closed or read error" messages:
     - Clean stale pipes: `rm -f Firmware_i Firmware_o`
     - Kill stale processes: `killall -9 firmware.elf imitation_software`
     - Re-run the test

---

### Test 3: Phase 0.9 Integration Test (Software2 + Router Thread)

**Purpose:** Verifies the Software2 router thread architecture, which handles concurrent keypad polling and speech output without packet type conflicts. **This is the only test that exercises the router thread!**

**Location:** `Software2/bin/phase0_test`

#### What This Test Covers (that other tests don't)
- ✅ Router thread in `comm.c` dispatching packets by type
- ✅ Concurrent keypad and speech threads waiting for responses
- ✅ Response queue FIFO ordering and timeout behavior
- ✅ Full Software2 → Firmware → Software2 roundtrip

#### Prerequisites
- SSH connection to Raspberry Pi
- USB keypad connected
- USB audio device connected
- Piper TTS model installed
- Firmware NOT already running (script starts it)

#### Execution Steps

**Option A: Automated Script (Recommended)**
```bash
# Run from RPi
cd ~/HAMPOD2026
./Documentation/scripts/Regression_Phase0_Integration.sh
```

**Option B: Manual Execution**
```bash
# Terminal 1 - Start Firmware
cd ~/HAMPOD2026/Firmware
make clean && make
sudo ./firmware.elf

# Terminal 2 - Start Phase0 Test
cd ~/HAMPOD2026/Software2
make phase0_test
sudo ./bin/phase0_test
```

#### Expected Behavior
1. **Startup:** Announcement: "Phase zero integration test ready. Press any key."
2. **Router Thread:** Logs show "Router thread started"
3. **Key Press:** "You pressed X" announcement for each key
4. **Key Hold:** "You held X" announcement for long presses
5. **No Errors:** NO "packet type mismatch" or "bruh" errors

#### Verification Checklist

| Check | Expected | Actual | Pass/Fail |
|-------|----------|--------|-----------|
| Firmware starts and creates pipes | Pipes exist in Firmware/ | | |
| Software2 connects | "Firmware communication initialized" | | |
| Router thread starts | "Router thread started" message | | |
| Startup announcement plays | User hears audio | | |
| Key press detected | "You pressed X" in logs | | |
| Audio heard on key press | User confirms audio output | | |
| Hold detection works | "You held X" announcement | | |
| **No packet errors** | No "type mismatch" or "bruh" | | |
| Clean exit on timeout/Ctrl+C | "Shutting down..." message | | |

#### ⚠️ Critical Verification Rules

1. **Packet Type Errors:**
   - If you see "packet type mismatch", "expected KEYPAD got AUDIO", or "bruh":
     - **This is a FAILURE** - the router thread is not working correctly
     - Check that `comm.c` has the router implementation
     - Verify `comm_wait_ready()` starts the router

2. **Audio Verification:**
   - If logs show speech commands but no audio:
     - **DO NOT** assume audio worked
     - **ASK:** "Did you hear the startup announcement and key press feedback?"

---

## 📝 Test Execution Log Template

Use this template to document each test run:

```markdown
## Test Run: [Date/Time]

**Tester:** [Name]
**Commit:** [Git commit hash before testing]
**Purpose:** [Why is this test being run?]

### HAL Integration Test
- [ ] Started successfully
- [ ] Keypad detected: Yes / No / N/A
- [ ] Audio heard: Yes / No / N/A
- [ ] Result: PASS / FAIL / SKIPPED

### Imitation Software Test
- [ ] Firmware started successfully
- [ ] Connection established
- [ ] Initial audio played: Yes / No
- [ ] Keypad input detected: Yes / No
- [ ] Audio playback working: Yes / No
- [ ] Result: PASS / FAIL / SKIPPED

### Notes:
[Any issues, warnings, or observations]

### Conclusion:
[PASS - Ready to commit / FAIL - Issues must be fixed]
```

---

## 🚀 Future Test Improvements

### Short-Term Improvements

1. **Automated Pass/Fail Detection**
   - Add log parsing to detect expected patterns
   - Return exit codes based on test success criteria
   - Create a CI-friendly test runner

2. **Test Timeouts**
   - Add configurable timeouts for all tests
   - Auto-fail if expected events don't occur within timeout

3. **Audio Verification Enhancement**
   - Log audio file paths and durations
   - Add checksum verification for pre-generated audio files

### Medium-Term Additions

1. **Software2 Communication Tests**
   - `test_comm_read.c` - Verify reading from Firmware
   - `test_comm_write.c` - Verify writing to Firmware
   - `test_keypad_events.c` - Verify keypad event handling
   - `test_speech_queue.c` - Verify speech queue operations

2. **End-to-End Mode Tests**
   - Frequency mode announcements
   - DTMF relay mode
   - Memory mode operations

3. **Stress Tests**
   - Rapid key pressing
   - Long-running stability tests
   - Memory leak detection

### Long-Term Goals

1. **Unit Test Coverage**
   - Port existing `Software/UnitTesting/` tests to Software2
   - Add unit tests for:
     - Packet serialization/deserialization
     - Key mapping functions
     - Mode state machine transitions
     - Configuration parsing
   - Target: Cover functionality not tested by integration tests

2. **Hardware Mock Layer**
   - Create software mocks for keypad input
   - Create audio output capture for verification
   - Enable testing without physical hardware

3. **Continuous Integration**
   - Automated test execution on Pi after each push
   - Test result reporting and tracking
   - Regression trend analysis

---

## 🔧 Troubleshooting

### Test Won't Start

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| "Failed to initialize keypad" | USB keypad not connected | Check `ls /dev/input/by-id/*kbd*` |
| "No audio devices found" | USB audio not connected | Check `aplay -l` |
| "Cannot open pipe" | Stale pipes from previous run | `rm -f Firmware_i Firmware_o` |
| Permission denied | Need sudo for input devices | Run with `sudo` |

### No Key Detection

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| Only `-` or `0x2D` in logs | No keys actually pressed | Press keys on USB keypad |
| No response to any key | Wrong input device | Check `/dev/input/by-id/` for keypad |
| Intermittent detection | USB connection issue | Try different USB port |

### No Audio Output

| Symptom | Possible Cause | Solution |
|---------|----------------|----------|
| Logs show audio commands | Speaker not connected | Check USB speaker |
| No TTS output | Piper not installed | Install Piper and model |
| Silent audio files | Wrong audio device | Check ALSA device with `aplay -l` |

---

## 📚 Related Documentation

- [RPi Setup Guide](Project_Overview_and_Onboarding/RPi_Setup_Guide.md) - Hardware setup instructions
- [Firmware Build Guide](../Firmware/BUILD.md) - Build system documentation
- [Firmware Test Fix Report](Project_Overview_and_Onboarding/Completed_Tasks/firmware_test_fix.md) - Previous debugging notes

---

*This is a living document. Update it as new tests are added or procedures change.*
