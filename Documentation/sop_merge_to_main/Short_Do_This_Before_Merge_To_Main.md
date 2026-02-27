# Pre-Merge Checklist (Short Version)

For the full SOP with detailed explanations, see [DO_THIS_BEFORE_MERGE_TO_MAIN.md](DO_THIS_BEFORE_MERGE_TO_MAIN.md).

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
cd ~/HAMPOD2026/Software2 && make tests && ./run_all_unit_tests.sh --all
```

All 4 unit tests + radio test must pass (5 total).

---

## 2. Phase 1 — Frequency Mode Regression (follow on-screen instructions)

```bash
cd ~/HAMPOD2026 && sudo bash ./Documentation/scripts/Regression_Phase_One_Manual_Radio_Test.sh
```

---

## 3. Phase 2 — Normal Mode Regression (follow on-screen instructions)

```bash
cd ~/HAMPOD2026 && sudo bash ./Documentation/scripts/Regression_Phase_Two_Manual_Test.sh
```

---

## Before Merge

- [ ] All tests above pass
- [ ] Code builds cleanly (`make` in both Firmware and Software2)
- [ ] `main` is up to date: `git pull origin main` and rebase/merge as needed
