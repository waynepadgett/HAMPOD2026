# HAMPOD2026 Project Plan

**Purpose:** Defines integration/testing/validation strategy, risks and mitigations, milestones and deliverables, and code readability analysis.  
**Audience:** Developers, maintainers, and stakeholders.

---

## 1. Integration, Testing, and Validation Strategy

### 1.1 Test Pyramid

| Layer | Scope | Artifacts | When to Run |
|-------|--------|-----------|-------------|
| **Unit** | Single modules (config, radio, speech, comm helpers) | `Software2/tests/test_*.c` (e.g. `test_config.c`, `test_radio.c`, `test_comm_*.c`) | Every code change in that module |
| **HAL** | Keypad + Audio + TTS without full stack | `Firmware/hal/tests/test_hal_integration` | After HAL or Firmware HAL changes |
| **Firmware integration** | Firmware ↔ Imitation Software (packet protocol) | `Firmware/imitation_software` + `Documentation/scripts/run_remote_test.sh` | After packet format or Firmware I/O changes |
| **Phase 0 integration** | Software2 ↔ Firmware (router thread, keypad + speech) | `Documentation/scripts/Regression_Phase0_Integration.sh` | Before any significant commit; validates router and pipes |
| **Mode / feature** | Normal mode, frequency mode, radio control | `Regression_Normal_Mode.sh`, `Regression_Frequency_Mode.sh`, `Regression_Phase_One_Manual_Radio_Test.sh`, etc. | After mode or radio logic changes |
| **Performance / stress** | Latency, CPU, buffer under-runs, long run | Custom harness; Section 9 targets in System_Architecture_Detail.md | Before release; periodic validation |

### 1.2 Validation Gates

- **Pre-commit (required):** Run at least `Regression_Phase0_Integration.sh` (and `Regression_HAL_Integration.sh` if Firmware/HAL changed). All must pass.
- **Pre-release:** Full regression suite + performance benchmarks meeting keypad-to-speech and audio latency targets (see System_Architecture_Detail.md §9).
- **Acceptance:** On-target hardware (Pi 3/4/5); keypad and USB audio connected; “System Ready” and key echo heard; no pipe leaks or packet type errors.

### 1.3 Success Criteria (Summary)

| Test / Gate | Success Criteria |
|-------------|------------------|
| Unit tests | All `make tests` targets pass; no memory leaks (valgrind where applicable). |
| HAL integration | Keypad keys detected and announced; TTS and audio output confirmed by tester. |
| Phase 0 integration | “Phase zero integration test ready” / “System Ready”; key press and hold announced; no “packet type mismatch” or “bruh” errors. |
| Regression suite | All `Documentation/scripts/Regression_*.sh` pass; no new failures vs baseline. |
| Performance | Keypad-to-speech ≤ 200 ms (Pi 5), ≤ 300 ms (Pi 3); TTS-to-audio ≤ 100 ms; buffer under-run &lt; 1%. |

### 1.4 References

- **Regression_Testing_Plan.md** – Step-by-step execution, prerequisites, verification checklists.
- **System_Architecture_Detail.md** §10 – Integration/test table and locations.
- Scripts: `Documentation/scripts/Regression_Phase0_Integration.sh`, `Regression_HAL_Integration.sh`, `Regression_Normal_Mode.sh`, `Regression_Frequency_Mode.sh`, etc.

---

## 2. Risks, Dependencies, and Mitigation Plans

### 2.1 Dependencies

