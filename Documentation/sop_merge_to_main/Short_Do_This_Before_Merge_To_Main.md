# Pre-Merge Checklist (Short Version)

For the full SOP with detailed explanations, see [Long_Do_This_Before_Merge_To_Main.md](Long_Do_This_Before_Merge_To_Main.md).

---

## Setup

1. Power on the Pi and the radio.
2. SSH into the Pi:
   ```bash
   ssh hampod3@hampod3.local
   ```

---

## 1. Unit Tests

```bash
cd ~/HAMPOD2026/Software2 && sudo make clean &&make tests && ./run_all_unit_tests.sh --all
```

All 4 unit tests + radio test must pass (5 total).

---

## 2. Phase 1 — Frequency Mode Regression (follow on-screen instructions)

```bash
cd ~/HAMPOD2026/Documentation/scripts/ && sudo bash ./Regression_Phase_One_Manual_Radio_Test.sh
```

---

## 3. Phase 2 — Normal Mode Regression (follow on-screen instructions)

```bash
cd ~/HAMPOD2026 && sudo bash ./Documentation/scripts/Regression_Phase_Two_Manual_Test.sh
```

---

## 4. Phase 3 — Configuration Mode Regression (follow on-screen instructions)

```bash
cd ~/HAMPOD2026 && sudo bash ./Documentation/scripts/Regression_Phase_Three_Manual_Test.sh
```

**Quick Keypad Check:**
1. While `hampod` is running, hold the **`-` (minus)** key to enter Config Mode.
2. Press **`/` (slash/A)** to step through parameters.
3. Hold the **`-` (minus)** key to save and exit, or hold the **`*` (asterisk/B)** key to cancel.

---

## 5. CLI Utilities Verification

1. Run `hampod start` and verify it successfully hands off to the underlying script. Connect and disconnect a radio while it's running to ensure polling rate adjustments are functional. Press Ctrl+C to stop.
2. Run `hampod backup-config`, provide a test name interactively, and verify a backup file is created in `Software2/config/backups`.
3. Open `Software2/config/hampod.conf` and temporarily change "volume" to 99.
4. Run `hampod restore-config`, select the backup you just made, and verify the volume reverted to the original value in `hampod.conf`.
5. Run `hampod clear-cache` and verify it reports success.
6. Run `hampod reset` and verify it reports success. If your layout was set to 'phone', verify it reset itself to 'calculator' default.

---

## Before Merge


- [ ] All tests above pass
- [ ] Code builds cleanly (`make` in both Firmware and Software2)
- [ ] `main` is up to date: `git pull origin main` and rebase/merge as needed
