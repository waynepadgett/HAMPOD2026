# Interrupt Fix and Persistent Piper Implementation Plan

> [!CAUTION]
> **STATUS: INTERRUPT NEEDS PHASE 3 - DIRECT ALSA**
> - Phase 1 (Interrupt Bypass): Implemented but insufficient (aplay buffering)
> - Phase 2 (Persistent Piper): ✅ Working - Latency improved
> - Phase 3 (Direct ALSA): Planned - will enable true interrupt
> - Branch: `feature/interrupt-and-persistent-piper`

---

## Problem Summary

Two critical issues in the HAMPOD audio system:

1. **Speech Cannot Be Interrupted** - Needs Phase 3
2. **Piper High Latency** - ✅ Resolved with persistent Piper

---

## Phase 2: Persistent Piper - COMPLETE ✅

Latency improvement is working. Piper runs as a persistent subprocess.

---

## Phase 1: Interrupt Bypass - IMPLEMENTED BUT INSUFFICIENT

### What We Tried

| Attempt | Result |
|---------|--------|
| IO Thread Bypass for 'i' packets | ✅ Interrupt IS received |
| Check `audio_interrupted` in `hal_audio_write_raw()` | ❌ Flag cleared by next TTS |
| Clear queue on interrupt | ❌ New packets arrive after clear |
| Synchronous interrupt ack | ❌ New TTS already in flight |

### Root Cause: `aplay` Buffering

We pipe audio to `aplay`, which has internal buffering. Even when we stop writing, `aplay` continues playing from its buffer. **We cannot interrupt audio already in aplay's buffer.**

---

## Phase 3: Direct ALSA Audio - PROPOSED SOLUTION

Replace the `popen("aplay ...")` pipeline with direct ALSA `snd_pcm_*` API calls. This gives us:

- **`snd_pcm_drop()`** - Immediately stops playback and discards buffer
- **`snd_pcm_prepare()`** - Resets for new audio
- **No subprocess** - One less thing to manage

### Benefits

1. True interrupt capability via `snd_pcm_drop()`
2. Slightly lower latency (no shell/aplay overhead)
3. Better error handling and recovery
4. Cleaner architecture

---

## User Review Required

> [!IMPORTANT]
> **Breaking Change**: This replaces the audio pipeline implementation. Extensive testing required before merge.

> [!WARNING]
> **Complexity**: Direct ALSA is more complex than popen/aplay. Error handling for ALSA quirks needed.

---

## Proposed Changes

### [MODIFY] [hal_audio_usb.c](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Firmware/hal/hal_audio_usb.c)

#### 1. Replace popen pipeline with ALSA PCM handle

```diff
+#include <alsa/asoundlib.h>

-static FILE *audio_pipe = NULL;
+static snd_pcm_t *pcm_handle = NULL;
+static snd_pcm_hw_params_t *hw_params = NULL;
```

#### 2. Replace `start_audio_pipeline()` with `open_pcm_device()`

```c
static int open_pcm_device(void) {
    int err;
    
    err = snd_pcm_open(&pcm_handle, audio_device, SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        fprintf(stderr, "HAL Audio: Cannot open device %s: %s\n", 
                audio_device, snd_strerror(err));
        return -1;
    }
    
    /* Allocate hardware parameters */
    snd_pcm_hw_params_malloc(&hw_params);
    snd_pcm_hw_params_any(pcm_handle, hw_params);
    
    /* Set parameters: 16kHz, mono, 16-bit signed little-endian */
    snd_pcm_hw_params_set_access(pcm_handle, hw_params,
                                  SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(pcm_handle, hw_params, AUDIO_CHANNELS);
    
    unsigned int rate = AUDIO_SAMPLE_RATE;
    snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &rate, NULL);
    
    /* Set buffer size for low latency (~100ms total, ~25ms period) */
    snd_pcm_uframes_t buffer_size = AUDIO_SAMPLE_RATE / 10;  /* 100ms */
    snd_pcm_uframes_t period_size = buffer_size / 4;          /* 25ms */
    snd_pcm_hw_params_set_buffer_size_near(pcm_handle, hw_params, &buffer_size);
    snd_pcm_hw_params_set_period_size_near(pcm_handle, hw_params, &period_size, NULL);
    
    /* Apply parameters */
    err = snd_pcm_hw_params(pcm_handle, hw_params);
    if (err < 0) {
        fprintf(stderr, "HAL Audio: Cannot set parameters: %s\n", 
                snd_strerror(err));
        snd_pcm_close(pcm_handle);
        pcm_handle = NULL;
        return -1;
    }
    
    snd_pcm_prepare(pcm_handle);
    printf("HAL Audio: ALSA PCM device opened (device=%s, rate=%d)\n",
           audio_device, rate);
    return 0;
}
```

