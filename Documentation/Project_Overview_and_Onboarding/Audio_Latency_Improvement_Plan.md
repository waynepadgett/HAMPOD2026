# Audio Latency and Beep Improvement Plan (Revised)

> **Goal**: Enable key beeps without overloading the speech queue or causing excessive latency. Focus on audio architecture only.

> **Status**: ✅ **IMPLEMENTED** (January 2, 2026)

---

## Implementation Summary

All phases have been implemented on the `set-mode-correction` branch:

| Part | Status | Implementation |
|------|--------|----------------|
| **Persistent ALSA** | ✅ Complete | `hal_audio_usb.c` with persistent `aplay` pipeline |
| **RAM-Cached Beeps** | ✅ Complete | 3 beep types loaded at init, played via `hal_audio_play_beep()` |
| **Interruptible Audio** | ✅ Complete | 50ms chunks with interrupt flag |
| **TTS Routing** | ✅ Complete | Piper output now streams through HAL for unified audio |

### Key Architecture Decision

Instead of running two separate `aplay` processes (one for beeps/WAV, one for Piper TTS), **all audio now routes through a single persistent pipeline**:

- `hal_audio_write_raw()` - Used by beeps and TTS
- TTS (`hal_tts_piper.c`) runs `piper --output_raw` and streams PCM through the HAL
- This eliminates device conflicts and makes TTS interruptible

---

## Problem Summary

Testing of the original Set Mode Correction Plan (Phase 1 beeps) revealed:

1. **Latency**: Each `aplay` call in `hal_audio_play_file()` spawns a new process, adding ~100-200ms startup latency per beep
2. **Queue Overflow**: Rapid key presses filled the audio queue faster than playback could drain it
3. **Audio Blocking**: Queue overflow caused key responses to block, making the system unresponsive

## Proposed Solution (Simplified)

| Part | Description | Benefit |
|------|-------------|---------|
| **1. Persistent ALSA** | Keep ALSA process running, stream audio data | Eliminates process spawn latency |
| **2. RAM-Cached Beeps** | Preload beep WAV files into memory at init | Zero file I/O delay for beeps |
| **3. Interruptible Audio** | 50ms buffer with stop-on-keypress capability | Responsive interruption, no queue overflow |

> [!NOTE]
> **Removed**: The separate dmix beep channel is no longer needed. With interrupt support, beeps can simply interrupt current audio and play immediately.

---

## Phase 1: Persistent ALSA Pipeline for Audio HAL

> **Goal**: Replace `system("aplay ...")` with a persistent pipeline for pre-generated audio files.

### Chunk 1.1: Create Persistent Audio Pipeline Infrastructure

**File**: `Firmware/hal/hal_audio_usb.c`

**Changes**:
1. Add static `FILE* audio_pipe` for persistent `aplay` process
2. Create `start_audio_pipeline()` function that opens an `aplay` process in raw PCM mode:
   ```c
   // Pipeline command: aplay in raw mode, 16-bit mono 16kHz (matches Piper TTS output)
   audio_pipe = popen("aplay -D plughw:3,0 -r 16000 -f S16_LE -c 1 -t raw -q -", "w");
   ```
3. Modify `hal_audio_init()` to start the pipeline
4. Modify `hal_audio_cleanup()` to close the pipeline

**New Internal Functions**:
```c
static int start_audio_pipeline(void);
static void stop_audio_pipeline(void);
```

**Test**: Initialize audio HAL, verify pipeline starts without error, cleanup without crash.

**Estimated Time**: 1-2 hours

---

### Chunk 1.2: Implement WAV File Streaming

**File**: `Firmware/hal/hal_audio_usb.c`

**Changes**:
1. Create `stream_wav_to_pipeline()` internal function
2. Parse WAV header to get:
   - Sample rate (must be 16000 Hz to match Piper)
   - Channels (1 = mono)
   - Bits per sample (16)
3. Stream PCM data from file to pipeline in 50ms chunks (~800 samples at 16kHz mono)
4. Replace `hal_audio_play_file()` implementation to use streaming

**WAV Header Parsing** (simplified):
```c
typedef struct {
    char riff[4];        // "RIFF"
    uint32_t file_size;
    char wave[4];        // "WAVE"
    char fmt[4];         // "fmt "
    uint32_t fmt_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data[4];        // "data"
    uint32_t data_size;
} WavHeader;
```

**Test**: Play a known WAV file through the pipeline, verify audio output.

**Estimated Time**: 2 hours

---

### Chunk 1.3: Add Audio HAL Unit Tests

**File**: `Firmware/hal/tests/test_hal_audio.c` (new)

**Tests**:
1. `test_audio_init_cleanup` - Pipeline starts and stops cleanly
2. `test_audio_play_file` - WAV file plays without error
3. `test_audio_multiple_files` - Sequential files play without gaps or errors

