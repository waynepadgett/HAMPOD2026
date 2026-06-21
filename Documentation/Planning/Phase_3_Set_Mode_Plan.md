> **Status:** 🔄 In Progress (implementation done, testing + frequency format remain)
> **Last Updated:** 2026-06-21
> **Source Merge:** Consolidated from `Phase_3_Set_Mode_Plan.md` + `Set_Mode_Correction_Plan.md`
> **Set_Mode_Correction_Plan.md:** Archived to `Planning/Completed/` — all content merged here.

# Phase 3: Set Mode Implementation Plan

## Overview

Set Mode lets users adjust radio parameters from the keypad. Based on
[ICOMReader_Manual_v106.txt](../Original_Hampod_Docs/ICOMReader_Manual_v106.txt) section 3.

### What Set Mode Is

- **Toggle mode** — press `[B]` to enter/exit; announces "Set" / "Set Off"
- **Parameter selection** — press a key (tap or hold, with optional `[A]` shift) to
  select a parameter; hears current value
- **Value editing** — keypad digits enter a numeric value; `[A]`/`[B]` increment/decrement;
  `[C]` toggles sub-parameters (e.g., VOX gain vs anti-gain)
- **Confirm** — `[#]` sends value to radio and exits Set Mode
- **Cancel** — `[D]` cancels edit (back to parameter selection); `[D]` from idle
  or `[*]` at any time exits Set Mode

### Prerequisites (Complete)

- ✅ Phase 0: Core Infrastructure (comm, speech, keypad, config)
- ✅ Phase 1: Frequency Mode (radio module, frequency entry)
- ✅ Phase 2: Normal Mode (query functions, auto announcements)

---

## Spec Compliance

Discrepancies between the ICOMReader manual specification and the current
implementation, as verified against source code:

| Area | Spec Says | Implementation | Status |
|------|-----------|----------------|--------|
| Key beep on press | Short beep when key pressed (configurable) | `keypad.c:84` sends IPC beep | ✅ |
| Hold indicator beep | Lower-pitch beep at 500ms | `keypad.c:81` sends IPC beep | ✅ |
| Error beep | Low-frequency beep on invalid key | `comm_play_beep()` wired in all modes | ✅ |
| Set Mode announcement | Says "Set" | `set_mode.c:454` — matches spec | ✅ |
| `[*]` in Set Mode | Cancel/exit Set Mode | `set_mode.c:651` — calls `set_mode_exit()` | ✅ |
| Frequency announcement | "dot" separator for sub-kHz digits | Always uses "point" | ❌ Needs fix |

---

## Key Bindings

### Core Set Mode Functions

| Key Sequence | Query (Normal Mode) | Set (Set Mode Active) |
|--------------|---------------------|------------------------|
| `[B]` | Enter Set Mode | Exit Set Mode / Toggle |
| `[9]` Hold | Power Level query | Power Level **set** |
| `[8]` Hold | Mic Gain query | Mic Gain **set** |
| `[Shift]+[9]` | Compression query | Compression **set** |
| `[7]` | Noise Blanker query | Noise Blanker **set** |
| `[8]` | Noise Reduction query | Noise Reduction **set** |
| `[4]` | PreAmp query | PreAmp **set** |
| `[Shift]+[4]` | Attenuation query | Attenuation **set** |
| `[4]` Hold | AGC query | AGC speed **set** |
| `[6]` | Filter Width query | Filter Width **set** |
| `[0]` | Mode query | Mode **cycle** |

### Bonus Parameters (Beyond Original Plan)

| Key Sequence | Set Parameter | Added In |
|-------------|---------------|----------|
| `[Shift]+[1]` | VOX (gain/anti-gain) | Phase 3 implementation |
| `[Shift]+[2]` | Tuning Step | Phase 3 implementation |
| `[Shift]+[6]` | Filter Number | Phase 3 implementation |
| `[Shift]+[8]` | Keyer Speed | Phase 3 implementation |

