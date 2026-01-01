# Currently Implemented Key Functions

This document lists all key press functions that are **currently implemented and functional** in the HAMPOD Software2. Functions are organized by mode, then by key.

**Last Updated:** 2025-12-29

> [!NOTE]
> This document will be updated as new key functions are implemented in future development phases.

---

## Global Keys

These keys work across multiple modes or have consistent behavior:

| Key | Action | Function |
|-----|--------|----------|
| `[A]` | Press | **Shift Toggle** - Activates shift state for shifted key combinations. Press again to deactivate. Announces "Shift" or "Shift off". |

---

## Normal Mode

Normal Mode is the default operating mode. Active when Frequency Mode and Set Mode are not active.

### Basic Queries

| Key | Action | Function |
|-----|--------|----------|
| `[0]` | Press | **Mode Query** - Announces the current operating mode (e.g., "USB", "LSB", "CW"). |
| `[1]` | Press | **VFO A Select** - Selects VFO A and announces the frequency. |
| `[1]` | Hold | **VFO B Select** - Selects VFO B and announces the frequency. |
| `[2]` | Press | **Frequency Query** - Announces the current working frequency (e.g., "14 point 0 4 0 7 0 megahertz"). |
| `[*]` | Press | **S-Meter** - Reads and announces the S-meter reading. |
| `[*]` | Hold | **Power Meter** - Reads and announces the RF power meter reading. |

### Parameter Queries

| Key | Action | Function |
|-----|--------|----------|
| `[4]` | Press | **PreAmp Query** - Announces current pre-amp setting (off, 1, or 2). |
| `[4]` | Hold | **AGC Query** - Announces current AGC setting (Off, Fast, Medium, Slow). |
| `[Shift]+[4]` | Press | **Attenuation Query** - Announces current attenuation level. |
| `[7]` | Press | **Noise Blanker Query** - Announces NB status and level (e.g., "Noise blanker on, level 5"). |
| `[8]` | Press | **Noise Reduction Query** - Announces NR status and level. |
| `[8]` | Hold | **Mic Gain Query** - Announces current mic gain level. |
| `[Shift]+[9]` | Press | **Compression Query** - Announces compression status and level. |
| `[9]` | Hold | **Power Level Query** - Announces current power level (e.g., "Power 50 percent"). |

### Mode Controls

| Key | Action | Function |
|-----|--------|----------|
| `[C]` | Press | **Announcements Toggle** - Toggles automatic frequency/mode announcements on or off. Announces "Announcements on" or "Announcements off". |
| `[B]` | Press | **Enter Set Mode** - Transitions to Set Mode. Announces "Set". |
| `[#]` | Press | **Enter Frequency Mode** - Transitions to Frequency Mode. Announces "Frequency Mode". |

---

## Frequency Mode

Frequency Mode allows direct frequency entry via the keypad. Entered by pressing `[#]` from Normal Mode.

### Entry and Navigation

| Key | Action | Function |
|-----|--------|----------|
| `[#]` | Press (from Normal) | **Enter Frequency Mode** - Announces "Frequency Mode". |
| `[#]` | Press (in Freq Mode) | **Cycle VFO** - Cycles through VFO A → VFO B → Current VFO. Announces the selected VFO. |

### Digit Entry

| Key | Action | Function |
|-----|--------|----------|
| `[0]`-`[9]` | Press | **Enter Digit** - Adds digit to frequency buffer and announces the digit. |
| `[*]` | Press (first) | **Decimal Point** - Inserts decimal point. Announces "point". |
| `[*]` | Press (second) | **Cancel** - Cancels entry and returns to Normal Mode. Announces "Cancelled". |

### Confirmation and Exit

| Key | Action | Function |
|-----|--------|----------|
| `[#]` | Press (after digits) | **Submit Frequency** - Sets the radio to the entered frequency and announces it. Auto-decimal is applied for 4-5 digit entries (e.g., "14025" → 14.025 MHz). |
| `[D]` | Press | **Cancel/Clear** - Clears entry and exits to Normal Mode. Announces "Cancelled". |

### Timeout Behavior

If no key is pressed for 10 seconds while in Frequency Mode, the mode times out and returns to Normal Mode with "Timeout" announcement.

---

## Set Mode

Set Mode allows adjustment of radio parameters. Entered by pressing `[B]` from Normal Mode.

### Mode Entry and Exit

| Key | Action | Function |
|-----|--------|----------|
| `[B]` | Press (from Normal) | **Enter Set Mode** - Announces "Set Mode". |
| `[B]` | Press (in Set Mode Idle) | **Exit Set Mode** - Announces "Set Mode Off". |
| `[D]` | Press | **Cancel/Exit** - If editing, cancels edit. If idle, exits Set Mode. |