#### 3. Modify `hal_audio_write_raw()` to use ALSA

```c
int hal_audio_write_raw(const int16_t *samples, size_t num_samples) {
    if (!initialized || pcm_handle == NULL) {
        fprintf(stderr, "HAL Audio: PCM device not available\n");
        return -1;
    }
    
    if (samples == NULL || num_samples == 0) {
        return 0;
    }
    
    /* Check interrupt flag */
    if (audio_interrupted) {
        return 0;  /* Drop audio - interrupted */
    }
    
    snd_pcm_sframes_t frames = snd_pcm_writei(pcm_handle, samples, num_samples);
    
    if (frames < 0) {
        /* Handle underrun or other errors */
        frames = snd_pcm_recover(pcm_handle, frames, 0);
        if (frames < 0) {
            fprintf(stderr, "HAL Audio: Write failed: %s\n", 
                    snd_strerror(frames));
            return -1;
        }
    }
    
    return 0;
}
```

#### 4. Implement true interrupt with `snd_pcm_drop()`

```c
void hal_audio_interrupt(void) {
    audio_interrupted = 1;
    
    /* Immediately stop playback and discard buffer */
    if (pcm_handle != NULL) {
        snd_pcm_drop(pcm_handle);
    }
}

void hal_audio_clear_interrupt(void) {
    audio_interrupted = 0;
    
    /* Prepare device for new audio */
    if (pcm_handle != NULL) {
        snd_pcm_prepare(pcm_handle);
    }
}
```

#### 5. Update cleanup

```c
void hal_audio_cleanup(void) {
    if (pcm_handle != NULL) {
        snd_pcm_drain(pcm_handle);
        snd_pcm_close(pcm_handle);
        pcm_handle = NULL;
    }
    if (hw_params != NULL) {
        snd_pcm_hw_params_free(hw_params);
        hw_params = NULL;
    }
    /* ... existing cleanup ... */
}
```

### [MODIFY] [Makefile](file:///Users/waynepadgett/Documents/developer/HAMPOD2026/Firmware/Makefile)

Add ALSA library to link flags (already present: `-lasound`).

---

## Verification Plan

### Automated Tests

#### Existing Tests (Regression)

```bash
cd ~/HAMPOD2026/Firmware/hal/tests
make clean && make
./test_hal_audio         # Verify basic audio still works
./test_interrupt_bypass  # Verify interrupt flag handling
./test_persistent_piper  # Verify TTS still works
```

#### New Test: Direct ALSA Interrupt

Add test case to `test_interrupt_bypass.c` that verifies:
1. Audio starts playing
2. `hal_audio_interrupt()` is called
3. Audio stops immediately (within 50ms)
4. `hal_audio_clear_interrupt()` + new audio plays

---

### Manual Testing

#### Test 1: Basic Audio Playback

**Steps**:
1. SSH to Pi: `ssh hampod@hampod.local`
2. Build and start HAMPOD: `cd ~/HAMPOD2026 && ./run_hampod.sh`
3. Press a key on the keypad
4. **Expected**: Beep + speech plays correctly

#### Test 2: Interrupt During Speech (Critical Test)

**Steps**:
1. HAMPOD running
2. Press a key that triggers frequency announcement (e.g., `[1]`)
3. While announcement is playing, quickly press another key (e.g., `[2]`)
4. **Expected**: First announcement STOPS, second key's response begins

**Pass Criteria**: Speech stops within ~100ms of second keypress

#### Test 3: Long Session Stability

**Steps**:
1. Leave HAMPOD running for 10+ minutes
2. Press keys periodically
3. Check memory usage: `ps aux | grep hampod`
4. **Expected**: Memory stable, no crashes

---

## Implementation Chunks

| Chunk | Description | Files | Risk |
|-------|-------------|-------|------|
| 3.1 | Add ALSA includes and PCM handle | `hal_audio_usb.c` | Low |
| 3.2 | Implement `open_pcm_device()` | `hal_audio_usb.c` | Medium |
| 3.3 | Replace `hal_audio_write_raw()` | `hal_audio_usb.c` | Medium |
| 3.4 | Implement `snd_pcm_drop()` interrupt | `hal_audio_usb.c` | Low |
| 3.5 | Update cleanup | `hal_audio_usb.c` | Low |
| 3.6 | Test and debug | Tests | Variable |

---

## Rollback Plan

If direct ALSA has issues:
1. Revert to `popen("aplay ...")` which is still in git history
2. Or use kill/restart aplay as fallback interrupt mechanism

---

## Questions for User

1. **Buffer size tradeoff**: Smaller buffer = faster interrupt but higher CPU/risk of underruns. Suggested 100ms total buffer, 25ms periods. Is this acceptable?

2. **Device selection**: Current code auto-detects USB speaker. Should we keep this behavior with direct ALSA?
