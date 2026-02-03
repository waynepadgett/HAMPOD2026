# Interrupt Fix and Persistent Piper Implementation Plan

## Problem Summary

Two critical issues were identified in the HAMPOD audio system:

### Issue 1: Speech Cannot Be Interrupted

**Root Cause**: The firmware audio processor is single-threaded with a queue. When `hal_tts_speak()` is called, it **blocks** while streaming Piper output. Interrupt commands ('i') are queued behind the blocking TTS call and only processed after speech completes.

```
Timeline (current broken behavior):
─────────────────────────────────────────────────────────────────────▶
│ TTS request queued → hal_tts_speak() BLOCKS
│                              │
│                              │ User presses key → 'i' packet QUEUED
│                              │   → But blocked behind TTS!
│                              ▼
│                        TTS finishes → NOW 'i' processed (too late!)
```

### Issue 2: Piper Starts Fresh for Each Utterance (High Latency)

**Root Cause**: Each call to `hal_tts_speak()` spawns a new Piper process via `popen()`. This incurs:
- Shell fork overhead
- Piper ONNX model loading (~150-300ms on Pi 5)

The Speech Comparison test (`speech_latency_test.c`) already demonstrated a persistent approach that eliminates this latency.

---

## Proposed Solution

### Fix 1: Interrupt Bypass

Modify the firmware IO thread to handle interrupt packets ('i') **immediately** rather than queuing them. This allows interrupts to reach the TTS while it's still running.

### Fix 2: Persistent Piper Process

Keep Piper running as a persistent subprocess. Send text via stdin, read audio from stdout. This eliminates model loading latency after the first utterance.

---

## User Review Required

> [!IMPORTANT]
> **Breaking Change Consideration**: The persistent Piper approach changes the TTS initialization behavior. If Piper fails to start, it will be detected at `hal_tts_init()` time rather than silently failing on first speak.

> [!WARNING]
> **Memory Usage**: Keeping Piper running consumes ~50-100MB RAM continuously. On Pi 3B+ with 1GB RAM, this may be significant. Please confirm this is acceptable.

**Questions for user**:
1. Is the increased memory usage acceptable for all target platforms?
2. Should we add a configuration option to disable persistent mode for low-memory systems?

---

## Proposed Changes

### Phase 1: Interrupt Bypass (Chunk 1)

Makes interrupt signals reach TTS immediately even during speech playback.

---

#### [MODIFY] [audio_firmware.c](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Firmware/audio_firmware.c)

**Change**: In `audio_io_thread()`, check for interrupt packets ('i') **before** queuing and handle them immediately.

```diff
 // In audio_io_thread(), after reading the packet data:
 
+    /* Handle interrupt packets immediately (bypass queue) */
+    if (type == AUDIO && size > 0 && buffer[0] == 'i') {
+      AUDIO_IO_PRINTF("Interrupt received - handling immediately\n");
+      hal_audio_interrupt();
+      hal_tts_interrupt();
+      
+      /* Send acknowledgment directly */
+      int ack_result = 0;
+      Inst_packet *ack_packet = create_inst_packet(
+          AUDIO, sizeof(int), (unsigned char *)&ack_result, tag);
+      write(output_pipe_fd, ack_packet, 8);
+      write(output_pipe_fd, ack_packet->data, sizeof(int));
+      destroy_inst_packet(&ack_packet);
+      continue;  /* Don't queue this packet */
+    }
+
     Inst_packet *new_packet = create_inst_packet(type, size, buffer, tag);
```

**Rationale**: The IO thread runs independently of the main audio processing loop. By handling interrupts here, they bypass the queue and take effect immediately.

---

#### [NEW] [test_interrupt_bypass.c](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Firmware/hal/tests/test_interrupt_bypass.c)

**Purpose**: Unit test that verifies interrupt handling works during TTS playback.

Test cases:
1. Verify `hal_tts_interrupt()` sets the `tts_interrupted` flag
2. Verify a long TTS utterance can be stopped mid-stream
3. Measure time from interrupt call to audio stop (should be <100ms)

---

### Phase 2: Persistent Piper (Chunks 2-4)

---

#### Chunk 2.1: Persistent Process Management

#### [MODIFY] [hal_tts_piper.c](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Firmware/hal/hal_tts_piper.c)

**Changes**:

