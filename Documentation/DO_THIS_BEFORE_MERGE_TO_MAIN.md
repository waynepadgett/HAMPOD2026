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

NOTE: regression tests may be out of date. see todo in @refactor_todos.md

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
TODO: check if the imitation sofware test script works, take out if it doesnt. (put in depreciated tests folder)
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

## 6. MANUAL TESTING (CRITICAL)

TODO

make a SOP to walk through all the modes to make sure nothing is broken


---


## SOP: Ham Radio Software Test Before Merging to `main`

**Scope:**  
Manual regression test of the ham radio “handpod” software on Raspberry Pi + radio, before merging any branch into `main`.

---

### 1\. Test Environment Setup

1.1 **Hardware required**

-   Raspberry Pi (Pi 3 or Pi 5 as appropriate)
-   Connected radio (same model as target deployment)
-   Power supply for Pi and radio
-   Keypad connected and working
-   Network access for SSH

1.2 **Pre‑test checks**

-   Confirm radio is physically connected to the Pi as in production.
-   Turn **radio OFF** and **Pi OFF** to start from a known state.

---

### 2\. Boot & Script Startup

2.1 **Power‑on sequence**

1.  Turn **Pi power supply ON**.
2.  Turn **radio ON**.
3.  Wait for the Pi to boot fully.

2.2 **SSH into the Pi**

-   From your workstation:
    
        ssh pi@HAMPAD3   # or the correct hostname/IP
        
    

2.3 **Run the `run_handpod` script**

1.  `cd` to the correct directory (adjust path if needed):
    
        cd /path/to/handpod   # e.g. /home/pi/handpod or similar
        
    
2.  Run the script:
    
        ./run_handpod.sh
        
    
    or
    
        sh run_handpod.sh
        
    
3.  Confirm that the script does **not** fail silently (if it does, treat as a test failure and fix before merge).

2.4 **If system is in a weird state**

-   Reboot cleanly:
    
        sudo reboot
        
    
-   After reboot, repeat 2.2 and 2.3.

---

### 3\. Initial “Ready” State Validation

3.1 **Expected startup behavior**

-   With radio ON and script running, the unit should **speak**:
    -   **“ready”** (or equivalent normal‑ready phrase)
-   It should **not** say “radio not found”.

3.2 **Failure behavior**

-   If the radio is not connected / not powered, you should hear it say:
    -   **“radio not found”** (or equivalent error).
-   If script does nothing / no speech: **test fails** (investigate run script, permissions, logs).

---

### 4\. Frequency Read‑Out & Tuning Knob

4.1 **Tuning knob behavior**

-   Spin the tuning knob.
-   Expected:
    -   It should **read the current frequency out loud** (e.g. “14.25371 megahertz”).
-   If it does not announce frequency with announcements enabled: **test fails**.

---

### 5\. Keypad Function Regression Checks

#### 5.1 Zero key behavior

1.  **Press and hold `0` key.**
2.  Expected spoken output:
    -   It should indicate that **zero is being held**, e.g., “held zero” (exact phrase may vary, but behavior should be consistent with existing implementation).
3.  If no speech or incorrect behavior: **test fails**.

---

#### 5.2 VFO / Frequency Mode (Enter key)

1.  Press **`Enter` key** once:
    -   Expected: it should say **“frequency mode”**.
2.  Press **`Enter` key** again:
    -   Expected: it should **toggle VFO** (between VFO 1 / VFO 2) and behave as in current implementation.

Confirm:

-   The system clearly switches between VFOs.
-   No crashes, hangs, or nonsensical announcements.

---

#### 5.3 Key “4” – AGC and Preamp

Key `4` is the only key with **three** related functions; all must be verified.

1.  **AGC readout**
    
    -   Press `4` once.
    -   Expected: it announces **AGC level** (e.g. “AGC medium”).
    -   If radio AGC is set differently, it should read that actual level instead.
2.  **Preamp status**
    
    -   Use the intended interaction pattern for preamp (press / hold per current design).
    -   Expected spoken output:
        -   “preamp on” / “preamp off” consistent with the radio state.
    -   Confirm that without pressing **set** (see next section), it is **reporting only**, not changing the setting.
3.  **Error handling**
    
    -   If AGC or preamp calls fail, expected audio should still be well‑formed (e.g., not garbled / partial) or there should be a clear spoken error.
    -   If it doesn’t say any AGC/preamp info: **test fails**.

---

### 6\. “Set” (Shift / Star) Behavior and Power Setting

**Key mapping reminder (for this test):**

-   Top row:
    -   **Slash key** on this keypad = **“shift”** (conceptually A/B/C/etc. internal mapping).
    -   **Star `*` key** (top row) = **“set” / cancel** function in software.
-   **Decimal / period `.` key** on keypad:
    -   Used in place of the original star on the handpod phone.
    -   Double‑tap decimal cancels a signal in the current design.

6.1 **Change and confirm power setting**

1.  Press the **`*` (set) key** (top row).
    -   Expected: spoken **“set”**.
2.  **Hold `9` key**.
    -   Expected: it speaks current power setting, e.g. **“power 45 percent”**.
