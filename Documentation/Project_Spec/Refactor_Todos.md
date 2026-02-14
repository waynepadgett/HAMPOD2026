# Refactor Todos: Path to Maximum Clarity

**Goal:** Bring the project to maximum clarity for contributors, maintainers, and users—documentation, code, structure, and tooling.

Use this as a checklist; reorder or split into sprints as needed.

---

## 1. Documentation clarity

### 1.1 Single source of truth for “what’s active”
- [ ] **State explicitly in one place** (e.g. root README or `Documentation/README.md`) that **Software2** is the active application and **Software** is legacy/reference. New contributors should not have to infer this.
- [ ] **Repository layout** – Add or update a short “Repository layout” section that lists: `Firmware/`, `Software2/`, `Software/` (legacy), `Documentation/`, `Documentation/scripts/`, `Hardware_Files/`, and what each is for. Root README has some of this; ensure it’s complete and points to Project_Spec for deeper docs.

### 1.2 Align Project_Spec with the active codebase
- [ ] **System_Architecture_Detail.md** – It currently describes the legacy **Software/** tree (ModeRouting.c, StateMachine.c, ConfigLoad.c, Configs.txt, KeyWatching.c, libModes.so). Either:
  - **Option A:** Update it to describe **Software2** (e.g. `set_mode.c`, `config.c`, `comm.c`, `speech.c`, `hampod.conf`), or
  - **Option B:** Add a clear note at the top: “This section describes the legacy Software/ architecture; the active application is Software2 (see README and Project_Plan).”
- [ ] **Config file name** – Everywhere in Project_Spec, use one canonical path: **`Software2/config/hampod.conf`**. No mixed references to `Configs.txt` without saying it’s legacy Software.

### 1.3 Script and path consistency
- [ ] **Script paths** – All docs that mention scripts should use **`Documentation/scripts/<script>.sh`** (or `.ps1`) so there’s no ambiguity. Audit: Hardware_Constraints, System_Architecture_Detail, Regression_Testing_Plan, README, any “quick start” guides.
- [ ] **RPi_Setup_Guide / install path** – Ensure “run from repo root” vs “run from `Documentation/scripts`” is stated consistently (e.g. “From repo root: `./Documentation/scripts/install_hampod.sh`”).

### 1.4 Doc cross-references and index
- [ ] **Documentation/README.md** – Keep the “Project_Spec” list up to date when new spec docs are added; consider a one-line purpose for each doc.
- [ ] **Tests index** – Either in Regression_Testing_Plan or a short “Test index” section elsewhere: list every test entry point (unit tests, HAL test, Phase 0, Normal Mode, Frequency Mode, manual radio tests, deprecated) with path and one-line purpose. Reduces “which script do I run?” confusion.

---

## 2. Code clarity

### 2.1 Radio layer (Software2)
- [ ] **Shared “rig level” helpers** – In `radio_setters.c` and `radio_queries.c`, extract the repeated pattern: lock → check `g_connected` / `g_rig` → call Hamlib → unlock → handle result. Implement e.g. `radio_get_level_float(..., float *out)` and `radio_set_level_float(..., float val)` (or similar) and use them in all level get/set functions to reduce duplication and standardize error handling.
- [ ] **Connection-check helper** – Add something like `radio_ensure_connected()` (or inline helper) that does “if (!g_connected || !g_rig) { unlock; return -1; }” so call sites are one line and logging can be centralized.
- [ ] **Error handling** – Where Hamlib `retcode != RIG_OK` is checked repeatedly, consider a small helper (e.g. `radio_check_ok(retcode)` or `rig_result_to_errno`) so logging and return values are consistent and easier to change later.

### 2.2 Config (Software2)
- [ ] **Optional** – Document the get/set pattern in `config.c` (e.g. in a short comment block or in Refactor_Todos) so future contributors know the intended style. No mandatory code change unless you introduce a generic get/set layer later.

### 2.3 Comm (Software2)
- [ ] **Section comments and docblocks** – Keep existing `// ==========` and function comments; add or refresh a brief module docblock at the top of `comm.c` describing: router thread, response queues, packet types, and how to add a new packet type. Helps anyone touching IPC.
- [ ] **Optional** – If `comm.c` grows further, consider splitting “response queue” logic into a separate file (e.g. `comm_queue.c`) so packet I/O and queue logic are easier to reason about.

### 2.4 Firmware / HAL
- [ ] **Optional** – In Firmware, if keypad and audio pipe/thread setup keeps duplicating, introduce a small shared “pipe lifecycle” or “thread runner” helper to reduce copy-paste and clarify intent.
- [ ] **hal_audio_usb.c** – If the file continues to grow, consider splitting “device enumeration / selection” from “playback / streaming” so each file has a single clear responsibility.

### 2.5 Naming and logging
- [ ] **Hot paths** – Avoid single-letter or opaque variable names in performance-critical or frequently-read code; use names that make the role obvious.
- [ ] **Logging** – Standardize use of `LOG_ERROR` / `DEBUG_PRINT` (or whatever the project uses) so log format is consistent and grep-friendly; document the convention in a short “Logging” note in README or a dev guide.

### 2.6 Dead or redundant code
- [ ] **Backups and duplicates** – Identify and either remove or clearly mark as “backup / deprecated” (e.g. `keypad_firmware_backup.c`). If kept, add a one-line comment at top: “Deprecated backup; do not use for new changes.”
- [ ] **Unused scripts** – Scripts in `deprecated_tests/` (e.g. `Regression_Imitation_Software.sh`): either document in the test index as “deprecated – use … instead” or remove if no longer needed.

---

## 3. Repository and structure clarity

### 3.1 Legacy Software/
- [ ] **Decision** – Choose one: (a) Keep `Software/` as reference and document it clearly as “Legacy (reference only; active app is Software2),” or (b) Move it to something like `archive/Software/` or `legacy/Software/` and update any doc or script that still points there. Then update README and Documentation/README accordingly.
- [ ] **Build scripts** – Ensure `remote_install.sh` and any CI/deploy scripts build **Firmware** and **Software2** only (or document that building Software is optional/legacy). Avoid implying that building Software is required for normal development.

### 3.2 Config and paths
- [ ] **One config path** – Everywhere (README, Project_Spec, scripts, comments): “The main config file is `Software2/config/hampod.conf`.” No mixed use of `Configs.txt` for the active app without a “legacy” label.

### 3.3 Entry points
- [ ] **Run instructions** – README and any “how to run” doc should give exactly one canonical way to run the app (e.g. `./Documentation/scripts/run_hampod.sh` from repo root), with “build manually” as an alternative. Avoid multiple conflicting “run from X” instructions.

---

## 4. Test clarity

### 4.1 Test index
- [ ] **Single list** – Create or update one place (e.g. in Regression_Testing_Plan or Documentation/README) that lists: unit tests (`Software2/tests/`), HAL integration (`Firmware/hal/tests/test_hal_integration`), Phase 0 (`Documentation/scripts/Regression_Phase0_Integration.sh`), Normal Mode, Frequency Mode, manual radio tests, and deprecated tests—with paths and one-line purpose each.

### 4.2 Required vs optional
- [ ] **Pre-commit** – Document clearly: “Before committing, run at least `Regression_Phase0_Integration.sh` (and `Regression_HAL_Integration.sh` if Firmware/HAL changed).” Stating this in README or a “Contributing” section reduces ambiguity.

### 4.3 Unit tests for refactor safety
- [ ] **Config** – Add or extend unit tests for `config.c` (load, save, undo, defaults) so future refactors don’t silently break behavior.
- [ ] **Comm** – Add or extend unit tests for `comm.c` (e.g. response queue push/pop, timeout behavior) so queue and timeout logic can be changed with confidence.

---

## 5. Build and deploy clarity

### 5.1 Dependencies
- [ ] **Single list** – Ensure `Documentation/scripts/install_hampod.sh` is the single place that lists all required packages (or references a small “dependencies” section in docs). Project_Spec and Hardware_Constraints can point to “see install_hampod.sh” for the exact list.
- [ ] **Build order** – Document once: “Build Firmware first, then Software2.” Mention in README “Development” section and in any script that builds both.

### 5.2 Remote install / deploy
- [ ] **remote_install.sh / .ps1** – Document what they do: e.g. “Commit, push, pull on Pi, then build Firmware and Software2.” Clarify that they do **not** build legacy Software unless otherwise intended.
- [ ] **test_and_deploy.ps1** – If it still builds both Software and Software2, add a comment that Software build is legacy/optional, or change it to build only Software2 for normal use.

---

## 6. Minor cleanup

### 6.1 Typos and small doc fixes
- [ ] **Project_Plan.md** – Fix the stray double asterisks in the code readability table (e.g. `speech.c**`, `frequency_mode.c**`).
- [ ] **Broken or local paths** – Search docs for file paths that are machine-specific (e.g. `c:/Users/wayne/...`) and replace with repo-relative paths (e.g. `Software2/src/comm.c`) or generic wording.

### 6.2 README freshness
- [ ] **README “Last updated”** – Set or maintain a “Last updated” (or “Doc last reviewed”) date when making larger doc or structure changes.
- [ ] **Documentation section** – In root README, add a link to **Project_Spec** (e.g. “Project specification and plan: Documentation/Project_Spec/”) so key design and refactor docs are one click away.

---

## 7. Priority overview

| Priority | Focus | Example items |
|----------|--------|----------------|
| **High** | No wrong mental model | “Software2 is active, Software is legacy” in one place; Project_Spec aligned or explicitly labeled; one config path. |
| **High** | Can run and test without guessing | Single “how to run” and “which test to run”; script paths always `Documentation/scripts/...`; test index. |
| **Medium** | Less duplication, safer refactors | Radio helpers; unit tests for config and comm; optional comm/HAL splits. |
| **Low** | Polish | Naming/logging convention doc; deprecated script list; typo and path cleanup. |

---

*Update this file as items are completed or new clarity gaps are found.*
