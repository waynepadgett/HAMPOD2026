# Formal Methods: Gap Analysis & Plan

> **Status:** Phase 0 complete (Step 0.1-0.3 verified, Step 0.4 .txt export deleted)
> **Created:** 2026-06-21
> **Last Updated:** 2026-06-21
> **Goal:** Get from current docs to formal-methods-ready TDD

---

## What Already Exists

| Asset | Status | Notes |
|-------|--------|-------|
| `High_Level_Requirements.md` | **130 HLRs, 66 SLRs** — thorough | Covers modes, key modifiers, radio ops, config architecture, audio, reliability |
| `hampod_config.cfr` | Clafer model — **verified** | Covers audio config and radio config; 3 constraint mismatches found (CFR-01 through CFR-03), 6 config HLRs not yet modeled (CFR-04 through CFR-09) |
| `High_Level_Requirements.txt` | Legacy export (stale) | Plain-text export of the .md as of commit 08c05fd — only covers HLR-001 through HLR-076 (the .md has 130). Used for external review; now superseded by .md. Deleted. |
| Original Hampod docs | 7 manuals (8 files) | ICOM (v1.06, Manual 2, Add-On), Kenwood v1.04, K3 v1.20, Yaesu v1.00, Yaesu FT-8x7 — ground truth for SLR-008 |

---

## Phase 0: Verify & Fix What Already Exists

**Do not add new work until this phase is complete.** If the foundation is wrong, everything built on top is garbage.

### Step 0.1 + 0.2: Verify HLRs and Cross-Reference Against Original Docs ✅ COMPLETE

**Scope:** Every SLR and HLR verified against all 8 original Hampod manuals. Internal consistency checked for parent references, numbering, priority contradictions, verification methods, duplicates, and coverage gaps.

#### 0.1.1 Internal Consistency Issues (4 found)

| ID | Category | Severity | Description |
|----|----------|----------|-------------|
| **INT-01** | Numbering | Low | **HLR-076** (Timeout Announcement) is inserted between HLR-067 and HLR-068, breaking sequential ordering. Traceability matrix (Section 4.2) is correct; only the document body has the out-of-sequence entry. |
| **INT-02** | Priority Contradiction | **High** | **HLR-116** (SHALL — "key press immediately interrupts speech") has parent **HLR-070** (SHOULD — "speech interruption"). In requirements engineering, a child cannot be stronger than its parent. **Fix: Upgrade HLR-070 from SHOULD to SHALL.** |
| **INT-03** | Verification Method | Medium | **HLR-111** ("SHALL be designed to minimize latency") uses "Test" but "designed to minimize" is not directly testable. **Fix: Change to "Analysis, Test" or rewrite as measurable.** |
| **INT-04** | Verification Method | Low | **SLR-033** ("SHALL NOT assume great technical familiarity") uses "Demonstration" but is very subjective. Consider adding measurable criteria. |

#### 0.1.2 Missing HLRs — Behaviors From Original Manuals With No Requirement (24 gaps)

**Must-fix (blocking for Phase 1):**

| ID | Missing Behavior | Source Evidence |
|----|-----------------|-----------------|
| **GAP-03** | **DTMF Mode entry key combo** — [Shift]+[D] Hold enters DTMF Mode | ICOM (line 162), K3 (lines 172-173), Yaesu (lines 148, 232, 340). HLR-026 describes exit but NOT entry. |
| **GAP-04** | **Memory Mode exit mechanism** — [*] or [D] exits Memory Mode | ICOM (lines 331, 333), K3 (line 167), Yaesu (line 373). HLR-020 specifies entry but no HLR specifies exit. |

**Should-fix (important):**

