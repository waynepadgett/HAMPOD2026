# Doyle's Changes — Analysis & Integration Plan

**Date:** 2026-02-22  
**Source repo:** `https://github.com/rhit-liangj/HAMPOD2026-liang2`  
**Target branch:** `feature/pi3-overclock` in the main HAMPOD2026 repo  
**Analyzed by:** Antigravity (AI assistant)

---

## Overview

Doyle made two functional changes in his fork. Both are small, self-contained, and should be straightforward to integrate. His repo is otherwise out-of-date with the current branch — the only differences that matter are the ones listed below.

---

## Change 1: Keypad Remapping (Phone-Style Layout)

### What he changed

**File:** `Firmware/hal/hal_keypad_usb.c`  
**Commit:** `f57b5ba` ("key pad remapping")

A standard USB numeric keypad has **7-8-9 across the top row** (calculator-style). Doyle remapped the keycode-to-symbol table so the keypad behaves like a **phone keypad** (1-2-3 across the top row).

### Exact diff

In the `keymap[]` array (currently starting at line 41 of your file):

| Line | Current (calculator-style) | Doyle's (phone-style) |
|------|---------------------------|----------------------|
| 44   | `{KEY_KP1, '1'}` | `{KEY_KP1, '7'}` |
| 45   | `{KEY_KP2, '2'}` | `{KEY_KP2, '8'}` |
| 46   | `{KEY_KP3, '3'}` | `{KEY_KP3, '9'}` |
| 47   | `{KEY_KP4, '4'}` | `{KEY_KP4, '4'}` *(unchanged)* |
| 48   | `{KEY_KP5, '5'}` | `{KEY_KP5, '5'}` *(unchanged)* |
| 49   | `{KEY_KP6, '6'}` | `{KEY_KP6, '6'}` *(unchanged)* |
| 50   | `{KEY_KP7, '7'}` | `{KEY_KP7, '0'}` |
| 51   | `{KEY_KP8, '8'}` | `{KEY_KP8, '1'}` |
| 52   | `{KEY_KP9, '9'}` | `{KEY_KP9, '2'}` |

The middle row (4-5-6) is unchanged. The top and bottom rows swap so that pressing the physical top-left key produces '1' (mapped via `KEY_KP7 → '1'`), matching phone convention.

### ⚠️ Potential Bug in Doyle's Mapping

Doyle's version maps **KEY_KP7 to '0'**, but his intent was to make the top row read 1-2-3. The expected phone-style mapping should be:

| Physical Position | Keycode | Calculator Symbol | Phone Symbol |
|---|---|---|---|
| Top-left     | KEY_KP7 | '7' | **'1'** |
| Top-center   | KEY_KP8 | '8' | **'2'** |
| Top-right    | KEY_KP9 | '9' | **'3'** |
| Middle-left  | KEY_KP4 | '4' | '4' |
| Middle-center| KEY_KP5 | '5' | '5' |
| Middle-right | KEY_KP6 | '6' | '6' |
| Bottom-left  | KEY_KP1 | '1' | **'7'** |
| Bottom-center| KEY_KP2 | '2' | **'8'** |
| Bottom-right | KEY_KP3 | '3' | **'9'** |
| Bottom (wide)| KEY_KP0 | '0' | '0' |

Doyle maps `KEY_KP7 → '0'` instead of `KEY_KP7 → '1'`. This looks like a mistake — 0 doesn't belong in the top row. **The correct phone-style mapping should be:**

```c
{KEY_KP1, '7'},  // Bottom-left  → 7
{KEY_KP2, '8'},  // Bottom-center → 8
{KEY_KP3, '9'},  // Bottom-right → 9
{KEY_KP4, '4'},  // Middle-left  → 4 (unchanged)
{KEY_KP5, '5'},  // Middle-center → 5 (unchanged)
{KEY_KP6, '6'},  // Middle-right → 6 (unchanged)
{KEY_KP7, '1'},  // Top-left     → 1
{KEY_KP8, '2'},  // Top-center   → 2
{KEY_KP9, '3'},  // Top-right    → 3
```

**Recommendation:** Verify with Doyle whether `KEY_KP7 → '0'` was intentional. Most likely the correct mapping for the top row is `'1'`, `'2'`, `'3'`.

### How to integrate (with compile-time switch)

You want both layouts available with a constant controlling which is active. Here is the recommended approach for `Firmware/hal/hal_keypad_usb.c`:

1. Add a `#define` near the top of the file (after the existing `#define` constants, around line 19):

```c
/* Set to 1 for phone-style keypad (1-2-3 on top row),
   set to 0 for calculator-style keypad (7-8-9 on top row) */
#define KEYPAD_PHONE_LAYOUT 0
```

2. Replace the numeric key entries in the `keymap[]` array (lines 43–52) with:

```c
    /* Numeric keys 0-9 */
    {KEY_KP0, '0'},
#if KEYPAD_PHONE_LAYOUT
    /* Phone-style: 1-2-3 on top, 7-8-9 on bottom */
    {KEY_KP1, '7'},
    {KEY_KP2, '8'},
    {KEY_KP3, '9'},
    {KEY_KP4, '4'},
    {KEY_KP5, '5'},
    {KEY_KP6, '6'},
    {KEY_KP7, '1'},
    {KEY_KP8, '2'},
    {KEY_KP9, '3'},
#else
    /* Calculator-style: 7-8-9 on top, 1-2-3 on bottom (default) */
    {KEY_KP1, '1'},
    {KEY_KP2, '2'},
    {KEY_KP3, '3'},
    {KEY_KP4, '4'},
    {KEY_KP5, '5'},
    {KEY_KP6, '6'},
    {KEY_KP7, '7'},
    {KEY_KP8, '8'},
    {KEY_KP9, '9'},
#endif
```