**Build**:
```bash
cd Firmware/hal/tests
make test_hal_audio
./test_hal_audio
```

**Estimated Time**: 1 hour

---

## Phase 2: RAM-Cached Beeps

> **Goal**: Load beep files into memory at startup to eliminate file I/O during playback.

### Chunk 2.1: Create Beep Cache Structure

**File**: `Firmware/hal/hal_audio_usb.c`

**Changes**:
1. Define beep cache structure:
   ```c
   typedef struct {
       int16_t* samples;      // PCM data in memory
       size_t num_samples;    // Number of samples
       int sample_rate;       // Sample rate
       int channels;          // 1 or 2
   } CachedAudio;
   
   static CachedAudio beep_keypress = {0};
   static CachedAudio beep_hold = {0};
   static CachedAudio beep_error = {0};
   ```

2. Create `load_wav_to_cache()` function:
   ```c
   static int load_wav_to_cache(const char* filepath, CachedAudio* cache);
   static void free_cached_audio(CachedAudio* cache);
   ```

3. Modify `hal_audio_init()` to preload beep files:
   ```c
   load_wav_to_cache("pregen_audio/beep_keypress.wav", &beep_keypress);
   load_wav_to_cache("pregen_audio/beep_hold.wav", &beep_hold);
   load_wav_to_cache("pregen_audio/beep_error.wav", &beep_error);
   ```

**Test**: Verify beep files load into memory without error.

**Estimated Time**: 1-2 hours

---

### Chunk 2.2: Add Beep Playback API

**File**: `Firmware/hal/hal_audio.h` and `Firmware/hal/hal_audio_usb.c`

**New API**:
```c
typedef enum {
    BEEP_KEYPRESS,
    BEEP_HOLD,
    BEEP_ERROR
} BeepType;

// Play a beep from RAM cache (fast, no file I/O)
int hal_audio_play_beep(BeepType type);
```

**Implementation**:
```c
int hal_audio_play_beep(BeepType type) {
    CachedAudio* beep = NULL;
    switch (type) {
        case BEEP_KEYPRESS: beep = &beep_keypress; break;
        case BEEP_HOLD:     beep = &beep_hold; break;
        case BEEP_ERROR:    beep = &beep_error; break;
    }
    if (!beep || !beep->samples) return -1;
    
    // Write directly to audio pipeline from RAM
    return hal_audio_write_raw(beep->samples, beep->num_samples);
}
```

**Test**: Call `hal_audio_play_beep()`, verify beeps play immediately.

**Estimated Time**: 1 hour

---

## Phase 3: Interruptible Audio with 50ms Buffer

> **Goal**: Allow key presses to interrupt current audio playback without killing processes.

### Chunk 3.1: Add Interrupt Flag and Chunked Playback

**File**: `Firmware/hal/hal_audio_usb.c`

**Changes**:
1. Add interrupt flag:
   ```c
   static volatile int audio_interrupted = 0;
   static volatile int audio_playing = 0;
   ```

2. Modify `stream_wav_to_pipeline()` to check flag between 50ms chunks:
   ```c
   while (bytes_remaining > 0 && !audio_interrupted) {
       size_t chunk_size = MIN(CHUNK_SIZE, bytes_remaining);
       fwrite(chunk_ptr, 1, chunk_size, audio_pipe);
       chunk_ptr += chunk_size;
       bytes_remaining -= chunk_size;
   }
   audio_interrupted = 0;  // Reset for next playback
   ```

3. Add interrupt function:
   ```c
   void hal_audio_interrupt(void) {
       audio_interrupted = 1;
   }
   
   int hal_audio_is_playing(void) {
       return audio_playing;
   }
   ```

**Test**: Start long audio, call interrupt, verify stops within ~50ms.

**Estimated Time**: 1-2 hours

---

### Chunk 3.2: Wire Interrupt to Keypad Handler

**File**: `Firmware/keypad_firmware.c`

**Changes**:
1. When a key press is detected, call `hal_audio_interrupt()` before processing:
   ```c
   // In key detection loop
   if (new_key != last_key && new_key != '-') {
       hal_audio_interrupt();  // Stop any playing audio
       // ... rest of key handling
   }
   ```

**Test**: 
1. Start long TTS phrase
2. Press a key
3. Verify: Audio stops quickly → beep plays → response plays

**Estimated Time**: 30 minutes

---

### Chunk 3.3: Coordinate Piper TTS with Interrupt Flag

**File**: `Firmware/hal/hal_tts_piper.c`

