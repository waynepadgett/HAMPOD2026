> **Status:** 🔄 Partial — Phase 1 code complete, Phase 2 (warmup script) remaining
> **Last Updated:** 2026-06-21

# Piper TTS Performance Cache — Phased Implementation Plan

**Branch**: 
`feature/performance-cache` (Old - merged into main)  
`June2026` (new - not started yet)

**Date**: 2026-02-26  
**Goal**: Reduce TTS latency by ≥50% for repeated phrases via disk caching


## Scratch notes and TODOS

NOTE: pi to run the testing on is:

pi ssh cmd:

ssh hamdevpi5-2@hamdevpi5-2.local

and

ssh hamdevpi0-3@hamdevpi0-3.local


testing phase 1:
cd ~/HAMPOD2026/Firmware && ./hal/tests/test_tts_cache

to clear cache:
sudo rm -rf /root/.cache/hampod/tts/*


current TODO list:
- [ ] Write Documentation/scripts/warmup_tts_cache.sh (Phase 2)
   write a bash script that loops over the phrases listed in lines 142-148, calls piper for each, and pipes to the right cache path. Test it on the Pi. Done.
- [ ] test it manually
- [ ] mark this file complete, update docs overview, move this doc to completed. 


testing manually:
```
# 1. Clear cache so you're starting fresh
sudo rm -rf /root/.cache/hampod/tts/*

# 2. Run the warmup script
./Documentation/scripts/warmup_tts_cache.sh

# 3. Verify files were created
ls -la /root/.cache/hampod/tts/

# 4. Verify warm speak works
cd ~/HAMPOD2026/Firmware && ./hal/tests/test_tts_cache
```


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
**Disk limit**: 10GB max, hard cap (no eviction needed — cache won't fill in practice)

---

## Phase 1: Basic Disk Cache (MVP)

The minimal viable cache — store raw PCM to disk, play from disk on repeat.

### New Files

| File | Purpose |
|------|---------|
| `Firmware/hal/hal_tts_cache.h` | Public cache API |
| `Firmware/hal/hal_tts_cache.c` | Disk cache: init, lookup, store, clear |

### API (actual, verified against `hal_tts_cache.h`)

```c
int  hal_tts_cache_init(void);                    // mkdir -p cache dir
int  hal_tts_cache_lookup(const char *text,       // Check disk for cached PCM
                          int16_t **samples,
                          size_t *num_samples);
void hal_tts_cache_release(int16_t *samples);     // Free samples from lookup
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

- Max total cache size: **10GB** (configurable via `HAMPOD_TTS_CACHE_MAX_SIZE` env var or `DEFAULT_MAX_DISK_CACHE_SIZE` define)
- **Current behavior:** Hard cap — when full, `hal_tts_cache_store()` returns -1 with "Disk cache full" error. No automatic eviction.
- Size tracking: On init, scans the cache dir and sums file sizes; maintains a running total

### Test: `test_tts_cache.c`

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
- [x] Warm speak latency < 2000ms to completion (actual threshold in test_tts_cache.c)
- [x] `test_persistent_piper` still passes
- [x] Disk limit enforced with LRU eviction (current: hard cap, returns -1 when full)
(LRU not needed 10gb limit is massive and will never realisticlally fill)
> **Note**: Fixed a race condition where interrupting TTS could cause partial utterances to be saved to the cache.

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

~~Phase 3 (Compression) — SKIPPED: Cache is only ~736KB for 28 phrases. Not worth the complexity.~~






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


---

## Verification Summary

| Phase | Test Command | Success Criteria |
|-------|-------------|------------------|
| 1 | `make clean && make` | Compiles clean ✅ |
| 1 | `./hal/tests/test_tts_cache` | Warm < 2000ms to completion ✅ |
| 1 | `./hal/tests/test_persistent_piper` | Existing tests pass |
| 2 | `./Documentation/scripts/warmup_tts_cache.sh` | Cache populated (not yet created) |

