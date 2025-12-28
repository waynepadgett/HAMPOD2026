# Manual Tests for HAMPOD Modes

This document contains manual test procedures for verifying the functionality of Normal Mode, Frequency Mode, and Set Mode on the HAMPOD device.

**Last Updated:** 2025-12-27

> [!IMPORTANT]
> This document has been revised to align with **ICOMReader_Manual_v106.txt** specifications.
> Multiple tests have been corrected from the previous version.
> Items marked with ⚠️ **CODE CORRECTION NEEDED** indicate that the current implementation likely does not match the specification and should be reviewed.

---

## Prerequisites

Before running these tests:

1. **Hardware Setup:**
   - Raspberry Pi connected to IC-7300 via USB cable
   - IC-7300 powered on with USB CI-V enabled
   - 16-key keypad connected to RPi GPIO
   - Audio output (speaker/headphones) connected

2. **Software Running:**
   - Firmware running: `cd ~/HAMPOD2026/Firmware && ./firmware.elf &`
   - Software running: `cd ~/HAMPOD2026/Software2 && ./bin/hampod`

3. **Verify Radio Connection:**
   - Check for `/dev/ttyUSB0`: `ls -la /dev/ttyUSB0`
   - Software should announce "Ham Pod" and then the ICOM identification number on startup

---

## Normal Mode Tests

Normal Mode is the default operating mode for basic radio operations.

> [!NOTE]
> Per ICOMReader_Manual_v106.txt, the `[A]` key is the **SHIFT** key in Normal Mode.
> A "Hold" key action means pressing and continuing to hold the key for an additional 0.5 seconds.

### Test N1: VFO Selection (Key [1])

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[1]` | Selects VFO A and announces frequency |
| 2 | Hold `[1]` | Selects VFO B and announces frequency |

**Reference:** ICOMManual lines 60-63

### Test N2: Frequency Query (Key [2])

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[2]` | Announces current working VFO frequency |
| 2 | Hold `[2]` | Toggles Memory Scan On/Disabled |

**Reference:** ICOMManual lines 68-69

### Test N3: Split Mode (Key [3])

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[3]` | Toggle Split Mode On/Disable |
| 2 | Hold `[3]` | Exchange VFO A and VFO B |

**Reference:** ICOMManual lines 75-76

⚠️ **CODE CORRECTION NEEDED:** Previous test incorrectly showed `[3]` as S-meter query.

### Test N4: Shift Key (Key [A])

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[A]` | Toggles Shift state on/off, announces "Shift" |
| 2 | Press `[A]` again | Shift state turns off |
| 3 | Hold `[A]` | Volume Up (shortcut key to increase HamPod volume) |

**Reference:** ICOMManual lines 81-84

⚠️ **CODE CORRECTION NEEDED:** Previous test incorrectly showed `[A]` as PTT function. The original HamPod does not have PTT control. `[A]` is the Shift key.

### Test N5: PreAmp Status (Key [4])

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[4]` | Announces PreAmp status |
| 2 | Hold `[4]` | Announces AGC status (slow, mid, fast) and level |

**Reference:** ICOMManual lines 86-91

⚠️ **CODE CORRECTION NEEDED:** Previous test incorrectly showed `[4]` as power level query.

### Test N6: Read Current Mode (Key [0])

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[0]` | Announces current Mode (e.g., "USB", "LSB", "CW") |
| 2 | Hold `[0]` | Toggles Data Mode On/Off on applicable radios |

**Reference:** ICOMManual lines 146-148

### Test N7: Noise Blanker Query (Key [7])

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[7]` | Announces Noise Blanker status and Level |
| 2 | Hold `[7]` | Announces Antenna Tuner status (Off, On, Tuning) |

**Reference:** ICOMManual lines 113-117

### Test N8: Noise Reduction Query (Key [8])

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[8]` | Announces Noise Reduction status and Level |
| 2 | Hold `[8]` | Announces Mic Gain Level |

**Reference:** ICOMManual lines 119-125

### Test N9: Power Level Query (Key [9])

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[9]` | Announces Auto Notch status, Manual Notch status and Frequency if enabled |
| 2 | Hold `[9]` | Announces Power Level |

**Reference:** ICOMManual lines 127-133

### Test N10: S-Meter Query (Key [*])

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[*]` | Reads S-Meter |
| 2 | Hold `[*]` | Reads RF Power Meter |
| 3 | Press `[Shift]+[*]` | Reads Compression Meter |
| 4 | Hold `[Shift]+[*]` | Reads ALC Meter |

**Reference:** ICOMManual lines 141-144