3.  Change power:
    -   Input a new value, e.g.:
        -   `5 5 .` (or `55` followed by the appropriate decimal / confirmation pattern used).
    -   Press `Enter`.
    -   Expected:
        -   It says **“power set to 55”** (or analogous text).
        -   Then **“set off”** when leaving set mode.
4.  Confirm that:
    -   The power setting actually changes on the radio.
    -   The announced value matches the radio’s setting.

If the setting does not take effect or spoken feedback is wrong/incomplete: **test fails**.

---

### 7\. Announcement On/Off Behavior (Minus Key)

7.1 **Turn announcements OFF**

1.  Press the **minus `-` key** (insult key).
2.  Expected:
    -   Announcements are turned **off**.
3.  Spin the tuning knob:
    -   Expected: **no frequency announcement**.

7.2 **Turn announcements ON**

1.  Press **minus `-` key** again (or follow current defined toggle pattern).
2.  Spin the tuning knob:
    -   Expected: frequency is announced again.

Note:

-   Current behavior is **not persisted across restart** (config option not yet implemented). That’s acceptable as long as runtime toggling works consistently.

---

### 8\. Interrupt Behavior (Key 2 + Key 4)

8.1 **Basic interrupt test**

1.  Press key `2` to read frequency.
    -   Expected: it starts reading, e.g. “14.25371 megahertz”.
2.  **While it is speaking**, press key `4`.
    -   Expected:
        -   Frequency announcement should **stop early**.
        -   It should then say **“preamp on”** / “preamp off” (or AGC info, depending on current mapping).

8.2 **Queue vs interrupt timing**

1.  Press key `2` and then **quickly** press key `4` *before* it starts speaking:
    -   Expected:
        -   Frequency readout plays **fully**.
        -   Then it says “AGC medium” or “preamp on/off” (queued behavior).
2.  Press key `2`, wait until speech begins, then press `4`:
    -   Expected:
        -   Frequency readout is **interrupted**.
        -   `4`’s function is spoken instead.

Note:

-   Some delay in generating the frequency text is expected; behavior may differ slightly between Pi 3 and Pi 5. Test passes as long as:
    -   Pre‑speech presses **queue** correctly.
    -   Mid‑speech presses **interrupt** correctly.
    -   No crashes, lockups, or overlapping unreadable speech.

---

### 9\. Frequency Entry & Validation

9.1 **Basic frequency mode entry**

1.  Press **Enter** to enter frequency mode.
    -   System speaks **“frequency mode”**.
2.  Enter a test frequency:
    -   Example 1 (fast entry):
        -   `7 1 2 3 4`
        -   Expect it to resolve and then speak, e.g. **“7.12345 megahertz”** (even if it sounds briefly confused while parsing).
    -   Example 2 (slow, clear entry):
        -   `1 4 . 3 6 9 8 7`
        -   Expect: **“14.36987 megahertz”**.
3.  Confirm:
    -   The **radio actually tunes** to the entered frequency.
    -   Final spoken frequency matches the radio frequency.

If tuning fails, speech is wrong, or it locks up on frequency entry: **test fails**.

---

### 10\. Error & Edge‑Case Observations

During all steps above, watch for:

-   Script crashes or silent exits.
-   Unexpected “radio not found” when radio is on and connected.
-   Garbled or clipped speech.
-   Key presses that do nothing, double‑trigger, or consistently mis‑map.
-   Noticeable regressions in behavior vs. current `main`.

Any such issue discovered should:

1.  Be **documented** in the PR (steps to reproduce and expected vs actual).
2.  Be **fixed** (and re‑tested using this SOP) **before** merging to `main`.

---

### 11\. Exit Criteria for Merge to `main`

Branch is **eligible to merge** only if:

-   All sections (2–9) above pass **without** regressions compared to current `main`.
-   No new silent failures or “radio not found” issues in valid setups.
-   Interrupt, frequency entry, and configuration behavior is consistent and stable.

If any step fails, treat the branch as **not ready**; fix and repeat the SOP from **Section 2**.

Notes on manual testing:
always check the key functions

start by turning on the pi and ssh into the pi the radio and running and turn on the power supply

cd HAMPOD2026/Documentation/scripts
./run_hampod.sh
this restarts the hampod 

spin the knob and check that it reads the frequency correctly

the zero button: (keypress beep then usb)
1 should say vfo(variable frequency ocilator)(can be a or b) and frequency 

enter key: says frequesncy mode
hitting it again changes the requency mode
hit the (deciml/delete) key twice to exit frequency mode (on this keypad)

press 4: preamp off
hold 4: agc medium (could be a diff level) error would be not speaking 
shift 4 is attenuation off 


legend:
shift is the slash 
star key is set


## Sources

| Document | Purpose |
|----------|---------|
| [Project_Plan.md](Project_Spec/Project_Plan.md) | Validation gates, test pyramid, pre-commit vs pre-release |
| [Regression_Testing_Plan.md](Regression_Testing_Plan.md) | Step-by-step test execution, prerequisites, verification checklists |
| [System_Architecture_Detail.md](Project_Spec/System_Architecture_Detail.md) | Integration/test table, performance targets (§9, §10) |
| [Hardware_Constraints.md](Project_Spec/Hardware_Constraints.md) | Target platforms, dependencies, deployment constraints |
| [Refactor_Todos.md](Project_Spec/Refactor_Todos.md) | Test index and pre-commit documentation improvements |


