# Piper TTS Performance Cache — Phased Implementation Plan

**Branch**: `feature/performance-cache`  
**Date**: 2026-02-26  
**Goal**: Reduce TTS latency by ≥50% for repeated phrases via disk caching

NOTE: pi to run the testing on is:

ssh hamdevpi0@hamdevpi0.local

testing phase 1:

cd ~/HAMPOD2026/Firmware && ./hal/tests/test_tts_cache

---

## Architecture Overview

```
┌─────────────────────────────────────────────────┐
│            hal_tts_speak("hello")                │
│                                                  │
│  ┌──────────────┐    miss                       │
│  │  Disk Cache  │──────────> Piper TTS          │
│  │  (10GB max)  │           ~3-4s (cold)        │
│  │  <1ms        │                │              │
│  └──────┬───────┘                │ save         │
│         │ hit                    ▼              │
│         ▼                   record PCM → disk   │
│    load + play via write_raw()                  │
└─────────────────────────────────────────────────┘
```

**Hashing**: DJB2 hash of input text → `%08x.pcm` filename  
**Cache dir**: `~/.cache/hampod/tts/` (overridable via `HAMPOD_TTS_CACHE_DIR` env var)  
**Disk limit**: 10GB max, LRU eviction when full

---

## Phase 1: Basic Disk Cache (MVP)

The minimal viable cache — store raw PCM to disk, play from disk on repeat.

### New Files

| File | Purpose |
|------|---------|
| `Firmware/hal/hal_tts_cache.h` | Public cache API |
| `Firmware/hal/hal_tts_cache.c` | Disk cache: init, lookup, store, eviction |

### API

```c
int  hal_tts_cache_init(void);                    // mkdir -p cache dir
int  hal_tts_cache_lookup(const char *text,       // Check disk for cached PCM
                          int16_t **samples,
                          size_t *num_samples);
int  hal_tts_cache_store(const char *text,        // Save PCM to disk
                         const int16_t *samples,
                         size_t num_samples);
void hal_tts_cache_cleanup(void);
int  hal_tts_cache_clear(void);                   // rm -rf cache contents
```

### Changes to Existing Files

- **`hal_tts_piper.c`**: Modify `hal_tts_speak()` to check cache before synthesizing. On miss, capture all PCM chunks into a buffer during synthesis, then `cache_store()` after playback.
- **`makefile`**: Add `hal/hal_tts_cache.c` to `HAL_SRCS` (piper builds only)

### Disk Limits

- Max total cache size: **10GB** (configurable via `#define`)
- Eviction: When storing a new entry would exceed the limit, delete the **oldest-accessed** `.pcm` files until there's room
- Size tracking: On init, scan the cache dir and sum file sizes; maintain a running total

### Test: `test_tts_cache_phase1.c`

| Test | Expected |
|------|----------|
| Speak "hello" (cold) | ~190-210ms, cache file created |
| Speak "hello" (warm) | **<10ms**, played from disk |
| Cache file exists | `~/.cache/hampod/tts/<hash>.pcm` present |
| Clear cache | File deleted, next speak is cold again |
| Existing Piper tests | Still pass (cache is transparent) |

```bash
# Build & run on Pi
cd ~/HAMPOD2026/Firmware
make clean && make
# Build test
cc -Wall -DUSE_PIPER hal/hal_tts_cache.c hal/hal_tts_piper.c \
   hal/hal_audio_usb.c hal/hal_usb_util.c hal/tests/test_tts_cache.c \
   -o hal/tests/test_tts_cache -lpthread -lasound
./hal/tests/test_tts_cache
```

### ✅ Phase 1 Done When
- [x] `make` compiles cleanly
- [x] Cold speak → cache file written
- [x] Warm speak latency < 1ms (to first chunk)
- [x] `test_persistent_piper` still passes
- [x] Disk limit enforced (tested with small limit)


---

## Phase 2: Cache Warmup Script

Pre-populate the cache with common HAMPOD phrases so they're instant from first boot.

### New File

| File | Purpose |
|------|---------|
| `Documentation/scripts/warmup_tts_cache.sh` | Generate cache entries for common phrases |

### Phrases to Pre-warm

