# HAMPOD HITL Test Specification

**Created:** 2026-04-04
**Status:** Draft — tests not yet implemented

---

## Overview

HAMPOD's HITL (Hardware-in-the-Loop) CI testing runs automated tests on the
self-hosted Raspberry Pi 5 runner. "Hardware in the loop" means the real Pi
hardware (audio subsystem, keypad, and optionally radio) participates in every
test run — but human interaction is replaced by software automation.

Tests are triggered automatically on every PR to `dev` or `main`, and can also
be triggered manually on any branch via the GitHub Actions UI ("Run workflow").

---

## Test Architecture

```
GitHub Actions (CI trigger)
        |
        v
Pi 5 Self-Hosted Runner
        |
   +---------+------------------+
   |                            |
Job 1: Audio + Keypad     Job 2: Radio Control
(always runs)             (skips if radio off/unplugged)
```

### Hardware Present on the Runner Pi

| Component    | Status        | Notes                                   |
|--------------|---------------|-----------------------------------------|
| USB keypad   | Always connected | 19-key keypad, same as production      |
| USB speaker  | Always connected | Or routed via snd-aloop for CI         |
| Piper TTS    | Installed     | Same model as production                |
| IC-7300 (or TS-570/TS-2000) | Physically nearby | Plugged in and on when radio tests needed |

---

## Job 1: Audio + Keypad HITL Tests

**Script:** `hampod-hitl-test/run_no_radio_tests.sh`
**Requires radio:** No
**Always passes:** Yes (unless a test explicitly fails)

### How It Works

1. **Virtual audio loopback** — `snd-aloop` kernel module creates a paired
   virtual soundcard. HAMPOD plays TTS audio to the loopback playback device.
   A capture process records from the paired capture device simultaneously.

2. **Transcript verification** — `whisper.cpp` transcribes the recorded audio
   clip. The transcript is compared (fuzzy match) against expected text stored
   in `hampod-hitl-test/expected/`.

3. **Input simulation** — Synthetic keypad events are injected via `uinput` or
   `evemu`, replacing physical button presses. HAMPOD's keypad HAL sees these
   as real HID events.

### Planned Test Coverage

| Test | Trigger | Expected TTS Output |
|------|---------|---------------------|
| Startup announcement | HAMPOD start | "system ready" (or similar) |
| Key press — digit 1 | Simulate key 1 in frequency entry | "one" |
| Key press — Enter | Simulate Enter key | varies by mode |
| Key hold — minus | Hold minus key | enters config mode announcement |
| Mode transition | Enter frequency mode | mode name announced |

> **Note:** Exact expected strings TBD when tests are written. Store in
> `hampod-hitl-test/expected/<test_name>.txt`.

---

## Job 2: Radio Control HITL Tests

**Script:** `hampod-hitl-test/run_radio_tests.sh`
**Requires radio:** Yes (IC-7300, TS-570, or TS-2000 via USB, powered on)
**Auto-skips if radio absent:** Yes — exits 0, job shows as passed

### How It Works

1. **Radio detection** — `rigctl` (hamlib) attempts a quick connection to the
   configured serial port. If it fails, the script exits 0 immediately and
   the job is marked as skipped/passed.

2. **Radio control tests** — HAMPOD is started with the radio connected. Tests
   simulate key sequences and verify the radio responds correctly (e.g.,
   frequency changes are confirmed by reading back from the radio via hamlib).

### Planned Test Coverage

| Test | Action | Verification |
|------|--------|--------------|
| Frequency entry | Enter 14.255.00 via keypad | Read back frequency from radio via hamlib |
| Band change | Enter a new band via keypad | Radio band changes, TTS announces |
| Connect/reconnect | Unplug and replug radio | HAMPOD recovers and re-announces |
| Polling rate adjustment | Connect, disconnect | Log shows polling rate changes |

---

## Hamlib Dummy Radio (Future Option)

Hamlib includes a dummy/virtual radio backend (`RIG_MODEL_DUMMY`, model 1) that
simulates a rig entirely in software. If this backend supports the commands
HAMPOD uses, some radio tests could be run in Job 1 without physical hardware.

**Status: Needs investigation.** Run `rigctl -m 1` to explore what the dummy
backend supports. Document findings in `hampod-hitl-test/radio/README.md`.

---

## CI Integration

Workflow file: `.github/workflows/hitl-test.yml`

Triggers:
- `pull_request` targeting `dev` or `main` — automatic
- `workflow_dispatch` — manual, any branch, from the GitHub Actions UI

To trigger manually: GitHub → Actions tab → "HITL Tests" → "Run workflow" →
select branch → "Run workflow".

---

## Relationship to Existing Tests

| Test type | Where | Runs in CI? | Requires human? |
|-----------|-------|-------------|-----------------|
| Unit tests | `Software2/tests/` | Future (separate job) | No |
| HAL automated tests | `Firmware/hal/tests/` | Future | No |
| HITL Job 1 (this doc) | `hampod-hitl-test/` | Yes (stub) | No |
| HITL Job 2 (this doc) | `hampod-hitl-test/` | Yes (stub, skips if no radio) | No |
| Manual regression | `Documentation/scripts/` | No | Yes |
| Pre-merge SOP | `Documentation/sop_merge_to_main/` | No | Yes |

The HITL CI tests are intended to automate what is currently covered by the
manual Phase 0 integration test (audio + keypad) and Phase 1 regression
(frequency mode with radio). Manual regression tests remain required for full
pre-merge validation until CI coverage is comprehensive.

---

## Open Questions / TODOs

- [ ] Confirm input simulation approach (uinput vs evemu vs pyserial)
- [ ] Determine whisper.cpp model to use and fuzzy match threshold
- [ ] Research hamlib dummy backend compatibility with HAMPOD commands
- [ ] Decide whether to run unit tests as a third CI job or a separate workflow
- [ ] Set `ALSA_CARD` or equivalent env var so HAMPOD targets the loopback device in CI
