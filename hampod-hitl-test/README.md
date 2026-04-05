# hampod-hitl-test

Automated Hardware-in-the-Loop (HITL) test infrastructure for HAMPOD2026.

These tests run on the self-hosted Raspberry Pi 5 CI runner. They use virtual
hardware (audio loopback, synthetic keypad input) to verify system behavior
automatically, without requiring a human to listen or press keys.

---

## Pi Setup Requirements

The following must be present on the runner Pi for these tests to work:

- **snd-aloop** kernel module loaded (`sudo modprobe snd-aloop`)
- **whisper.cpp** built and accessible (for transcribing TTS output)
- **Piper TTS** installed with the HAMPOD model (same as production)
- **USB keypad** connected (for tests that use real keypad input)
- **pyserial** or equivalent installed (for synthetic keypad input simulation)
- **hamlib** installed (`libhamlib-utils`) for radio detection and control

Optional (for Job 2 / radio tests):
- IC-7300, TS-570, or TS-2000 connected via USB and powered on

---

## Folder Structure

```
hampod-hitl-test/
├── README.md                 <- This file
├── run_no_radio_tests.sh     <- Entry point for Job 1 (no radio needed)
├── run_radio_tests.sh        <- Entry point for Job 2 (skips if no radio)
├── audio/
│   └── README.md             <- snd-aloop setup and whisper.cpp usage
├── input/
│   └── README.md             <- Keypad input simulation approach
├── radio/
│   └── README.md             <- Radio detection, hamlib probing, dummy backend
└── expected/
    └── README.md             <- Expected transcript fixture files
```

---

## Running Locally

```bash
# Job 1 (always safe to run):
bash hampod-hitl-test/run_no_radio_tests.sh

# Job 2 (will skip gracefully if no radio connected):
bash hampod-hitl-test/run_radio_tests.sh
```