### Test N11: Frequency Announcement Toggle (Key [C])

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[C]` | Toggles Frequency Announcement Delay Verbosity on/disable |
| 2 | Hold `[C]` | Enters Configuration Mode |

**Reference:** ICOMManual lines 135-138

⚠️ **CODE CORRECTION NEEDED:** Previous test incorrectly showed `[C]` as volume control. The original HamPod uses `[A] Hold` for Volume Up and `[B] Hold` for Volume Down.

### Test N12: Verbosity Toggle (Key [D])

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[D]` | Toggles Verbosity on/disable (limits labeling of announcements) |
| 2 | Hold `[D]` | Switch between serial ports 1 and 2 |

**Reference:** ICOMManual lines 158-162

⚠️ **CODE CORRECTION NEEDED:** Previous test incorrectly showed `[D]` as mode cycling.

### Test N13: Volume Control (Keys [A] Hold, [B] Hold)

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Hold `[A]` | Increases HamPod volume |
| 2 | Hold `[B]` | Decreases HamPod volume |

**Reference:** ICOMManual lines 82, 109

### Test N14: Set/Band Key (Key [B])

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[B]` | Enters Set Mode (or rotates between Set/Band/Off on supported models) |
| 2 | Hold `[B]` | Volume Down (shortcut key) |

**Reference:** ICOMManual lines 108-111

---

## Frequency Mode Tests

Frequency Mode allows direct frequency entry via the keypad.

> [!NOTE]
> Per ICOMReader_Manual_v106.txt, the `[*]` key functions as the DECIMAL POINT key in this mode.
> The `[#]` key is the ENTER key.
> The `[D]` key functions as a clear key.

### Test F1: Entry and VFO Selection

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[#]` | Enters Frequency Mode. If previously entered, same VFO is pre-selected |
| 2 | Press `[#]` again | Cycles to next VFO: VFO A → VFO B → Current VFO → VFO A... |
| 3 | Press `[*]` twice | Returns to Normal Mode (second decimal clears entry) |
| 4 | Press `[D]` | Clear key - clears entry and returns to Normal Mode |

**Reference:** ICOMManual lines 170-176

### Test F2: Manual Frequency Entry with Decimal

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[#]` | Enters Frequency Mode |
| 2 | Press `[#]` again | Selects VFO A |
| 3 | Press `[1]` `[4]` | Announces each digit |
| 4 | Press `[*]` (decimal) | Announces "point" |
| 5 | Press `[0]` `[7]` `[4]` | Announces each digit |
| 6 | Press `[#]` (enter) | Sets frequency, announces frequency |
| 7 | Verify on radio display | Should show 14.074.00 MHz |

**Reference:** ICOMManual lines 170-176

### Test F3: Auto-Decimal Frequency Entry (4-5 digits)

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[#]` | Enters Frequency Mode |
| 2 | Press `[1]` `[4]` `[0]` `[2]` `[5]` | Announces each digit (no decimal needed) |
| 3 | Press `[#]` | Auto-inserts decimal, sets and announces frequency |
| 4 | Verify on radio display | Should show 14.025.00 MHz |

**Additional Auto-Decimal Test Cases:**

| Input | Expected Frequency |
|-------|-------------------|
| `7074` | 7.074 MHz |
| `14074` | 14.074 MHz |
| `21200` | 21.200 MHz |
| `28400` | 28.400 MHz |

> [!TIP]
> The auto-decimal feature is a convenience in the HAMPOD implementation. The ICOMManual describes explicit decimal entry.

### Test F4: Frequency Entry Cancellation

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[#]` | Enters Frequency Mode |
| 2 | Press `[1]` `[4]` | Announces digits |
| 3 | Press `[D]` | Clear key - announces "Cancelled", exits to Normal Mode |
| 4 | Verify on radio | Frequency unchanged |

**Reference:** ICOMManual line 175

### Test F5: Frequency Entry Timeout

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[#]` | Enters Frequency Mode |
| 2 | Enter some digits | Announces digits |
| 3 | Wait timeout period (if enabled) | If Key Timeout option is enabled, announces timeout and exits to Normal Mode |

**Reference:** ICOMManual line 176

> [!NOTE]
> Key Timeout Duration is a configuration option (disable / 5-30 seconds). Test behavior depends on configuration.

### Test F6: Special Frequency 777

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[#]` | Enters Frequency Mode |
| 2 | Enter `[7]` `[7]` `[7]` | Announces digits |
| 3 | Press `[#]` | Announces product information and firmware version |

**Reference:** ICOMManual line 178

### Test F7: Special Frequency 999

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[#]` | Enters Frequency Mode |
| 2 | Enter `[9]` `[9]` `[9]` | Announces digits |
| 3 | Press `[#]` | Restores HamPod EEPROM to factory defaults and performs reset |

**Reference:** ICOMManual line 179

> [!CAUTION]
> Test F7 will reset all configuration settings. Use with care.

---

## Set Mode Tests

Set Mode allows adjustment of radio parameters like power, mic gain, noise blanker, etc.

> [!IMPORTANT]
> Per ICOMReader_Manual_v106.txt Section 3:
> - `[B]` rotates from Set to Band to Off (or just Set/Off if no band stacking registers)
> - Use the **same keys as Normal Mode** to select parameters (e.g., `[9] Hold` for Power)
> - `[A]` and `[B]` increment/decrement, enable/disable, or rotate values
> - `[C]` is used for parameters requiring additional data selection
> - `[#]` sends the command to the radio after entering a value

### Test S1: Enter and Exit Set Mode

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[B]` | Announces "Set" (enters Set Mode) |
| 2 | Press `[B]` again | If band stacking available: "Band". Otherwise exits Set Mode |
| 3 | Press `[B]` again | Exits Set Mode (announces "Off" or similar) |
| 4 | Press `[*]` or `[D]` | Also exits Set Mode |

**Reference:** ICOMManual lines 182-192

⚠️ **CODE CORRECTION NEEDED:** Previous test said [B] announces "Set Mode, Power" - ICOMManual just says "Set".

### Test S2: Power Level Setting

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[B]` | Enters Set Mode |
| 2 | Hold `[9]` | Reads current Power Level (same as Normal Mode query) |
| 3 | Enter `[5]` `[0]` | Announces each digit |
| 4 | Press `[#]` | Sends command to radio, confirms "Power set to 50" |
| 5 | Verify on radio | Power level should show 50W |

**Reference:** ICOMManual lines 128-129

### Test S3: Mic Gain Setting

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[B]` | Enters Set Mode |
| 2 | Hold `[8]` | Selects Mic Gain for setting, announces current level |
| 3 | Enter new value | Announces each digit |
| 4 | Press `[#]` | Sends command to radio |

**Reference:** ICOMManual lines 121-122

### Test S4: Noise Blanker Setting

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[B]` | Enters Set Mode |
| 2 | Press `[7]` | Selects Noise Blanker (press, not hold) |
| 3 | Press `[A]` | Enable Noise Blanker (or toggle on) |
| 4 | Press `[B]` | Disable Noise Blanker (or toggle off) |
| 5 | Enter level digits | Sets NB level |
| 6 | Press `[#]` | Sends command to radio |

**Reference:** ICOMManual lines 113-114

> [!NOTE]
> The `[A]` and `[B]` keys enable/disable toggle parameters in Set Mode.

### Test S5: Noise Reduction Setting

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[B]` | Enters Set Mode |
| 2 | Press `[8]` | Selects Noise Reduction |
| 3 | Press `[A]` | Enable Noise Reduction |
| 4 | Press `[B]` | Disable Noise Reduction |
| 5 | Enter level digits | Sets NR level |
| 6 | Press `[#]` | Sends command to radio |

**Reference:** ICOMManual lines 119-120

### Test S6: PreAmp Setting

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[B]` | Enters Set Mode |
| 2 | Press `[4]` | Selects PreAmp for modification |
| 3 | Press `[1]` | Immediately turns PreAmp 1 on, disables PreAmp 2 |
| 4 | Press `[2]` | Immediately turns PreAmp 2 on, disables PreAmp 1 |
| 5 | Press `[0]` | Disables all PreAmps |
| 6 | Press `[#]` or `[D]` | Exits Set Mode |

**Reference:** ICOMManual lines 215-221

> [!NOTE]
> PreAmp changes are immediate - no need to press `[#]` to confirm.
> HamPod remains in Set Mode after setting PreAmp so you can experiment.

### Test S7: AGC Setting

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[B]` | Enters Set Mode |
| 2 | Hold `[4]` | Selects AGC for modification |
| 3 | Hold `[1]` | Immediately selects Fast AGC speed |
| 4 | Hold `[2]` | Immediately selects Mid AGC speed |
| 5 | Hold `[3]` | Immediately selects Slow AGC speed |
| 6 | Enter level digits | Sets AGC level (0-13) |
| 7 | Press `[#]` | Sends command to radio and exits Set Mode |

**Reference:** ICOMManual lines 240-248

### Test S8: Attenuation Setting

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[B]` | Enters Set Mode |
| 2 | Press `[Shift]+[4]` (i.e., `[A]` then `[4]`) | Selects Attenuation for modification |
| 3 | Enter 1-2 digits | Sets attenuation level (typically 0-45) |
| 4 | Press `[#]` | Sends command to radio and exits Set Mode |

**Reference:** ICOMManual lines 223-228

> [!NOTE]
> Valid attenuation values vary by radio model. See ICOMManual for specifics.

### Test S9: Mode Setting

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[B]` | Enters Set Mode |
| 2 | Press `[0]` | Selects Mode for modification |
| 3 | Use `[A]`/`[B]` or digits | Cycles/selects mode |
| 4 | Press `[#]` | Confirms and exits |

**Reference:** ICOMManual lines 146-148, 204

### Test S10: Compression Setting

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[B]` | Enters Set Mode |
| 2 | Press `[Shift]+[9]` | Selects Compression for modification |
| 3 | Press `[A]` | Enable Compression |
| 4 | Press `[B]` | Disable Compression |
| 5 | Enter level digits | Sets Compression level (0-10 for most, 0-100 for 756Pro series) |
| 6 | Press `[#]` | Sends command to radio |

**Reference:** ICOMManual lines 130-131

### Test S11: Shift Key in Set Mode

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[B]` | Enters Set Mode |
| 2 | Press `[A]` | Activates Shift (for shifted key functions) |
| 3 | Press shifted key (e.g., `[4]` for attenuation) | Performs shifted action |
| 4 | Shift state auto-clears after one key |

**Reference:** ICOMManual lines 81-84

---

## Integration Tests

These tests verify that modes work together correctly.

### Test I1: Mode Transitions

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Start in Normal Mode | Basic key functions work |
| 2 | Press `[#]` | Enter Frequency Mode |
| 3 | Press `[*]` twice or `[D]` | Exit back to Normal Mode |
| 4 | Press `[B]` | Enter Set Mode |
| 5 | Press `[*]` or `[D]` | Exit back to Normal Mode |

### Test I2: Mode Priority

| Step | Action | Expected Result |
|------|--------|-----------------|
| 1 | Press `[#]` to enter Frequency Mode | In Frequency Mode |
| 2 | Press `[B]` | Should be ignored (Frequency Mode active) |
| 3 | Press `[D]` to cancel | Back to Normal Mode |
| 4 | Press `[B]` to enter Set Mode | In Set Mode |
| 5 | Press `[#]` | Should confirm value/exit parameter (not enter Freq Mode) |

### Test I3: Radio Connection Required Tests

⚠️ **Note:** These tests require the radio to be connected (`/dev/ttyUSB0` present).

| Test | Action | Expected Result |
|------|--------|-----------------|
| Query Frequency | Press `[2]` in Normal Mode | Current frequency announced |
| Set Frequency | Enter frequency in Freq Mode | Radio tunes to frequency |
| Set Power | Hold `[9]` in Set Mode, enter value, press `[#]` | Radio power level changes |

---

## Summary of Code Corrections Needed

> [!WARNING]
> The following areas of the current implementation likely need review and correction to match ICOMReader_Manual_v106.txt:

### Normal Mode (`normal_mode.c`)
1. **`[C]` key** - Currently implemented as volume/announcements toggle. Per ICOMManual, `[C]` should toggle Frequency Announcement Delay, and `[C] Hold` enters Configuration Mode.
2. **`[D]` key** - Not fully implemented. Per ICOMManual, `[D]` toggles Verbosity.
3. **Missing keys** - Many Normal Mode query functions not implemented: `[3]` Split, `[4]` PreAmp, `[5]` Tone, `[6]` Filter, `[7]` NB, `[8]` NR, `[9]` Notch, etc.
4. **Volume control** - Should be `[A] Hold` = Volume Up, `[B] Hold` = Volume Down.

### Set Mode (`set_mode.c`)
1. **Parameter selection** - Current implementation uses `[#]` to navigate parameters. ICOMManual uses the same keys as Normal Mode (e.g., `[9] Hold` for Power, `[4]` for PreAmp).
2. **Entry announcement** - Current implementation may announce "Set Mode, Power". ICOMManual just says "Set".
3. **Value confirmation** - ICOMManual uses `[#]` only to send value to radio after entry, not for navigation.

### Frequency Mode (`frequency_mode.c`)
1. Generally correct, but VFO cycle should match spec exactly (VFO A → VFO B → Current VFO).

---

## Troubleshooting

### Radio Not Connected

If tests requiring radio fail:

1. Check USB connection: `ls -la /dev/ttyUSB0`
2. Check radio power and CI-V USB setting
3. Check software log: `tail -f /tmp/hampod.log`

### No Audio Output

1. Check speaker/headphone connection
2. Verify volume: Hold `[A]` to increase HamPod volume
3. Check ALSA: `aplay -l` to list audio devices

### No Keypad Response

1. Check GPIO connections
2. Verify Firmware is running: `ps aux | grep firmware`
3. Check pipes exist: `ls -la ~/HAMPOD2026/Firmware/Firmware_*`

---

## Test Results Log

Use this section to record test results:

| Date | Tester | Test ID | Pass/Fail | Notes |
|------|--------|---------|-----------|-------|
| | | | | |
| | | | | |
| | | | | |