| ID | Missing Behavior | Source Evidence |
|----|-----------------|-----------------|
| **GAP-01** | **Quick Band Change mode** — Press [B] to "Band" mode, then number key for band jump (1=160m, 2=80m, etc.) | ICOM (line 306), K3 (line 128), Yaesu (lines 353-359). All manuals. No HLR. |
| **GAP-02** | **Volume Up/Down shortcuts** — [A] Hold = Vol Up, [B] Hold = Vol Down | ICOM (lines 82, 109), K3 (lines 95, 129), Yaesu (lines 78, 103). Every manual. No HLR. |
| **GAP-05** | **DStar Editable Text Buffer** — character-by-character editing, cursor ops, 6 hot keys (Link/Talk/Unlink/Relink/Echo/Info) | ICOM (lines 369-410). HLR-037 is far too vague — ~40 lines of behavior uncovered. |
| **GAP-08** | **777 firmware version query** — Enter 777 + Enter announces version | ICOM (line 178), K3 (line 190), Yaesu (lines 153, 164). HLR-075 covers 999 but 777 is missing. |
| **GAP-13** | **Last Frequency Recall** — [Shift]+[#] recalls last entered frequency | ICOM (lines 155-156), K3 (lines 167-168), Yaesu (lines 132-134). All manuals. No HLR. |
| **GAP-09** | **AF/RF Announcement toggle** — separate config to disable AF/RF encoder announcements | ICOM (line 476), K3 (line 224), Yaesu (lines 418, 577). Config option #6 in all manuals. |
| **GAP-10** | **Power On To Port** — config to select startup serial port | ICOM (line 499), K3 (line 240), Yaesu (line 433). Config option #10 in all manuals. |
| **GAP-15** | **Voice parameter ranges** — Volume (0-9), Speed (0-13), Pitch (0-99), etc. | All manuals. HLR-063/064 say "adjustable" without ranges. Needed for config. |

**Nice-to-fix (defer):**

| ID | Missing Behavior | Source |
|----|-----------------|--------|
| **GAP-06** | Memory Mode 2 (auto-transfer) — [Shift]+[C] Hold | ICOM-specific |
| **GAP-07** | Memory Channel Name Tags via DStar buffer | ICOM |
| **GAP-11** | Dual Receiver support (Main/Sub) | ICOM IC-7800/7851/9100/9700 |
| **GAP-12** | Dial/VFO Lock and Panel Lock | ICOM |
| **GAP-14** | Debug Mode — [Shift]+[B] speaks CI-V data | ICOM |
| **GAP-16** | Yaesu Menu item access via Set+# | Yaesu-specific |
| **GAP-17** | Memory Scan toggle — [2] Hold | ICOM |
| **GAP-18** | Monitor Level query/set | ICOM |
| **GAP-19** | Key beeps muted during DTMF Mode | K3 |
| **GAP-20** | K3: Watch VFO Display modes | K3-specific |
| **GAP-21** | K3: SWR Watch mode | K3-specific |
| **GAP-22** | K3: Text Buffer read mode (decoded CW/data) | K3-specific |
| **GAP-23** | Yaesu: A/B and A=B button simulation | Yaesu-specific |
| **GAP-24** | Frequency shorthand broader than HLR-017 | Kenwood |

#### 0.1.3 Contradictions Found

| ID | Severity | Contradiction | Resolution Needed |
|----|----------|--------------|-------------------|
| **CON-01** | **High** | **Memory Mode radio behavior** — ICOM: radio stays in current mode. Yaesu: radio switches to Memory Mode. These are opposite. HLR-019 doesn't specify. | Decide: per-radio via config, or pick one behavior? |
| **CON-02** | Low | **DTMF entry key combo varies** — ICOM: Shift+D Hold. Kenwood: Shift+D. K3: both. | Config-driven, but need HLR for entry at all. |
| **INT-C1** | High | **HLR-116 (SHALL) vs HLR-070 (SHOULD)** — child stronger than parent. | Same as INT-02. Upgrade HLR-070. |

#### 0.1.4 Coverage Summary

| Metric | Count |
|--------|-------|
| Total HLRs verified | 130 |
| HLRs confirmed correct | ~100 |
| HLRs with issues | 4 |
| Missing HLRs (gaps) | 24 |
| Contradictions with OG docs | 2 |
| Internal contradictions | 1 |
| Parent references valid | 130/130 |
| Circular references | 0 |
| Duplicates found | 0 |

---

### Step 0.3: Verify hampod_config.cfr against HLRs ✅ COMPLETE

Cross-referenced every Clafer constraint, field, and section against the 130 HLRs in `High_Level_Requirements.md`.

#### 0.3.1 Constraints That Match HLRs (confirmed correct)

| Clafer Constraint | HLR | Match |
|-------------------|-----|-------|
| `targetLatencyMs >= 100` | HLR-109 (100ms target) | ✅ |
| `targetLatencyMs <= 300` | HLR-110 (300ms max) | ✅ |
| `keyPressFrequencyHz = 1000` | HLR-113 (1000Hz) | ✅ |
| `keyPressDurationMs = 50` | HLR-113 (50ms) | ✅ |
| `keyHoldFrequencyHz = 700` | HLR-114 (700Hz) | ✅ |
| `keyHoldDurationMs = 50` | HLR-114 (50ms) | ✅ |
| `errorFrequencyHz = 400` | HLR-115 (400Hz) | ✅ |
| `errorDurationMs = 100` | HLR-115 (100ms) | ✅ |
| `beepLatencyMaxMs = 50` | HLR-112 (50ms max) | ✅ |
| `audioBufferMs 20-100` | HLR-125 (~50ms target) | ✅ range covers target |
| `Pi5: latency <= 100` | HLR-126 (100ms on Pi5) | ✅ |
| `Pi3B: latency <= 300` | HLR-127 (300ms on Pi3B) | ✅ |
| `Pi3B: uses Festival` | HLR-127 (Festival on Pi3B) | ✅ |
| `[#KnownDevice >= 0]` | HLR-103 (missing devices tolerated) | ✅ |

#### 0.3.2 Constraints That DON'T Match HLRs (3 issues)

| ID | Clafer Value | HLR Spec | Issue | Severity |
|----|-------------|----------|-------|----------|
| **CFR-01** | `timeoutMs >= 1000` (1 sec min) | HLR-069: "default 5-20 range" (likely seconds) | Minimum should be **5000ms**, not 1000ms. If HLR-069 means 5-20 seconds, the model allows timeouts as low as 1 second which is too aggressive for radio communication. | Medium |
| **CFR-02** | `Pi4: targetLatencyMs <= 200` | HLR-128: "characterized during development" | HLR-128 has **no hard number** — Pi4 performance is untested. The model assumes 200ms which is speculative. Should be `<= 300` (same as Pi3B) until characterization is done, or remove the constraint. | Low |
| **CFR-03** | `persistentLoading` as optional boolean | HLR-123: "SHOULD keep synth loaded" | HLR-123 is SHOULD, not optional. The model makes it a `?` (0 or 1) flag. This is correct for a config model but should note that HLR-123 defaults to enabled. | Low |

#### 0.3.3 HLRs That the Clafer Model Should Cover But Doesn't (structural gaps)

These are HLRs that define config parameters or config-related behavior not captured in the model:

| ID | HLR | What's Missing | Impact |
|----|-----|---------------|--------|
| **CFR-04** | HLR-059 (configurable key mapping) | Model has `KeyMappings` but no structure for the **keypad layout abstraction** (HLR-060: phone vs. numeric keypad mapping). The `keyCode : string` field is fine, but there's no layout type selector. | Medium — Phase 2 |
| **CFR-05** | HLR-061 (config parser security) | No model section for parser validation rules. The model assumes valid input. | Low — implementation concern |
| **CFR-06** | HLR-063 through HLR-067 | **System config parameters** (volume, speed, verbosity, beep enable, timeout) are not in the model. These are HAMPOD system settings, not radio config. | Medium — need a `SystemConfig` section |
| **CFR-07** | HLR-075 (factory reset via 999) | Not in the model. Behavioral, not structural. | Low |
| **CFR-08** | HLR-089/090 (unsupported feature handling) | Model has `OptionalFeatures` but no structure for **graceful degradation behavior** (announce "not available", continue operating). | Low — behavioral |
| **CFR-09** | HLR-092 through HLR-095 | **Runtime config management** (memory loading, device switching, failure handling) not in model. These are behavioral, not config structure. | Low — runtime concern |

#### 0.3.4 Model Sections That Don't Correspond to Any HLR

| Clafer Section | Issue |
|----------------|-------|
| `OtherTTS ?` (HLR-130) | HLR-130 says "SHOULD support addition of alternative TTS" — the model correctly makes this optional. No issue. |
| `VoiceModel : string` under Piper | Implementation detail not in HLR-121. Fine for config model. |
| `PreGeneratedAudioCache` sub-fields (cacheDigits, cacheFrequencyComponents, etc.) | HLR-129 says "caching of pre-generated audio for common announcements" but doesn't enumerate what to cache. The model adds reasonable implementation detail. Fine. |
| `ManufacturerFeatures` (icomFeatures, kenwoodFeatures, yaessuFeatures) | Not tied to specific HLRs. This is manufacturer-specific config structure. Fine for config model. |

#### 0.3.5 Coverage Summary

| Metric | Count |
|--------|-------|
| HLRs directly referenced in model | ~35 |
| HLRs with matching constraints | 14 |
| Constraints with HLR mismatches | 3 |
| Config-related HLRs not in model | 6 |
| Behavioral HLRs not in model (expected) | ~90 |
| Model sections without HLR refs | 4 (all acceptable) |

**Verdict:** The Clafer model is **solid for audio config and radio config structure**. The main gaps are: (1) the timeout minimum is too low, (2) Pi4 latency is speculative, and (3) system config parameters (volume, speed, etc.) are missing a model section. None of these are blocking — they're fixable in Phase 1.

### Step 0.4: Delete the .txt export

`High_Level_Requirements.txt` was a plain-text export of the .md for external review. It was generated at commit 08c05fd and only contains HLR-001 through HLR-076 — the .md has since grown to 130 HLRs. It has been deleted. The .md is the single source of truth.
**Status:** ✅ COMPLETE — file deleted.

---

## Phase 1: Fill the Gaps (After Verification)

### Step 1.1: Fix the broken requirements
From the verification report (Section 0.1), fix every flagged HLR/SLR:
- Upgrade HLR-070 from SHOULD to SHALL (INT-02)
- Fix HLR-076 numbering (INT-01)
- Fix HLR-111 verification method (INT-03)
- Resolve CON-01 (Memory Mode radio behavior decision)

### Step 1.2: Add missing requirements
From the verification report, add HLRs for the 24 gaps identified. Priority order:
1. GAP-03 (DTMF entry) and GAP-04 (Memory exit) — blocking
2. GAP-01, 02, 05, 08, 09, 10, 13, 15 — important
3. GAP-06, 07, 11, 12, 14, 16-24 — defer as needed

Keep the same format (ID, SHALL/SHOULD/MAY, parent, verification method).

### Step 1.3: Write LLRs for the 5 critical subsystems
For each subsystem, define:
- Function signatures (name, params, return type)
- State transitions (if stateful)
- Error conditions
- Timing constraints (where applicable)

Start with **key dispatch** — it's the most testable and least dependent on hardware.

### Step 1.4: Write test specs for top 10 HLRs
Pick the 10 HLRs that cover the most critical user-facing behavior:
1. HLR-005 (key function assignment — 4 modifiers)
2. HLR-007 (set mode — query vs. adjust pattern)
3. HLR-013 through HLR-016 (frequency entry — the most complex state machine)
4. HLR-068 (key response time — 100ms target)
5. HLR-072 (radio disconnect handling)
6. HLR-116 (key press interrupts speech)
7. HLR-077 (config-driven behavior)

For each, write a test spec in plain English or pseudocode. No need for formal notation yet.

---

## Phase 2: Build on Verified Foundation

### Step 2.1: Write one radio config file (IC-7300)
Use the Clafer model as a template. Define key mappings, Hamlib bindings, and announcement text for the IC-7300. This validates that the config architecture actually works before generalizing.

### Step 2.2: Implement one test suite end-to-end
Write actual C unit tests for key dispatch (Step 1.3 LLRs + Step 1.4 test specs). Use a lightweight framework (Unity, Check, or just assert). Run on the Pi. If this works, the approach is validated for the other subsystems.

---

## What This Plan Skips (On Purpose)

- **Full traceability matrix** — nice to have, not needed for TDD
- **130 HLRs worth of LLRs** — do the critical 5 first, generalize later
- **Formal proof of correctness** — aim for "tests catch bugs," not "math proves correctness"

---

## What Kills This

1. **Skipping Phase 0** — if you start writing new docs before verifying the old ones, you'll compound errors. Do the verification first, no shortcuts.
2. **Perfectionism on the OG docs** — the original docs are a reference, not gospel. If the HLRs capture the behavior, move on. Don't spend weeks re-reading manuals to verify a SHOULD requirement.
3. **Scope creep on LLRs** — if you try to write LLRs for all 130 HLRs before writing any tests, you'll never start. Do 5 subsystems, then iterate.
4. **Hardware dependency** — some tests (audio latency, radio communication) need a real Pi + radio. Start with the software-only subsystems (key dispatch, config parsing) that can be tested on any machine.

---

## Appendix A: Formal Methods Ideas (Future Consideration)

> **NOTE:** The following represents ideas for potential future formal methods application. These are speculative and not currently scheduled. They serve as a "north star" for ensuring high reliability in the future.

### A.1 Where to Start (If Applying Formal Methods)

Applying formal methods to an existing embedded system like HAMPOD is best done incrementally. You should not try to "prove the whole system" at once.

**Recommended Starting Point: The Communication Protocol**
The most critical and error-prone part of the system is the **Firmware <-> Software Pipe Communication Protocol**.
*   **Why:** Concurrency, race conditions, and deadlocks are notoriously hard to test via standard methods (as seen with the recent "zombie process" and "connection bugs").
*   **Goal:** Formally specify the packet exchange to prove that no sequence of events can cause the system to hang or desync.

**Secondary Starting Point: The User Interface State Machine**
The "Hampod" user experience is effectively a large Hierarchical State Machine (Modes, Menus, Input States).
*   **Why:** Ensuring the user never gets "stuck" in a mode and that every input has a deterministic result.
*   **Goal:** Model the transition logic (e.g., "Frequency Entry Mode") to verify that all inputs (valid numeric, invalid, backspace, mode switching) are handled correctly.

### A.2 Properties and Behaviors to Prove

**Safety Properties (Bad things never happen)**
1.  **Deadlock Freedom**: The Firmware and Software never wait on each other indefinitely.
2.  **Buffer Safety**: The C-based Firmware never reads/writes partially formed packets (buffer overflows/underflows).
3.  **State Validity**: The system never enters an undefined Mode or State.

**Liveness Properties (Good things eventually happen)**
1.  **Responsiveness**: If a key is pressed (and hardware works), an Audio response is *always* generated eventually.
2.  **Progress**: A firmware update or mode change command always completes or times out safely (never hangs).

**Functional Correctness (The system does what it should)**
1.  **Mode Logic**: "If in Frequency Mode, pressing '1' then 'Enter' results in a Hamlib command to set frequency to '1' (plus logic for Hz/kHz)."
2.  **Hardware Abstraction**: "The HAL `read()` function always returns a valid event or a 'no-event' code, never garbage."

### A.3 Appropriate Tools

Given the stack (C code on Raspberry Pi, Inter-Process Communication via Pipes, potential future rewrites):

| Tool | Use Case | Benefit | Effort |
|------|----------|---------|--------|
| **TLA+** | Modeling the Pipe Protocol and "Fresh Start" architecture | Excellent for finding race conditions in communication layer | Moderate |
| **Spin (Promela)** | Verifying message passing protocol between Firmware and Software | Simpler for C-programmers than TLA+; perfect for "If I send A, then B must happen" | Low-Moderate |
| **Frama-C** | Verifying C firmware code (`hal_keypad.c`, `firmware.c`) | Can prove absence of buffer overflows and runtime errors via ACSL annotations | High |
| **Statecharts / XState** | Visualizing and verifying Input Mode logic | Helps ensure UI logic covers all edge cases | Low |

### A.4 Formal Methods Roadmap (If Applied)

1.  **Phase 0**: Write a TLA+ spec for the Pipe Protocol (Firmware <-> Software handshakes).
2.  **Phase 1**: Annotate critical `hal_*.h` interfaces with Frama-C contracts to define exactly what the hardware layer promises.
3.  **Phase 2**: Model the complex "Frequency Entry Mode" logic to ensure all edge cases are handled before coding the new Software layer.