| Dependency | Type | Version / Constraint | Mitigation if Unavailable |
|------------|------|----------------------|----------------------------|
| **Debian (Trixie)** | OS | 64-bit ARM | Document supported release; avoid relying on unreleased or EOL versions. |
| **Hamlib** | Library | Version pinned in `install_hampod.sh` | Pin version in install script; document and provide script to install matching Hamlib build. |
| **ALSA** | System | libasound2, alsa-utils | Standard on Debian; install via `apt`; fallback to default device if preferred USB device missing. |
| **Piper TTS** | External | Model + binary | Install via `Documentation/scripts/install_piper.sh`; document model path and offline use. |
| **USB keypad** | Hardware | NKRO-capable, 12+ keys | Document minimum key set (0–9, *, #, A–D); test with known-good keypad. |
| **USB audio** | Hardware | User-configurable | Prefer user-specified device; fallback to default ALSA; handle hot-plug re-init. |
| **Raspberry Pi 3/4/5** | Hardware | BCM2710/2711/2712 | Validate on each target; document RAM and thermal limits (see Hardware_Constraints.md). |
| **gcc, make, pthreads** | Build | Standard toolchain | Listed in `Documentation/scripts/install_hampod.sh`; no exotic extensions. |

### 2.2 Risks and Mitigation

| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| **Audio device not found** | Medium | High (no speech) | Preference list in config; fallback to default ALSA; clear spoken and logged error. |
| **Hamlib version mismatch** | Low | Medium | Pin Hamlib version in install script; document and script matching install. |
| **Keypad debounce / ghosting** | Low | Medium | Debounce with configurable interval; unit tests for key events; NKRO requirement in docs. |
| **Configuration corruption** | Low | High | Write to temp file then atomic rename; validate on load; optional read-only rootfs. |
| **Power loss during write** | Medium | High | SD-card protection (`power_down_protection.sh`); atomic writes; watchdog for clean shutdown. |
| **Pipe leaks / stale FIFOs** | Low | Medium | `pipe_remover.sh` on startup/cleanup; regression tests check for leaks. |
| **Router thread packet mismatch** | Low | High | Phase 0 integration test must pass; type-based dispatch and queues in `comm.c`. |
| **High latency on Pi 3** | Medium | Medium | Target ≤ 300 ms keypad-to-speech on Pi 3; lightweight modes; optional overclock doc (OVERCLOCK_AND_TODOS.md). |
| **Third-party repo (Piper, Hamlib) churn** | Low | Medium | Pin versions; document source and commit/tag; local mirror or cache if needed. |

### 2.3 Dependency Diagram (Conceptual)

```
HAMPOD Application
  ├── Software2 (main app)  →  config (hampod.conf), Hamlib, comm (pipes)
  ├── Firmware (firmware.elf) →  HAL (keypad, audio, TTS), pipes
  └── install_hampod.sh     →  apt, Piper install script, clone, build
```

---

## 3. Milestones, Deliverables, and Success Criteria

### 3.1 Milestones

| Milestone | Deliverables | Success Criteria |
|-----------|--------------|-------------------|
| **M1: Environment and HAL** | Working Pi image (Debian Trixie); install script; HAL integration test passing. | `install_hampod.sh` completes; `test_hal_integration` runs; keypad and TTS/audio confirmed by tester. |
| **M2: Pipe protocol and Phase 0** | Firmware ↔ Software2 pipe protocol; router thread; Phase 0 integration test. | `Regression_Phase0_Integration.sh` passes; key press and hold announced; no packet type errors. |
| **M3: Core modes and radio** | Normal mode; frequency mode; config load/save; at least one radio (e.g. IC-7300) working. | `Regression_Normal_Mode.sh` and `Regression_Frequency_Mode.sh` (or manual radio test) pass; frequency readout and band control verified. |
| **M4: Multi-radio and robustness** | Up to 10 radios in config; audio device fallback; config validation and error messages. | Multiple radios in `hampod.conf`; preferred audio fallback works; invalid config produces clear errors. |
| **M5: Release readiness** | Regression suite green; latency/performance targets met; docs updated (Hardware_Constraints, System_Architecture, etc.). | All regression scripts pass on Pi 3, Pi 4, Pi 5; keypad-to-speech and TTS latency within spec; no known critical bugs. |

### 3.2 Deliverables (Artifacts)

- **Code:** Software2 (main app), Firmware (firmware.elf + HAL), scripts under `Documentation/scripts/`.
- **Config:** `Software2/config/hampod.conf` with documented options and defaults.
- **Docs:** README, Hardware_Constraints, System_Architecture, Project_Functional_Requirements, Regression_Testing_Plan, this Project_Plan.
- **Tests:** Unit tests in `Software2/tests/`, HAL tests in `Firmware/hal/tests/`, regression scripts in `Documentation/scripts/`.
- **Install/deploy:** `install_hampod.sh`, `hampod_on_powerup.sh`, `power_down_protection.sh`, `remote_install.sh` (.sh and .ps1).

### 3.3 Success Criteria (Project-Level)

- **Functional:** Keypad input → mode handling → Hamlib control → speech/audio output; config load/save and multi-radio selection.
- **Performance:** Keypad-to-speech ≤ 200 ms (Pi 5), ≤ 300 ms (Pi 3); TTS-to-audio ≤ 100 ms; CPU and buffer targets per System_Architecture_Detail §9.
- **Quality:** No regressions in automated suite; no pipe leaks; clean shutdown and optional read-only root for field use.
- **Accessibility:** All critical feedback via speech; tactile-first keypad; documented for visually impaired operators.

---

## 4. Code Readability Analysis

### 4.1 Scope

- **Software2:** `src/*.c` (main application and modes).
- **Firmware:** `firmware.c`, `keypad_firmware.c`, `audio_firmware.c`, and `hal/*.c`.

### 4.2 Complexity (Size and Responsibility)

| File | Lines (approx) | Assessment |
|------|------------------|------------|
| **Software2:** `comm.c` | ~650 | High – central IPC, router thread, queues, timeouts. Already split into logical sections; consider extracting queue logic or packet serialization to a separate module if it grows. |
| **Software2:** `config.c` | ~605 | Moderate–high – init, undo, parse, write, many getters/setters. Structure is clear; undo and file I/O could be separate files later. |
| **Software2:** `radio_setters.c` | ~643 | Moderate – many similar get/set wrappers around Hamlib. Good candidate for macro or code generation to reduce repetition. |
| **Software2:** `set_mode.c` | ~592 | Moderate – mode state and key handling. Readable; could split “digit entry” vs “navigation” if it grows. |
| **Software2:** `speech.c` | ~369 | Moderate – queue and TTS interface. Clear API. |
| **Software2:** `frequency_mode.c`, `normal_mode.c` | ~309–361 | Good – single responsibility per mode. |
| **Firmware:** `hal_audio_usb.c` | ~728 | High – ALSA and device selection. Long file; consider splitting “device enumeration” vs “playback/streaming” if maintained heavily. |
| **Firmware:** `firmware.c` | ~447 | Moderate – main loop and pipe setup. Acceptable for a main orchestrator. |

**Summary:** No single file is unmanageable; the largest are central modules (comm, config, radio_setters). Refactors are optional and can be driven by change rate and bug density.

### 4.3 Naming

- **Consistent prefixes:** `config_*`, `radio_*`, `comm_*`, `speech_*`, `keypad_*`, `*_mode_*` – good; public API is easy to discover.
- **Module state:** `g_config`, `g_rig_mutex`, `fd_firmware_in` – clear and scoped.
- **Types:** `CommPacket`, `HampodConfig`, `KeyPressEvent` – descriptive.
- **Improvements:** Avoid single-letter or opaque names in hot paths; keep `LOG_ERROR` / `DEBUG_PRINT` usage consistent so logs stay interpretable.

### 4.4 Duplication

- **Radio get/set pattern:** In `radio_setters.c` (and similar in `radio_queries.c`), many functions follow the same pattern: lock → check `g_connected`/`g_rig` → call Hamlib → unlock → map result/error. **Suggestion:** Introduce small helpers, e.g. `radio_get_level_float()` / `radio_set_level_float()`, or a macro for “lock–check–call–unlock–return,” to reduce copy-paste and keep behavior consistent.
- **Config get/set:** `config.c` has many symmetric getters/setters; structure is consistent and acceptable; further reduction would require code generation or a more generic get/set layer.
- **Error handling:** Repeated “if (retcode != RIG_OK) { … return -1; }” and “if (!g_connected || !g_rig) { unlock; return -1; }” patterns. Centralizing in helpers (e.g. `radio_check_ok(retcode)`) would improve readability and make error logging uniform.
- **Firmware/HAL:** Pipe open/close and thread setup patterns repeat across keypad and audio; shared “pipe lifecycle” or “thread runner” helpers could reduce duplication if more I/O threads are added.

### 4.5 Recommendations

1. **Short term:** Add or extend unit tests for `config` and `comm` (queue/timeout behavior) to lock in readability and refactor safety.
2. **Medium term:** In `radio_setters.c` / `radio_queries.c`, extract common “rig level get/set” and “connection check” logic into shared helpers to cut duplication and standardize error handling.
3. **As needed:** If `comm.c` or `hal_audio_usb.c` grows further, split by responsibility (e.g. queue vs packet I/O; device list vs playback).
4. **General:** Keep section comments (e.g. `// ==========`) and docblocks for public APIs; they already aid navigation in the larger files.

---

*This plan should be updated when major dependencies change, new milestones are agreed, or the test strategy is extended.*
