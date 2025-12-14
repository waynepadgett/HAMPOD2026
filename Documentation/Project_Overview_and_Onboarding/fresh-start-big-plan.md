# Fresh Start: Big Picture Implementation Plan

## Overview

This document prioritizes mode implementation order to minimize rework and overall effort. Each mode will need its own detailed plan (like `fresh-start-first-freq-mode.md` for Frequency Mode), but this establishes the sequence and dependencies.

### Detailed Plans

| Plan | Description | Status |
|------|-------------|--------|
| [Phase 0: Core Infrastructure](fresh-start-phase-zero-plan.md) | Pipe comm, speech, keypad, config modules | ⏳ In Progress |
| [Phase 1: Frequency Mode](fresh-start-first-freq-mode.md) | First working mode implementation | ⏳ Not Started |

### Prerequisites (COMPLETED)

The following Firmware foundation work is already complete and verified:

- ✅ **Firmware HAL**: USB keypad and audio working
- ✅ **Piper TTS Integration**: Compile-time switch between Piper and Festival
- ✅ **Pipe Protocol**: Firmware creates and manages named pipes
- ✅ **Integration Test**: `imitation_software` validates bidirectional communication

See: [Hampod_RPi_change_plan.md](../Hampod_RPi_change_plan.md) for full Firmware progress.

### Target Behavior

The goal is to implement all modes described in [ICOMReader_Manual_v106.txt](../Original_Hampod_Docs/ICOMReader_Manual_v106.txt):

| Mode | Purpose | Priority |
|------|---------|----------|
| Normal Mode | Query radio status, announce changes | Core |
| Frequency Entry Mode | Direct frequency input | High |
| Set Mode | Adjust radio parameters | Medium |
| Memory Mode | Store/recall frequencies | Medium |
| DTMF Mode | Generate touch tones | Low |
| DStar Mode | DStar-specific features | Low (radio-specific) |
| Configuration Mode | HamPod settings | Required |

### New Features (Beyond Original HAMPOD)

| Feature | Purpose |
|---------|---------|
| Radio Selection | Choose/detect radio model (not just ICOM) |
| Speech Synthesizer Selection | Switch between Festival/Piper at runtime |

---

## Reuse from Old Code (2025 Team)

Before rewriting everything, copy/adapt these working pieces:

| Component | Source | Notes |
|-----------|--------|-------|
| Hamlib wrappers | `HamlibWrappedFunctions/` | `get_level()`, `set_func()`, `get_current_frequency()` etc. are well-structured |
| KeyPress struct | `KeyWatching.h` | Clean design: `{keyPressed, shiftAmount, isHold}` |
| Audio file caching | `FirmwareCommunication.c` | HashMap of pre-generated audio reduces TTS latency |
| Dictionary substitution | `FirmwareCommunication.c` | Word replacement before speaking (abbreviations → full words) |


## Recommended Implementation Order

### Phase 0: Core Infrastructure (Prerequisites for ALL modes)
Build these foundation components FIRST - they are used by everything:

1. **Pipe Communication Module** (`comm.c`)
   - Wraps Firmware pipe protocol
   - Handles packet creation/parsing
   - Used by speech, keypad, and all modes

2. **Speech Module** (`speech.c`)
   - Festival TTS (default)
   - **Key design decision:** Separate thread for audio playback
   - Non-blocking queue-based design (see Architecture Notes)
   - **Audio caching:** HashMap of pre-generated WAV files for common phrases
   - **Dictionary substitution:** Replace abbreviations/words before speaking

3. **Keypad Module** (`keypad.c`)
   - Blocking read from Firmware
   - Key beep support (see Architecture Notes)
   - **Reuse `KeyPress` struct:** `{keyPressed, shiftAmount, isHold}` from old code

4. **Radio Module** (`radio.c`)
   - Hamlib wrapper
   - Radio-agnostic API
   - **Design for polling** (see Architecture Notes)

5. **Configuration Storage** (`config.c`)
   - Save/load settings to file
   - Radio model, speech settings, preferences

**Why first?** Every mode depends on these. Get them right once.

---

### Phase 1: Minimal Viable Product
*Goal: Demonstrate working end-to-end flow*

1. **Configuration Mode** (basic)
   - Volume, speech rate, key beep toggle
   - Radio model selection
   - Speech synthesizer selection

2. **Frequency Entry Mode**
   - Already planned in `fresh-start-first-freq-mode.md`
   - Validates entire pipe communication path

**Checkpoint:** User can set radio model, enter frequency, hear confirmation.

---