### Parameter Selection (from Set Mode Idle)

| Key | Action | Parameter | Function |
|-----|--------|-----------|----------|
| `[9]` | Hold | **Power Level** | Announces current power level (e.g., "Power 50 percent"). |
| `[8]` | Hold | **Mic Gain** | Announces current mic gain level. |
| `[Shift]+[9]` | Press | **Compression** | Announces compression status and level. |
| `[7]` | Press | **Noise Blanker** | Announces NB status and level (e.g., "Noise blanker on, level 5"). |
| `[8]` | Press | **Noise Reduction** | Announces NR status and level. |
| `[4]` | Hold | **AGC** | Announces current AGC setting. |
| `[4]` | Press | **PreAmp** | Announces current pre-amp setting (off, 1, or 2). |
| `[Shift]+[4]` | Press | **Attenuation** | Announces current attenuation level. |
| `[0]` | Press | **Mode** | Selects Mode parameter for editing. |

### Value Entry (in Editing State)

| Key | Action | Function |
|-----|--------|----------|
| `[0]`-`[9]` | Press | **Enter Digit** - Accumulates value and announces the digit. |
| `[#]` | Press | **Confirm Value** - Sends the value to the radio and announces confirmation. Returns to Set Mode Idle. |
| `[*]` | Press | **Clear Value** - Clears the accumulated value. Announces "Cleared". |
| `[D]` | Press | **Cancel Edit** - Cancels editing and returns to Set Mode Idle. Announces "Cancelled". |

### Toggle Controls (NB, NR, Compression)

When editing Noise Blanker, Noise Reduction, or Compression:

| Key | Action | Function |
|-----|--------|----------|
| `[A]` | Press | **Enable** - Enables the feature. Announces "[Feature] on". |
| `[B]` | Press | **Disable** - Disables the feature. Announces "[Feature] off". |

### AGC Speed Controls

When editing AGC (`[4]` Hold):

| Key | Action | Function |
|-----|--------|----------|
| `[1]` | Hold | **Fast AGC** - Sets AGC to Fast. Announces "AGC Fast". |
| `[2]` | Hold | **Medium AGC** - Sets AGC to Medium. Announces "AGC Medium". |
| `[3]` | Hold | **Slow AGC** - Sets AGC to Slow. Announces "AGC Slow". |

### Mode Cycling

When editing Mode (`[0]` press in idle):

| Key | Action | Function |
|-----|--------|----------|
| `[0]` | Press | **Cycle Mode** - Cycles through available modes (USB → LSB → CW → etc.) and announces the new mode. |

---

## Keys Not Yet Implemented

The following keys from the ICOMReader Manual are not yet functional:

### Normal Mode (Planned)
- `[3]` - Split Mode toggle / VFO exchange
- `[5]` - Tone functions
- `[6]` - Filter functions
- `[7]` Hold - Tuner query/control
- `[9]` Press - Notch query
- `[A]` Hold - Volume Up
- `[B]` Hold - Volume Down
- `[C]` Hold - Configuration Mode entry
- `[D]` - Verbosity toggle / Serial port switch

### Frequency Mode (Planned)
- `[777]` - Product info
- `[999]` - Factory reset

### Set Mode (Complete for Current Parameters)
- Additional parameters as needed

---

## Audio Feedback

Audio beeps provide immediate feedback for key presses. Beeps are **configurable** via the `key_beep` setting in `hampod.conf`.

| Event | Beep | Description |
|-------|------|-------------|
| Key Press | 1000Hz, 50ms | Short beep when any key is pressed |
| Key Hold | 700Hz, 50ms | Lower-pitch beep when key is held (500ms threshold) |
| Error | 400Hz, 100ms | Low beep for invalid key or action (Phase 2) |

**Configuration**: Set `key_beep = 1` to enable, `key_beep = 0` to disable.

> [!NOTE]
> Beep audio files must be generated on the RPi using `./Firmware/pregen_audio/generate_beeps.sh` after installing `sox`.

---

## Implementation Status Summary

| Mode | Implemented | Total Planned | Status |
|------|-------------|---------------|--------|
| **Normal Mode** | 16 key functions | ~20+ | � Mostly Complete |
| **Frequency Mode** | All core functions | Core complete | 🟢 Complete |
| **Set Mode** | All core parameters | Core complete | 🟢 Complete |
| **Audio Beeps** | Press/Hold beeps | Press/Hold/Error | 🟡 Partial |

---

## Version History

| Date | Changes |
|------|---------|
| 2026-01-01 | Added Normal Mode parameter query keys ([4], [7], [8], [9] with press/hold/shift) |
| 2025-12-31 | Added Audio Feedback section for key beeps |
| 2025-12-29 | Initial document - cataloged all implemented functions |