### Set Mode Control Keys

| Key | Function |
|-----|----------|
| `[A]` | Increment value / Enable |
| `[B]` | Decrement value / Disable / Exit Set Mode |
| `[C]` | Toggle sub-parameter (e.g., VOX gain vs anti-gain) |
| `[#]` | Confirm and send to radio (also exits Set Mode) |
| `[D]` | Cancel edit → back to IDLE; from IDLE → exit Set Mode |
| `[*]` | Cancel and exit Set Mode (per spec) |
| `[0]-[9]` | Enter numeric value |

---

## Architecture

### Code Structure

```
Software2/
├── src/
│   ├── set_mode.c            # Main Set Mode dispatcher and state (714 lines)
│   ├── radio_setters.c       # Hamlib set functions (781 lines, 22+ functions)
├── include/
│   ├── set_mode.h            # Set Mode API (134 lines, 10 functions)
│   └── radio_setters.h       # Setter function declarations (203 lines)
└── tests/
    └── test_set_mode.c       # ❌ Not yet created
```

### Set Mode State Machine

**States:**

```
SET_MODE_OFF       — Not in Set Mode (Normal Mode active)
SET_MODE_IDLE      — In Set Mode, awaiting parameter selection
SET_MODE_EDITING   — Editing a specific parameter value
```

**Note:** `SET_MODE_CONFIRM` was proposed in the original design but is
unused — the actual code jumps directly from EDITING to `apply_value()`
which calls `set_mode_exit()` (→ OFF) or to `set_mode_cancel_edit()` (→ IDLE).

**Transitions (actual, verified against set_mode.c):**

```
  set_mode_enter()          → OFF → IDLE
  select_parameter()        → IDLE → EDITING
  set_mode_cancel_edit()    → EDITING → IDLE
  set_mode_exit()           → anything → OFF
  apply_value()             → EDITING → calls set_mode_exit() → OFF
  [0-9] digit entry         → stays in EDITING
  [#] with no value         → EDITING → OFF
  [D] in EDITING            → EDITING → IDLE
  [D] in IDLE               → IDLE → OFF
  [*] at any state          → anything → OFF
  [B] in IDLE               → IDLE → OFF
  [B] in OFF                → OFF → IDLE
```

### Public API (`set_mode.h`)

```c
// Initialization
void set_mode_init(void);

// State queries
bool set_mode_is_active(void);
SetModeState set_mode_get_state(void);
SetModeParameter set_mode_get_parameter(void);    // extra, not in original plan

// Key handler — returns true if key consumed
bool set_mode_handle_key(char key, bool is_hold, bool is_shifted);

// Entry/exit/edit
void set_mode_enter(void);
void set_mode_exit(void);
void set_mode_cancel_edit(void);                  // extra, not in original plan
void set_mode_clear_value(void);                  // extra, not in original plan
const char *set_mode_get_value_buffer(void);      // extra, not in original plan
```

### Key Routing Priority (Actual, from `main.c:on_keypress`)

```
1. Config Mode                 ← highest priority (settings/shutdown)
2. Set Mode                    ← if set_mode_is_active()
3. [A] shift toggle            ← toggles g_shift_active (only when not in Set Mode)
4. [B] enter Set Mode          ← set_mode_enter() when not active
5. Frequency Mode              ← frequency_mode_handle_key()
6. Normal Mode                 ← normal_mode_handle_key() with is_shifted, in_set_mode
7. Fallthrough                 ← "Pressed X" / "Held X" announcement
```

**Shift state:** `g_shift_active` is declared in `main.c`, toggled by `[A]`,
and auto-cleared (one-shot) after any consumed key. Passed as `is_shifted`
to `set_mode_handle_key()` and `normal_mode_handle_key()`.

### Radio Setters API (`radio_setters.h`, corrected)

All 9 planned functions exist, plus 13+ additional getters. Key signatures:

```c
// Power and gain levels (0-100 normalized)
int radio_set_power(int level);
int radio_set_mic_gain(int level);
int radio_set_compression(int level);

// Noise controls (toggle + level)
int radio_set_nb(bool enabled, int level);
int radio_set_nr(bool enabled, int level);

// AGC — corrected: speed only, no level parameter
typedef enum { AGC_OFF, AGC_FAST, AGC_MEDIUM, AGC_SLOW } AgcSpeed;
int radio_set_agc_speed(AgcSpeed speed);

// Preamp and attenuation
int radio_set_preamp(int state);           // 0=off, 1=preamp1, 2=preamp2
int radio_set_attenuation(int db);

// Mode cycling
int radio_cycle_mode(void);
```

---

## Completed Work

### Phase 3 Implementation (12-Chunk Plan)

| # | Chunk | Status | Details |
|---|-------|--------|---------|
| 1 | Documentation & Branch | ✅ | Plan created, branch set up |
| 2 | Radio Setters Module | ✅ | 22+ functions in `radio_setters.c` (9 planned + extra getters + bonus params) |
| 3 | Set Mode Skeleton | ✅ | 10 functions in `set_mode.c` (6 planned + 4 extras) |
| 4 | Power Level Setting | ✅ | `[9]` Hold → power, `apply_value()` → `radio_set_power()` |
| 5 | Mic Gain and Compression | ✅ | `[8]` Hold → mic gain, `[Shift]+[9]` → compression |
| 6 | Noise Blanker and NR | ✅ | `[7]` → NB, `[8]` → NR, toggle with `[A]`/`[B]` |
| 7 | AGC Settings | ✅ | `[4]` Hold → AGC, speed via `[1]/[2]/[3]` Hold |
| 8 | Preamp and Attenuation | ✅ | `[4]` → preamp, `[Shift]+[4]` → attenuation |
| 9 | Main Integration | ✅ | Includes Config Mode routing (plan omitted it), [A] shift auto-clear, [B] entry handler |
| **Bonus** | Tuning Step, VOX, Filter Number, Keyer Speed | ✅ | Added during implementation beyond original scope |

### Set Mode Corrections

| Phase | Scope | Status | Details |
|-------|-------|--------|---------|
| 1 | Audio Feedback System | ✅ | Beep WAV files in `Firmware/pregen_audio/`, `hal_audio_play_beep()` API, IPC bypass in `audio_firmware.c`, `keypad.c` beep triggers |
| 2 | Key Beep Integration | ✅ | `CommBeepType` enum, `comm_play_beep()` in all modes (8 calls in `set_mode.c`, 3 in `frequency_mode.c`, 3 in `config_mode.c`, 2 in `keypad.c`), guarded by `config_get_key_beep_enabled()` |
| 3.1 | `[*]` Key Behavior Verified | ✅ | `set_mode.c:651` calls `set_mode_exit()` — matches spec |
| 3.2 | `[B]` Cycling Verified | ✅ | On/Off toggle confirmed; toggle-params (NB, NR, Compression, VOX) in editing state also disable — documented as intentional |

---

## Remaining Work

### 1. `test_set_mode.c` Unit Tests

**Estimate:** ~15 min

Create `Software2/tests/test_set_mode.c` covering:
- `[B]` enters Set Mode (OFF → IDLE)
- `[B]` again exits Set Mode (IDLE → OFF)
- `[*]` cancels/exits from IDLE and EDITING
- `[D]` cancels edit (EDITING → IDLE)
- `[D]` exits from IDLE (IDLE → OFF)
- `[#]` confirms value (EDITING → calls apply_value → OFF)
- Digit entry accumulates value buffer
- `[A]` increments, `[B]` decrements in EDITING

**Run:**
```bash
cd Software2 && make tests && ./bin/test_set_mode
```

### 2. Frequency Announcement Format ("dot" vs "point")