3. No other files need to change for this feature.

---

## Change 2: VOX Status Query in Normal Mode

### What he changed

This change spans **three files** (not one as originally expected):

| File | Change |
|------|--------|
| `Software2/src/radio_queries.c` | Added `radio_get_vox_status()` function |
| `Software2/include/radio_queries.h` | Added `radio_get_vox_status()` declaration |
| `Software2/src/normal_mode.c` | Added Shift+1 handler to query & announce VOX status |

**Commits:** `d0d5154` through `cfcebb9` (multiple iterations; final state shown below)

### What the VOX feature does

When the user presses **Shift+1** in Normal Mode, the system:
1. Calls `radio_get_vox_status()` which uses Hamlib's `rig_get_func()` with `RIG_FUNC_VOX`
2. Announces one of:
   - "VOX status unavailable" (if radio doesn't support it or isn't connected)
   - "VOX is now on" (if VOX is enabled)
   - "VOX is off" (if VOX is disabled)

### Detailed changes needed

#### File 1: `Software2/src/radio_queries.c`

Add the following function at the end of the file (after `radio_get_power_string`). Note: Doyle also added `#include <stdbool.h>` — this is **not needed** in your version because `stdbool.h` is already included via `radio_queries.h` → `#include <stdbool.h>`.

```c
// ============================================================================
// VOX Operations
// ============================================================================

/**
 * @brief Get VOX (Voice-Operated Transmit) status
 *
 * @return 1 if VOX is on, 0 if off, -1 on error
 */
int radio_get_vox_status(void) {
    pthread_mutex_lock(&g_rig_mutex);

    if (!g_connected || !g_rig) {
        pthread_mutex_unlock(&g_rig_mutex);
        return -1;
    }

    int status = 0;
    int retcode = rig_get_func(g_rig, RIG_VFO_CURR, RIG_FUNC_VOX, &status);

    pthread_mutex_unlock(&g_rig_mutex);

    if (retcode != RIG_OK) {
        DEBUG_PRINT("radio_get_vox_status: %s\n", rigerror(retcode));
        return -1;
    }

    return status ? 1 : 0;
}
```

#### File 2: `Software2/include/radio_queries.h`

Add the following declaration before the `#endif` at the end of the file:

```c
// ============================================================================
// VOX Operations
// ============================================================================

/**
 * @brief Get VOX (Voice-Operated Transmit) status
 *
 * @return 1 if VOX is on, 0 if off, -1 on error
 */
int radio_get_vox_status(void);
```

#### File 3: `Software2/src/normal_mode.c`

Modify the `key == '1'` handler (currently lines 103–126) to add a Shift+1 case *before* the existing press/hold logic:

**Current code (lines 103–126):**
```c
    // [1] - VFO selection
    if (key == '1') {
        if (!is_hold) {
            // Select VFO A
            ...
        } else {
            // Select VFO B
            ...
        }
        return true;
    }
```

**New code:**
```c
    // [1] - VFO selection / [Shift+1] - VOX status query
    if (key == '1') {
        if (is_shifted && !is_hold) {
            // [Shift]+[1] - VOX status query
            int vox = radio_get_vox_status();
            if (vox < 0) {
                speech_say_text("VOX status unavailable");
            } else if (vox == 1) {
                speech_say_text("VOX is on");
            } else {
                speech_say_text("VOX is off");
            }
            return true;
        } else if (!is_hold) {
            // Select VFO A
            ...
        } else {
            // Select VFO B
            ...
        }
        return true;
    }
```

### Style notes

Doyle's code in `normal_mode.c` has some inconsistent indentation (the VOX block is indented differently from the surrounding code). The version above cleans that up to match your existing style.

Also, Doyle's speech text says "VOX is **now** on" — consider whether you want "now" in there. The word "now" could be misleading since this is a *query* (reading the current state), not a *toggle* (changing the state). Recommend: "VOX is on" / "VOX is off".

---

## Summary of Files to Modify

| # | File | Change Type | Notes |
|---|------|-------------|-------|
| 1 | `Firmware/hal/hal_keypad_usb.c` | Modify | Add `#if` for phone/calculator layout switch |
| 2 | `Software2/src/radio_queries.c` | Add function | `radio_get_vox_status()` at end of file |
| 3 | `Software2/include/radio_queries.h` | Add declaration | `radio_get_vox_status()` prototype |
| 4 | `Software2/src/normal_mode.c` | Modify | Add Shift+1 VOX query before existing VFO logic |

---

## Items Requiring Decision

1. **Keypad mapping bug:** Confirm with Doyle whether `KEY_KP7 → '0'` was intentional or should be `KEY_KP7 → '1'`. The analysis above assumes the corrected phone-style mapping.

2. **VOX announcement wording:** "VOX is now on" vs "VOX is on" — recommend dropping "now" since this is a status query, not a toggle.

3. **Default keypad layout:** The `#define KEYPAD_PHONE_LAYOUT 0` defaults to calculator-style. Change to `1` if you want phone-style as the default for new builds.
