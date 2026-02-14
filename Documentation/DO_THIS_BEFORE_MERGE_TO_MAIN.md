# DO THIS BEFORE MERGE TO MAIN

Checklist of required steps before merging a feature branch into `main`. All tests must pass.

---

## 1. Required (Every Merge)

### 1.1 Phase 0 Integration Test
Validates Software2 ↔ Firmware communication, router thread, keypad + speech.

```bash
cd ~/HAMPOD2026
./Documentation/scripts/Regression_Phase0_Integration.sh
```

**Pass criteria:** "Phase zero integration test ready" announced; key press and hold work; no "packet type mismatch" or "bruh" errors.

### 1.2 HAL Integration Test *(if Firmware or HAL changed)*
Validates keypad, audio, and TTS HAL without full stack.

```bash
cd ~/HAMPOD2026
./Documentation/scripts/Regression_HAL_Integration.sh
```

**Pass criteria:** Keypad keys detected and announced; TTS/audio output confirmed by tester.

### 1.3 Unit Tests *(if Software2 changed)*
```bash
cd ~/HAMPOD2026/Software2
make tests
```

**Pass criteria:** All `test_*` targets pass; no memory leaks.

---

## 2. Required for Mode/Radio Changes

Run these if you changed Normal Mode, Frequency Mode, Set Mode, or radio logic:

### 2.1 Normal Mode Regression
```bash
cd ~/HAMPOD2026
./Documentation/scripts/Regression_Normal_Mode.sh
```

### 2.2 Frequency Mode Regression
```bash
cd ~/HAMPOD2026
./Documentation/scripts/Regression_Frequency_Mode.sh
```

### 2.3 Manual Radio Test *(if radio control changed; requires radio connected)*
```bash
cd ~/HAMPOD2026
./Documentation/scripts/Regression_Phase_One_Manual_Radio_Test.sh
```

---

## 3. Optional / Pre-Release

For major releases or before tagging:

- **Full regression suite:** Run all `Documentation/scripts/Regression_*.sh` scripts (exclude `deprecated_tests/`).
- **Firmware packet protocol:** `./Documentation/scripts/run_remote_test.sh` (Imitation Software test).
- **Set Mode / Phase 2 manual test:** `./Documentation/scripts/Regression_Phase_Two_Manual_Test.sh`.

---

## 4. Before Merge

- [ ] All required tests for your change scope pass
- [ ] Code builds: `make` in Firmware and Software2
- [ ] No pipe leaks or stale FIFOs after test runs
- [ ] `main` is up to date: `git pull origin main` and rebase/merge as needed

---

## 5. Quick Reference: What to Run by Change Type

| Change type                  | Phase 0 | HAL   | Unit  | Normal | Freq  | Radio |
|-----------------------------|---------|-------|-------|--------|-------|-------|
| Docs only                   | —       | —     | —     | —      | —     | —     |
| Config/comment only         | —       | —     | —     | —      | —     | —     |
| Software2 (comm, config, etc)| ✓       | —     | ✓     | —      | —     | —     |
| Firmware / HAL              | ✓       | ✓     | —     | —      | —     | —     |
| Mode logic (Normal, Freq)   | ✓       | —     | ✓     | ✓      | ✓     | ✓*    |
| Radio / Hamlib              | ✓       | —     | ✓     | ✓      | ✓     | ✓     |

\* Manual radio test if radio control changed.

---

## Sources

| Document | Purpose |
|----------|---------|
| [Project_Plan.md](Project_Spec/Project_Plan.md) | Validation gates, test pyramid, pre-commit vs pre-release |
| [Regression_Testing_Plan.md](Regression_Testing_Plan.md) | Step-by-step test execution, prerequisites, verification checklists |
| [System_Architecture_Detail.md](Project_Spec/System_Architecture_Detail.md) | Integration/test table, performance targets (§9, §10) |
| [Hardware_Constraints.md](Project_Spec/Hardware_Constraints.md) | Target platforms, dependencies, deployment constraints |
| [Refactor_Todos.md](Project_Spec/Refactor_Todos.md) | Test index and pre-commit documentation improvements |