1. Add static variables for persistent Piper pipes:
```c
static FILE *piper_stdin = NULL;   /* Write text here */
static FILE *piper_stdout = NULL;  /* Read PCM from here */
static pid_t piper_pid = -1;       /* Process ID for cleanup */
```

2. Modify `hal_tts_init()` to start persistent Piper:
```c
int hal_tts_init(void) {
    // ... existing checks ...
    
    /* Start persistent Piper process */
    int stdin_pipe[2], stdout_pipe[2];
    pipe(stdin_pipe);
    pipe(stdout_pipe);
    
    piper_pid = fork();
    if (piper_pid == 0) {
        /* Child: run Piper */
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        execlp("piper", "piper", "--model", PIPER_MODEL_PATH,
               "--length_scale", PIPER_SPEED, "--output_raw", NULL);
        exit(1);
    }
    
    /* Parent: save pipe handles */
    piper_stdin = fdopen(stdin_pipe[1], "w");
    piper_stdout = fdopen(stdout_pipe[0], "r");
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);
    
    initialized = 1;
    return 0;
}
```

3. Modify `hal_tts_speak()` to use persistent pipes:
```c
int hal_tts_speak(const char *text, const char *output_file) {
    (void)output_file;
    
    if (!initialized || piper_stdin == NULL) {
        return hal_tts_init() == 0 ? hal_tts_speak(text, output_file) : -1;
    }
    
    tts_interrupted = 0;
    
    /* Send text to Piper (with newline to trigger processing) */
    fprintf(piper_stdin, "%s\n", text);
    fflush(piper_stdin);
    
    /* Stream PCM output in chunks */
    int16_t chunk_buffer[TTS_CHUNK_SAMPLES];
    while (!tts_interrupted) {
        size_t bytes_read = fread(chunk_buffer, 1, TTS_CHUNK_BYTES, piper_stdout);
        if (bytes_read == 0) break;
        if (hal_audio_write_raw(chunk_buffer, bytes_read / 2) != 0) break;
    }
    
    return 0;
}
```

4. Modify `hal_tts_cleanup()` to terminate Piper:
```c
void hal_tts_cleanup(void) {
    if (piper_stdin) { fclose(piper_stdin); piper_stdin = NULL; }
    if (piper_stdout) { fclose(piper_stdout); piper_stdout = NULL; }
    if (piper_pid > 0) { kill(piper_pid, SIGTERM); waitpid(piper_pid, NULL, 0); }
    initialized = 0;
}
```

---

#### Chunk 2.2: End-of-Utterance Detection

**Challenge**: How do we know when Piper has finished generating audio for one utterance?

**Solution Options**:
1. **Silence detection**: Read until we get a chunk of all zeros (risky - what if the audio has silence?)
2. **Timeout**: Wait for N ms of no data (adds latency)
3. **Sentinel text**: Send a special marker after each utterance that Piper will produce a known audio pattern for
4. **Pre-calculate duration**: Send text, estimate duration from text length (unreliable)

**Recommended**: Option 2 with a short timeout (50ms). If no data arrives within 50ms after receiving at least some data, assume utterance is complete.

```c
/* In hal_tts_speak read loop */
while (!tts_interrupted) {
    /* Set read timeout using select() */
    fd_set fds;
    struct timeval tv = {0, 50000}; /* 50ms timeout */
    FD_ZERO(&fds);
    FD_SET(fileno(piper_stdout), &fds);
    
    int ready = select(fileno(piper_stdout) + 1, &fds, NULL, NULL, &tv);
    if (ready <= 0) break;  /* Timeout or error = end of utterance */
    
    size_t bytes_read = fread(chunk_buffer, 1, TTS_CHUNK_BYTES, piper_stdout);
    // ...
}
```

---

#### [NEW] [test_persistent_piper.c](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Firmware/hal/tests/test_persistent_piper.c)

**Purpose**: Unit tests for persistent Piper functionality.

Test cases:
1. Verify `hal_tts_init()` starts Piper process (check piper_pid > 0)
2. Verify `hal_tts_speak("hello", NULL)` produces audio
3. Verify multiple sequential speaks work (second speak faster than first)
4. Verify `hal_tts_cleanup()` terminates Piper
5. Verify interrupt during persistent speak stops audio

---

#### [MODIFY] [Makefile](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Firmware/hal/tests/Makefile)