**Estimate:** ~1 hour

**Files:** `Software2/src/normal_mode.c` and `Software2/src/frequency_mode.c`

**Current:** Both `announce_frequency()` functions always use "point"
(`normal_mode.c:66`, `frequency_mode.c:104`).

**Required:** Per spec, speak digits down to 1 kHz with "point", then
"dot" + remaining digits for sub-kHz. Omit sub-kHz if all zeros.

**Examples:**
- `14.250000` → "fourteen point two five zero"
- `14.250500` → "fourteen point two five zero dot five"

### 3. Set Mode Integration Test Script

**Estimate:** ~15 min

Create `Documentation/scripts/Regression_Set_Mode.sh` — build, run unit
tests, report pass/fail.

**Note:** `Documentation/scripts/Regression_Phase_Three_Manual_Test.sh`
currently exists but tests **Configuration Mode** (not Set Mode). Either
rename it to reflect Config Mode scope, or create a separate Set Mode
manual test script.

### 4. Final Regression & Merge

**Estimate:** ~20 min

Run all regression tests, then merge.

```bash
git checkout main
git merge <current-branch> --no-ff
git push
```

### 5. Update `Currently_Implemented_Keys.md` *(separate commit)*

The audit found this file has stale content that should be fixed, but it
is outside the scope of Set Mode plan consolidation:

| Item | Stale Content | Correction Needed |
|------|--------------|-------------------|
| Line 100 | `[B]` says "Set Mode" | Code says "Set" |
| Line 101 | `[B]` exit says "Set Mode Off" | Code says "Set Off" |
| Line 124 | `[*]` says "Clear Value" | `[*]` exits Set Mode entirely |
| Line 204 | Audio beeps status 🟡 Partial | Should be 🟢 Complete |
| Line 5 | Last Updated: 2025-12-29 | Bump date |

---

## Git Strategy

### Branch Structure

```
June2026  (active development)
  └── (most corrections already applied to working tree)
```

`origin/feature/set-mode` exists but contains the original Phase 3
implementation commits. The current working tree on `June2026` has all
Phase 3 code plus Phase 1-2 corrections committed incrementally.

### Commits Applied (Completed)

| Area | Description | Status |
|------|------------|--------|
| Firmware beep audio files | Pre-generated WAVs in `Firmware/pregen_audio/` | ✅ |
| Firmware beep playback API | `hal_audio_play_beep()`, `audio_play_beep()`, IPC bypass | ✅ |
| Software `comm_play_beep()` | `CommBeepType` enum, IPC beep request | ✅ |
| Error beeps across modes | 16+ `comm_play_beep()` calls in all mode handlers | ✅ |
| `[*]` fix for Set Mode | `set_mode.c:651` — exits mode per spec | ✅ |
| Full parameter set | Power, Mic Gain, Compression, NB, NR, AGC, Preamp, Atten, + 4 bonus | ✅ |

### Remaining Commits

| Commit | Phase | Status |
|--------|-------|--------|
| `software: Fix frequency announcement format` | Phase 4 | ❌ Not started |
| `software: Add test_set_mode unit tests` | Phase 3.3 | ❌ Not started |
| `docs: Update Set Mode documentation` | Phase 5 | ❌ Not started |
| `docs: Add Set Mode regression script` | Phase 5 | ❌ Not started |

---

## Verification Checklist

### Implementation Verified ✅
- [x] `set_mode_init()`, `set_mode_enter()`, `set_mode_exit()` exist and work
- [x] `set_mode_handle_key()` routes all key types in Set Mode
- [x] `radio_set_power()` / `radio_set_mic_gain()` / etc. call Hamlib
- [x] All 8 planned parameters settable (power, mic, comp, NB, NR, AGC, preamp, atten)
- [x] 4 bonus parameters tunable (Tuning Step, VOX, Filter Number, Keyer Speed)
- [x] Key routing: Config → Set → [A]/[B] → Freq → Normal
- [x] `[A]` shift toggle + auto-clear works correctly
- [x] Full workflow: Normal → Set → adjust power → `[#]` → Set Off