Derived from the existing `pregen_audio/` directory contents:
- Digits: "0" through "9"
- Letters: "A", "B", "C", "D"
- Special: "point", "pound"
- Status: "on", "off", "out of range", "frequency"
- Menu: "Starting normal operations", "Select save file to load up", etc.
- Mode: "Config", "Normal", "DTMF", "Monitoring", "frequency mode"

### How It Works

```bash
#!/bin/bash
# For each phrase: echo "phrase" | piper --model ... --output_raw > ~/.cache/hampod/tts/<hash>.pcm
```

The hash must match the DJB2 hash used in C code. The script will include a small C helper or use the same algorithm in shell/python.

### ✅ Phase 2 Done When
- [ ] Script runs without errors
- [ ] Cache dir contains entries for all common phrases
- [ ] Firmware starts up and speaks cached phrases instantly

---

## Phase 3: Compression Optimization

Compress cached PCM files to reduce disk usage. Since the audio is speech (not music) and clarity matters more than fidelity, lossy compression is ideal.

### Approach: μ-law / A-law Compression (Recommended)

| Method | Compression | Quality | CPU Cost | Complexity |
|--------|-------------|---------|----------|------------|
| **μ-law (G.711)** | **2:1** (16-bit → 8-bit) | Excellent for speech | Near-zero | Trivial |
| ADPCM (IMA) | ~4:1 | Good | Very low | Moderate |
| Opus | ~10:1+ | Excellent | Moderate | External lib |
| MP3/Vorbis | ~10:1 | Good | Moderate | External lib |

**Recommended: μ-law encoding**
- **2:1 compression** with essentially no quality loss for speech
- Encoding/decoding is a single lookup table (256 bytes) — negligible CPU
- No external dependencies
- Well-established standard for telephone-quality speech (which is exactly our use case)
- Keeps sub-1ms disk playback target achievable (decode is ~1 CPU cycle per sample)

### Implementation

- **Store**: After Piper generates raw PCM, encode 16-bit samples → 8-bit μ-law, save to disk
- **Load**: Read compressed from disk, decode on-the-fly during `write_raw()` chunks
- **Playback**: Decode μ-law → 16-bit PCM per chunk (negligible CPU cost)
- File extension: `.ulaw` instead of `.pcm`

### Alternative: IMA ADPCM (if more compression is needed)

If 2:1 isn't enough:
- ~4:1 compression ratio
- Slightly more complex but still no external dependencies
- ~10 CPU cycles per sample decode
- Could be added as a future option

### ✅ Phase 3 Done When
- [ ] Cached files are μ-law encoded (half the size)
- [ ] Playback quality is clear and intelligible
- [ ] No measurable latency increase from decode

---

## Overclocking Assessment

With caching in place, aggressive overclocking provides **diminishing returns**:

| Scenario | Without Cache | With Cache |
|----------|--------------|------------|
| First novel phrase | ~3-4s | ~3-4s (same — must synthesize) |
| Repeated phrase | ~3-4s | **<1ms** (disk cache) |
| OC benefit | ~15% faster TTS | Only helps novel phrases |

**Recommendation**: Keep the existing conservative 1.5GHz OC on the Pi 3B+. Don't push further — the cache handles the latency problem far more effectively than hardware tweaks.

---

## File Summary

| Phase | File | Action |
|-------|------|--------|
| 1 | `Firmware/hal/hal_tts_cache.h` | NEW ✅ |
| 1 | `Firmware/hal/hal_tts_cache.c` | NEW ✅ |
| 1 | `Firmware/hal/hal_tts_piper.c` | MODIFY ✅ |
| 1 | `Firmware/makefile` | MODIFY ✅ |
| 1 | `Firmware/hal/tests/test_tts_cache.c` | NEW ✅ |
| 2 | `Documentation/scripts/warmup_tts_cache.sh` | NEW |
| 3 | `Firmware/hal/hal_tts_cache.c` | MODIFY (add μ-law) |

---

## Verification Summary

| Phase | Test Command | Success Criteria |
|-------|-------------|------------------|
| 1 | `make clean && make` | Compiles clean ✅ |
| 1 | `./hal/tests/test_tts_cache` | Warm < 1ms to first chunk ✅ |
| 1 | `./hal/tests/test_persistent_piper` | Existing tests pass |
| 2 | `./Documentation/scripts/warmup_tts_cache.sh` | Cache populated |
| 3 | `du -sh ~/.cache/hampod/tts/` | ~50% smaller |