Add new test targets:
```makefile
test_interrupt_bypass: test_interrupt_bypass.c $(HAL_AUDIO) $(HAL_TTS) $(HAL_USB_UTIL)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test_persistent_piper: test_persistent_piper.c $(HAL_AUDIO) $(HAL_TTS) $(HAL_USB_UTIL)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
```

---

## Verification Plan

### Automated Tests

#### Unit Tests (Run on Raspberry Pi)

```bash
# Build all tests
cd ~/HAMPOD2026/Firmware/hal/tests
make clean
make all

# Run automated tests (Phase 1)
./test_interrupt_bypass

# Run automated tests (Phase 2)  
./test_persistent_piper

# Run existing audio tests (regression)
./test_hal_audio
```

**Expected Results**:
- All tests pass with 0 failures
- Interrupt test shows <100ms interrupt latency
- Persistent Piper test shows second utterance faster than first

---

#### Integration Test

```bash
# Run full integration test
cd ~/HAMPOD2026/Firmware/hal/tests
sudo ./test_hal_integration
```

Press keys on the keypad and verify:
1. Key names are spoken
2. Pressing a key during speech interrupts previous speech
3. Audio latency feels responsive (sub-500ms from keypress to first audio)

---

### Manual Testing on Raspberry Pi

#### Test 1: Interrupt During Speech (Phase 1)

**Setup**:
1. SSH into Raspberry Pi
2. Build and start HAMPOD: `./run_hampod.sh`

**Steps**:
1. Press `[1]` key to trigger frequency announcement
2. While frequency is being announced, press any other key (e.g., `[2]`)
3. Observe: Does the frequency announcement stop?

**Expected Result**: 
- First announcement stops immediately when second key is pressed
- Second key's action begins (beep and/or announcement)

**Pass Criteria**: Speech stops within ~100ms of keypress

---

#### Test 2: Latency Improvement (Phase 2)

**Setup**:
1. Time how long from keypress to first audio

**Steps**:
1. Press `[1]` key
2. Measure time from key press to first sound (use stopwatch or subjective feel)
3. Compare to pre-change behavior

**Expected Result**:
- First utterance after startup: similar latency (Piper still loading model)
- Second and subsequent utterances: noticeably faster (~100-200ms improvement)

---

#### Test 3: Long Session Stability (Phase 2)

**Setup**:
1. Leave HAMPOD running for 10+ minutes

**Steps**:
1. Press keys periodically over 10 minutes
2. Verify speech continues to work
3. Check for memory leaks: `ps aux | grep hampod`

**Expected Result**:
- Memory usage stays stable
- Speech continues to work correctly

---

### Regression Tests

Run the existing test suite to ensure no regressions:

```bash
# Firmware HAL tests
cd ~/HAMPOD2026/Firmware/hal/tests
make test

# Software2 tests  
cd ~/HAMPOD2026/Software2
make test
```

**Expected**: All existing tests still pass.

---

## Implementation Chunks (Build Order)

| Chunk | Description | Files Modified | Can Build? | Can Test? |
|-------|-------------|----------------|------------|-----------|
| 1.1 | Interrupt bypass in IO thread | `audio_firmware.c` | ✅ | ✅ Manual |
| 1.2 | Interrupt bypass unit test | `test_interrupt_bypass.c`, `Makefile` | ✅ | ✅ Automated |
| 2.1 | Persistent Piper process management | `hal_tts_piper.c` | ✅ | ✅ Manual |
| 2.2 | End-of-utterance detection with timeout | `hal_tts_piper.c` | ✅ | ✅ Manual |
| 2.3 | Persistent Piper unit tests | `test_persistent_piper.c`, `Makefile` | ✅ | ✅ Automated |
| 3.1 | Integration testing | — | — | ✅ Manual |
| 3.2 | Regression testing | — | — | ✅ Automated |

---

## Rollback Plan

If issues are found after merging:
1. `git revert HEAD` to undo the merge commit
2. Or cherry-pick only the interrupt fix if persistent Piper has issues

---

## Approval Gates

- [ ] **Gate 1**: User approves this plan before implementation begins
- [ ] **Gate 2**: User approves Phase 1 (interrupt fix) after testing
- [ ] **Gate 3**: User approves Phase 2 (persistent Piper) after testing
- [ ] **Gate 4**: User approves merge to main after all tests pass