### Beep / Audio Verified ✅
- [x] Beep WAV files exist and play correctly
- [x] `hal_audio_play_beep()` / `audio_play_beep()` implemented
- [x] Key press beep fires on key down (via `keypad.c` → IPC)
- [x] Hold beep fires at 500ms
- [x] Error beep plays on invalid key in all modes
- [x] Beeps mix correctly with speech (ALSA dmix, beep bypass)
- [x] `config key_beep = 0` disables all beeps
- [x] `comm_play_beep()` API works across all modes

### Set Mode Behavior Verified ✅
- [x] `[*]` exits Set Mode (calls `set_mode_exit()`)
- [x] `[B]` toggles OFF ↔ IDLE
- [x] `[D]` cancels edit (EDITING → IDLE) or exits (IDLE → OFF)
- [x] `[#]` confirms value → `apply_value()` → exits Set Mode
- [x] Set Mode announcement says "Set" / "Set Off"

### Remaining ❌
- [ ] Frequency announcement uses "dot" for sub-kHz (still uses "point")
- [ ] `test_set_mode.c` unit tests created and passing
- [ ] Set Mode regression script created
- [ ] `Currently_Implemented_Keys.md` updated (separate commit)

---

## Estimated Effort (Remaining)

| Item | Estimate |
|------|----------|
| `test_set_mode.c` unit tests | ~15 min |
| Frequency "dot" vs "point" fix | ~1 hour |
| Set Mode regression script | ~15 min |
| Final regression + merge | ~20 min |
| `Currently_Implemented_Keys.md` update (separate commit) | ~15 min |
| **Total remaining** | **~2 hours** |

---

## Appendices

### IC-7300 Parameter Ranges

The IC-7300 (Hamlib model 3073, verified in `config.h:20`) supports:

| Parameter | Range | Notes |
|-----------|-------|-------|
| Power level | 0-100W | Normalized 0-100% |
| Mic gain | 0-100% | |
| Compression | 0-10 or 0-100 | Varies by radio; code normalizes to 0-100 |
| NB / NR | On/Off + level 0-10 | Toggle via `[A]`/`[B]` in editing state |
| AGC speed | Fast / Medium / Slow | Enum-based (no numeric level) |
| Preamp | Off / 1 / 2 | Three states |
| Attenuation | 0+ dB | Accepts any value; typical 0/20 dB |

### Shift Key Architecture

The `[A]` shift key is managed in `main.c`:

- `g_shift_active` static variable (line 53)
- `[A]` tap toggles it (line 88-91), announces "Shift" / "Shift off"
- **One-shot auto-clear:** After any consumed key, shift auto-clears
  (lines 69-71, 80-82, 107-109, 116-118, 132-134)
- Shift state is captured as `was_shifted = g_shift_active` before
  routing (line 64), then passed as `is_shifted` to `set_mode_handle_key()`
  and `normal_mode_handle_key()`
- Set Mode uses `is_shifted` for: Compression, Keyer Speed, Filter
  Number, Tuning Step, VOX, Attenuation (6 shift-aware params)
- When not in Set Mode, `[A]` is free for Normal Mode shift queries

### `[B]` Key Detail

`[B]` entry is split between two files by design:

- **`main.c:95`** — enters Set Mode from Normal Mode when Set Mode is not
  active (handles the transition)
- **`set_mode.c:494-522`** — handles all Set Mode internal `[B]`
  behaviors: IDLE → OFF toggle, and in EDITING: if the current parameter
  is a toggle-type (NB, NR, Compression, VOX), `[B]` disables it;
  otherwise `[B]` exits Set Mode

This dual handling is intentional: `main.c` owns the mode-entry decision,
`set_mode.c` owns all in-mode behavior.
