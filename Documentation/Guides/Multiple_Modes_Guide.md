# Multiple Modes Guide

> **Last Updated:** 2026-06-21

How Normal, Frequency, and Set modes work together.

---

## Overview

The HAMPOD has three core interaction modes. Only one is active at a time (except Set Mode, which layers on top of Normal Mode to enable write operations).

| Mode | What It Does | How to Enter |
|------|-------------|--------------|
| **Normal** | Query radio state, hear auto-announcements | Active on startup; default fallback when no other mode is active |
| **Frequency** | Type a frequency directly via numeric keypad | Press `[#]` (Enter key) |
| **Set** | Adjust radio parameters (power, AGC, etc.) | Press `[B]` (`[*]` on keypad) |

> See the [Key Mapping Reference](../Reference/Currently_Implemented_Keys.md) for which physical key maps to each logical name (A, B, C, D, #, *).

---

## Normal Mode

**The default operating mode.** Active on startup. Whenever you are not in Frequency Mode, Set Mode, or Config Mode, you are in Normal Mode.

**What you can do:**
- **VFO selection** — `[1]` press = VFO A, `[1]` hold = VFO B
- **Queries** — frequency (`[2]`), operating mode (`[0]`), S-meter (`[*]`), preamp/AGC/attenuation (`[4]`), noise blanker (`[7]`), noise reduction (`[8]`), mic gain (`[8]` hold), compression (`Shift+[9]`), power level (`[9]` hold)
- **Auto-announce** — the system detects frequency and mode changes from the radio and announces them automatically
- **Shift** — press `[A]` to toggle shift, giving alternate key functions
- **Announcements** — press `[C]` to toggle auto-announce on/off

Normal Mode is **read-only** by default. To change a radio parameter, you enter Set Mode.

---

## Frequency Mode

**Direct frequency entry.** Entered by pressing `[#]` (Enter key). Used when you want to punch in a specific frequency rather than turning the dial.

**How it works:**
1. Press `[#]` — hear "Frequency Mode"
2. Select a VFO — press `[#]` again to cycle: A → B → Current
3. Enter digits `[0]`–`[9]` — the frequency accumulates as you type
4. Press `[*]` to insert a decimal point (press again to cancel the entry)
5. Press `[#]` to submit — the radio tunes to the entered frequency
6. **Timeout** — if no key is pressed for 10 seconds, the mode cancels automatically

**Exit:** Submit (`[#]`), cancel (`[*]` or `[D]`), or let it time out.

---

## Set Mode

**Adjust radio parameters.** Entered by pressing `[B]` (`[*]` on keypad). While active, all keys are consumed by Set Mode until you explicitly exit.

**How it works:**
1. Press `[B]` — hear "Set", you are now in Set Mode
2. Select a parameter — press the key bound to the parameter you want to change (e.g., hold `[9]` for Power Level)
3. Enter a numeric value by typing digits, or use `[A]` (enable) / `[B]` (disable) for boolean parameters (NB, NR, Compression)
4. Press `[#]` to apply the change and exit Set Mode
5. Press `[*]` at any time to cancel and exit; `[D]` cancels the current edit and returns to parameter selection

**Common parameters:** Power Level, Mic Gain, Compression, Noise Blanker, Noise Reduction, AGC, PreAmp, Attenuation, Operating Mode.

> **Set Mode vs Config Mode:** Set Mode adjusts **radio** parameters (power, AGC, etc.) via the ICOM CAT interface. Config Mode (planned, enter by holding `[C]`) adjusts **HAMPOD system** settings (volume, speech speed, key beep). Different concerns, different entry keys.

---

## Key Routing Priority

When you press a key, the system checks modes in this order. The first one that handles the key wins:

1. **Config Mode** (if active) — highest priority, consumes all keys
2. **Set Mode** (if active) — consumes all keys until you exit
3. **Global keys** — `[A]` toggles shift, `[B]` enters Set Mode
4. **Frequency Mode** — handles keys if a frequency entry is in progress
5. **Normal Mode** — handles remaining unhandled keys (queries, VFO, etc.)
6. **Fallthrough** — announces "Pressed X" or "Held X" if no mode consumed it

This means: if Set Mode is active, all keys route there first. Normal Mode queries won't work until you exit Set Mode. Similarly, Frequency Mode takes priority over Normal Mode while a frequency entry is active.