### Phase 2: Normal Mode (Core Functionality)
*Goal: Passive monitoring and manual queries*

> [!IMPORTANT]
> The old `NormalMode.c` was 1016 lines with a massive switch statement. Break this into smaller handler groups to keep each file ~100-200 lines.

**Recommended structure:**
```
normal_mode/
  ├── normal_mode.c      # Main dispatcher, mode state
  ├── vfo_handlers.c     # VFO A/B/C selection and frequency queries
  ├── meter_handlers.c   # S-meter, SWR, power readings
  ├── level_handlers.c   # AGC, squelch, NR, NB, compression
  └── func_handlers.c    # VOX, RIT, XIT, PTT toggles
```

1. **Radio Polling Thread** (see Architecture)
   - Background thread polls radio state
   - Detects frequency/mode/VFO changes
   - Posts events to announcement queue

2. **Query Functions** (Normal Mode keys)
   - `[2]` - Current frequency
   - `[0]` - Current mode (LSB/USB/CW/etc.)
   - `[*]` - S-meter
   - `[1]` / `[1]` Hold - Select VFO A/B

3. **Automatic Announcements**
   - Frequency change (with configurable delay)
   - Mode change
   - VFO change

**Checkpoint:** User can query radio, hear automatic announcements on changes.

---

### Phase 3: Set Mode
*Goal: Adjust radio parameters from keypad*

- Power level, Mic gain, Compression
- Filter width, AGC settings
- Noise blanker/reduction

**Checkpoint:** User can adjust radio settings via keypad.

---

### Phase 4: Memory Mode
*Goal: Store and recall frequency presets*

- Save current VFO to memory slot
- Recall memory to VFO
- Announce stored frequency/mode

**Checkpoint:** User can save/recall frequency presets.

---

### Phase 5: Optional/Low Priority

- **DTMF Mode** - Generate touch tones through speaker
- **DStar Mode** - Only if user has DStar-capable radio

---

### Firmware Modifications: When to Do Them

The Firmware is currently working and should be modified only when necessary. Here's when:

**Phase 0 (Optional - for zero-lag key beeps):**
- Add key beep playback in Firmware before sending key to Software
- Do this during Phase 0.3 (Keypad Module) if you want zero-lag beeps
- Otherwise, handle beeps in Software (acceptable ~50ms lag)

**No changes needed for:**
- Frequency Mode
- Normal Mode queries
- Set Mode
- Memory Mode

**Defer unless blocking:**
- If you encounter a Firmware bug, fix it then
- Don't proactively refactor working Firmware code


## Architecture Recommendations

### Problem 1: Detecting Radio Changes

**The Challenge:**
When user turns VFO dial on radio, software should announce the new frequency. But software doesn't receive events from radio - it must poll.

**Recommended Solution: Dedicated Polling Thread**

```
┌─────────────────────────────────────────────────────────┐
│                    MAIN THREAD                           │
│  ┌─────────────┐                                        │
│  │ Keypad Loop │ ──► Mode State Machines                │
│  └─────────────┘                                        │
└────────────────────────────────────────────────────────┘
         │                                     ▲
         │ keypress events                     │ "announce X"
         ▼                                     │
┌─────────────────────────────────────────────────────────┐
│                  ANNOUNCEMENT QUEUE                      │
│  (Thread-safe queue of speech items)                    │
└─────────────────────────────────────────────────────────┘
         ▲                                     │
         │ "frequency changed"                 │
         │                                     ▼
┌─────────────────────────────────────────────────────────┐
│                  POLLING THREAD                          │
│  - Poll radio every 100-200ms                           │
│  - Compare to last known state                          │
│  - If changed: post announcement to queue               │
│  - Configurable delay before announcing                 │
└─────────────────────────────────────────────────────────┘
```

**Key design points:**
- Polling thread runs independently of keypad handling
- Changes detected by comparing current state to cache
- "Frequency Announcement Delay" = wait for dial to stop (configurable 1-5 seconds)
- Thread-safe queue for announcements

---

### Problem 2: Key Beeps with Minimal Lag

**The Challenge:**
When user presses key, beep should be immediate even if speech is in progress.

**Recommended Solution: Pre-generated WAV + Audio Priority**

1. **Pre-generate beep.wav** - 50ms beep sound file
2. **Separate audio channels or interrupt:**
   - Option A: Play beep on separate audio stream (if ALSA supports mixing)
   - Option B: Immediately interrupt speech, play beep, resume speech
   - Option C: Play beep at Firmware level before sending to Software