**Changes**:
1. Piper keeps its own persistent `aplay` pipeline (no change to architecture)
2. Add interrupt awareness by checking `hal_audio_is_playing()` before speaking:
   ```c
   int hal_tts_speak(const char* text, const char* output_file) {
       // Wait briefly if beep is playing (optional, for cleaner audio)
       // Or just let aplay handle mixing
       
       fprintf(persistent_pipe, "%s\n", text);
       fflush(persistent_pipe);
       return 0;
   }
   ```

3. Optionally expose an interrupt function for TTS as well:
   ```c
   void hal_tts_interrupt(void);  // Close and reopen pipeline to stop speech
   ```

**TTS Interrupt Strategy**: Close and reopen the Piper pipe to stop speech. The ~100ms delay on next speech is acceptable and preferred over letting long sentences finish.

```c
void hal_tts_interrupt(void) {
    if (persistent_pipe) {
        pclose(persistent_pipe);
        persistent_pipe = NULL;
    }
    // Will be reopened on next hal_tts_speak() call
}
```

**Test**: Verify TTS still works correctly, and interrupt stops speech within ~100ms.

**Estimated Time**: 1 hour

---

## Phase 4: Integration and Testing

### Chunk 4.1: Create Integration Test Script

**File**: `Documentation/scripts/test_audio_latency.sh` (new)

**Tests**:
1. Measure beep latency (key press to audio start)
2. Verify interrupt behavior (current audio stops on key press)
3. Stress test: 10 rapid key presses, verify no queue overflow

**Target Metrics**:
- Beep latency: < 50ms
- Interrupt latency: < 100ms
- No crashes or hangs under rapid input

**Estimated Time**: 1 hour

---

### Chunk 4.2: Run Regression Tests

Run all existing regression tests:
```bash
./Documentation/scripts/Regression_Frequency_Mode.sh
./Documentation/scripts/Regression_Normal_Mode.sh
./Documentation/scripts/Regression_Phase0_Integration.sh
./Documentation/scripts/Regression_HAL_Integration.sh
```

**Acceptance Criteria**: All tests pass.

---

## Verification Checklist

### Phase 1: Persistent ALSA
- [ ] Pipeline starts on init without error
- [ ] WAV files stream through pipeline correctly
- [ ] Multiple sequential files play without gaps
- [ ] Unit tests pass

### Phase 2: RAM-Cached Beeps
- [ ] Beep files load into RAM at init
- [ ] `hal_audio_play_beep()` plays from RAM
- [ ] No file I/O during beep playback

### Phase 3: Interruptible Audio
- [ ] Interrupt flag stops file playback within 50ms
- [ ] Key press triggers interrupt before processing
- [ ] TTS continues to work correctly
- [ ] No orphaned processes after interrupts

### Phase 4: Integration
- [ ] Beep latency < 50ms
- [ ] No queue overflow with rapid key presses
- [ ] All regression tests pass

---

## Git Strategy

```bash
# Create feature branch
git checkout main
git pull origin main
git checkout -b set-mode-correction
```

### Commit Plan

| Chunk | Commit Message |
|-------|----------------|
| 1.1 | `firmware: Add persistent ALSA pipeline infrastructure` |
| 1.2 | `firmware: Implement WAV file streaming to pipeline` |
| 1.3 | `firmware: Add audio HAL unit tests` |
| 2.1 | `firmware: Add beep cache structure and loader` |
| 2.2 | `firmware: Add hal_audio_play_beep API` |
| 3.1 | `firmware: Add interruptible audio with 50ms buffer` |
| 3.2 | `firmware: Wire audio interrupt to keypad handler` |
| 3.3 | `firmware: Verify Piper TTS works with new audio HAL` |
| 4.1-4.2 | `test: Add audio latency tests and run regression suite` |

### Merge Strategy

```bash
# After all tests pass
git checkout main
git merge set-mode-correction
git push origin main
```

---

## Estimated Effort (Revised)

| Phase | Chunks | Estimated Time |
|-------|--------|----------------|
| Phase 1: Persistent ALSA | 3 | 4-5 hours |
| Phase 2: RAM-Cached Beeps | 2 | 2-3 hours |
| Phase 3: Interruptible Audio | 3 | 2.5-3.5 hours |
| Phase 4: Integration | 2 | 1-2 hours |
| **Total** | **10** | **9.5-13.5 hours** |

---

## Dependencies

| Dependency | Status | Notes |
|------------|--------|-------|
| ALSA dev libraries | Installed | `libasound2-dev` on RPi |
| sox | Installed | For generating beep WAV files |
| Piper TTS | Installed | Via `install_piper.sh` |
| USB Audio device | Required | Hardware dependency |
| USB Keypad | Required | Hardware dependency |
| Beep WAV files | To create | `beep_keypress.wav`, `beep_hold.wav`, `beep_error.wav` |