**Simplest approach for v1:**
- Have Firmware handle key beeps directly (before sending key to Software)
- This requires small Firmware modification but gives zero-lag beeps

**Alternative (pure Software):**
- Use `aplay` with `--process-id-file` to track playing audio
- Kill playing audio, play beep, restart speech
- Acceptable latency: ~50ms

> [!NOTE]
> **RPi5 + Debian Trixie:** ALSA's `dmix` plugin supports simultaneous audio playback. Configure `/etc/asound.conf` or `~/.asoundrc` with a dmix PCM device, and two `aplay` processes can run concurrently (beep + speech mix together). This makes **Option A the cleanest solution** - no Firmware changes, no interrupt/resume logic needed.

---

### Problem 3: Speech Synthesizer Selection (Festival vs Piper)

**Festival:**
- Pros: Already working, simple, low CPU
- Cons: Robotic voice

**Piper:**
- Pros: More natural voice, local (no internet)
- Cons: Higher CPU, 200-500ms latency per phrase

**Recommended Approach:**
1. Abstract speech in `speech.c` with backend selection
2. Start with Festival (known working)
3. Add Piper backend later
4. Config option to switch

```c
// speech.h
typedef enum { SPEECH_FESTIVAL, SPEECH_PIPER } SpeechBackend;
void speech_set_backend(SpeechBackend backend);
void speech_say(const char* text);  // Uses selected backend
```

**Piper latency mitigation:**
- Pre-generate common phrases (digits, "megahertz", "frequency", etc.)
- Only use TTS for dynamic content
- Cache recently spoken phrases

---

### Problem 4: Radio Selection

**Options:**

1. **Manual Selection (Config Mode)**
   - User selects radio model from menu
   - Stored in config file
   - Simple, reliable

2. **Auto-Detection**
   - Query `/dev/ttyUSB*` devices
   - Try Hamlib probe on each
   - May be slow, may find wrong device

**Recommended: Manual with optional auto-detect**
- Default: Manual selection in Configuration Mode
- Optional: "Auto-detect" menu item that scans and suggests
- Store last successful radio model

---

## Implementation Dependency Graph

```
                    ┌─────────────────┐
                    │ Pipe Comm (0.1) │
                    └────────┬────────┘
              ┌──────────────┼──────────────┐
              ▼              ▼              ▼
      ┌───────────┐  ┌───────────┐  ┌───────────┐
      │ Speech    │  │ Keypad    │  │ Radio     │
      │ (0.2)     │  │ (0.3)     │  │ (0.4)     │
      └─────┬─────┘  └─────┬─────┘  └─────┬─────┘
            │              │              │
            └──────────────┴──────────────┘
                           │
              ┌────────────┴────────────┐
              ▼                         ▼
      ┌───────────────┐        ┌───────────────┐
      │ Config Mode   │        │ Frequency     │
      │ (1.1)         │        │ Mode (1.2)    │
      └───────────────┘        └───────────────┘
                                      │
                           ┌──────────┴──────────┐
                           ▼                     ▼
                   ┌───────────────┐     ┌───────────────┐
                   │ Normal Mode   │     │ Set Mode      │
                   │ (2.x)         │     │ (3.x)         │
                   └───────────────┘     └───────────────┘
                           │
                           ▼
                   ┌───────────────┐
                   │ Memory Mode   │
                   │ (4.x)         │
                   └───────────────┘
```

---

## Summary: Implementation Order

| Order | Component | Depends On | Effort |
|-------|-----------|------------|--------|
| 0.1 | Pipe Communication | Firmware | 2 hours |
| 0.2 | Speech Module | Pipe Comm | 3 hours |
| 0.3 | Keypad Module | Pipe Comm | 2 hours |
| 0.4 | Radio Module (Hamlib) | - | 3 hours |
| 0.5 | Config Storage | - | 2 hours |
| 1.1 | Configuration Mode | All 0.x | 3 hours |
| 1.2 | Frequency Mode | Speech, Keypad, Radio | 4 hours |
| 2.1 | Polling Thread | Radio | 3 hours |
| 2.2 | Normal Mode Queries | Polling, Speech | 4 hours |
| 2.3 | Auto Announcements | Polling, Speech | 2 hours |
| 3.x | Set Mode | Normal Mode | 4 hours |
| 4.x | Memory Mode | Set Mode | 4 hours |

**Total estimated: ~36 hours** (over multiple sessions)

---

## Next Steps

1. Review and approve this big-picture plan
2. Complete Phase 0 infrastructure (one component at a time)
3. Create detailed plans for each mode as you reach them
