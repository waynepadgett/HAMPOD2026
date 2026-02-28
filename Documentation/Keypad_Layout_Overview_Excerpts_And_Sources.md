# Keypad Layout Overview Excerpts and Sources

This file collects raw excerpts, comments, and documentation snippets regarding the keypad mappings, shift states, and layouts to help untangle the HAMPOD keypad interface.

---

## Key Mapping Quick Reference (from Key_Mapping_Process.md)

| Physical Key | Linux Keycode       | HAMPOD Symbol | Original HamPod Function       |
|--------------|---------------------|---------------|--------------------------------|
| 0-9          | `KEY_KP0`-`KEY_KP9` | `'0'`-`'9'`   | Numeric input, frequency entry |
| /            | `KEY_KPSLASH`       | `'A'`         | SHIFT function                 |
| *            | `KEY_KPASTERISK`    | `'B'`         | Band/Set                       |
| -            | `KEY_KPMINUS`       | `'C'`         | Announcements toggle           |
| +            | `KEY_KPPLUS`        | `'D'`         | Verbosity toggle               |
| Enter        | `KEY_KPENTER`       | `'#'`         | Enter/Confirm                  |
| . (DEL)      | `KEY_KPDOT`         | `'*'`         | S-Meter, Cancel                |
| Num Lock     | `KEY_NUMLOCK`       | `'X'`         | Special function               |
| Backspace    | `KEY_BACKSPACE`     | `'Y'`         | Special function               |

---


bug tracking: 
i cant understand what it says when i press the numlock key. apparently it says X 


## Config Mode — Phased Implementation Plan

**Branch**: `feature/config-mode`  
**Goal**: Implement Configuration Mode ([C] Hold) per HLR-029 through HLR-035  
**Pattern**: Modeled after `set_mode.c` state machine

---

### Architecture Overview

```
                          ┌──────────────────────────────────────────┐
                          │            on_keypress (main.c)          │
                          │                                          │
                          │  ┌─ config_mode_is_active()? ──────────┐│
                          │  │  YES: config_mode_handle_key()      ││
                          │  │       return                         ││
                          │  └──────────────────────────────────────┘│
                          │  ┌─ set_mode_is_active()? ─────────────┐│
                          │  │  ...existing routing...             ││
                          │  └──────────────────────────────────────┘│
                          └──────────────────────────────────────────┘

    Config Mode State Machine:
    ┌─────────────┐  [C] Hold   ┌──────────────────┐
    │ CONFIG_OFF  │ ──────────> │ CONFIG_BROWSING  │
    └─────────────┘             │                  │
          ▲                     │  [A] = next param│
          │                     │  [B] = prev param│
          │ [C] Hold = save     │  [C] = increment │
          │ [D] Hold = discard  │  [D] = decrement │
          │ timeout  = discard  │  others = beep   │
          └─────────────────────┘──────────────────┘
```

---

### Phase 1: Core Config Mode Module

Create a new state machine module following the `set_mode.c` pattern exactly.

#### New Files

| File | Purpose |
|------|---------|
| `Software2/src/config_mode.c` | Config Mode state machine, key handling, parameter navigation |
| `Software2/include/config_mode.h` | Public API: enter, exit, is_active, handle_key |

#### API

```c
void config_mode_init(void);
void config_mode_enter(void);                          // → CONFIG_BROWSING
void config_mode_exit_save(void);                       // save to hampod.conf + exit
void config_mode_exit_discard(void);                    // undo all changes + exit
bool config_mode_is_active(void);
bool config_mode_handle_key(char key, bool is_hold);   // returns true if consumed
```

#### State Machine

| State | Key | Action | Speech |
|-------|-----|--------|--------|
| CONFIG_BROWSING | `[A]` press | Next parameter (circular) | param name + value |
| CONFIG_BROWSING | `[B]` press | Previous parameter (circular) | param name + value |
| CONFIG_BROWSING | `[C]` press | Increment current value | new value |
| CONFIG_BROWSING | `[D]` press | Decrement current value | new value |
| CONFIG_BROWSING | `[C]` hold | **Save** to `hampod.conf` + exit | "Configuration saved" |
| CONFIG_BROWSING | `[D]` hold | **Discard** changes + exit | "Configuration cancelled" |
| CONFIG_BROWSING | any other | Error beep, ignored | — |
| CONFIG_BROWSING | timeout | Exit without saving | "Timeout" |

#### Initial Parameter List (3 params for Phase 1)

| Index | Parameter | Config Getter/Setter | Range | Increment | Announce Format |
|-------|-----------|---------------------|-------|-----------|-----------------|
| 0 | Volume | `config_get/set_volume()` | 0–100 | +/- 10 | "Volume, 50 percent" |
| 1 | Speech Speed | `config_get/set_speech_speed()` | 0.5–2.0 | +/- 0.1 | "Speed, 1 point 2" |
| 2 | Key Beep | `config_get/set_key_beep_enabled()` | on/off | toggle | "Key Beep, on" |

> **Design Decision**: The original HAMPOD had 22 parameters. For Phase 1, we implement only the 3 that already have working backend support in `config.c`. Additional params (verbosity, key timeout, freq announce delay) can be added later as Phase 2.

#### Changes to Existing Files

**`Software2/src/normal_mode.c`** (line 263):
```diff
- // [C] - Toggle verbosity (press) / Config mode entry (hold, not implemented)
+ // [C] - Toggle verbosity (press) / Config mode entry (hold)
+ if (key == 'C' && is_hold) {
+   config_mode_enter();
+   return true;
+ }
```

**`Software2/src/main.c`** `on_keypress()`:
```diff
  // Route to config mode first (highest priority when active)
+ if (config_mode_is_active()) {
+   config_mode_handle_key(kp->key, kp->isHold);
+   return;
+ }
  // Route to set mode (if active)
  if (set_mode_is_active()) { ...
```

**`Software2/src/makefile`** (or `CMakeLists.txt`):
- Add `src/config_mode.c` to `SW_SRCS`

#### ✅ Phase 1 Done When
- [ ] `make` compiles cleanly in Software2
- [ ] Hold [C] in Normal Mode enters Config Mode ("Configuration Mode" announced)
- [ ] [A]/[B] step through 3 parameters with circular wrapping
- [ ] [C]/[D] increment/decrement values with audible feedback
- [ ] Hold [C] saves and exits ("Configuration saved")
- [ ] Hold [D] discards and exits ("Configuration cancelled")
- [ ] All other keys produce error beep
- [ ] Shift key (A) does NOT toggle shift in Config Mode
- [ ] Existing unit tests still pass

---

### Phase 2: Runtime Parameter Application

Currently, speech speed and volume are only applied **once at startup** in `main.c`. Config Mode needs them to take effect immediately.

#### Changes to Existing Files

**`Software2/src/main.c`** — Extract `amixer` logic into a reusable helper:
```diff
+ // New helper function
+ void audio_apply_volume(int volume_percent) {
+   int card = config_get_audio_card_number();
+   char vol_cmd[128];
+   if (card >= 0) {
+     snprintf(vol_cmd, sizeof(vol_cmd),
+              "amixer -c %d -q sset PCM %d%% 2>/dev/null", card, volume_percent);
+   } else {
+     snprintf(vol_cmd, sizeof(vol_cmd),
+              "amixer -q sset PCM %d%% 2>/dev/null", volume_percent);
+   }
+   system(vol_cmd);
+ }
```

**`Software2/src/config_mode.c`** — After each value change:
- **Volume**: call `audio_apply_volume(new_val)` so user hears the change immediately
- **Speech Speed**: call `comm_set_speech_speed(new_val)` so Piper adjusts immediately
- **Key Beep**: already works in real-time (checked per-keypress in `keypad.c`) ✅

#### ✅ Phase 2 Done When
- [ ] Changing volume in Config Mode immediately changes audio output level
- [ ] Changing speech speed in Config Mode immediately changes TTS rate
- [ ] After discard, volume + speed revert to previous values audibly

---

### Phase 3: Save/Discard with Undo (Leverage Existing Infrastructure)

The `config.c` module already has:
- **10-deep undo history** via `history_push()`/`history_pop()`
- **Auto-save on every setter call** via `config_write_file()`

This creates a design challenge: each [C]/[D] press auto-saves immediately. To support "discard all changes", we need to snapshot the config on entry and restore on discard.

#### Implementation Strategy

```c
// On config_mode_enter():
//   1. Record how many undo steps are currently available
//   2. Note: each C/D press will auto-save (existing behavior)

// On config_mode_exit_save():
//   1. Call config_save() (already saved by setters, but explicit)
//   2. Announce "Configuration saved"
//   3. Transition to CONFIG_OFF

// On config_mode_exit_discard():
//   1. Call config_undo() N times to revert all changes made during session
//   2. Re-apply volume via audio_apply_volume()
//   3. Re-apply speed via comm_set_speech_speed()
//   4. Announce "Configuration cancelled"
//   5. Transition to CONFIG_OFF
```

#### ✅ Phase 3 Done When
- [ ] Save path persists all changes to `hampod.conf`
- [ ] Discard path reverts all changes made during the session
- [ ] After discard, `hampod.conf` reflects pre-session values
- [ ] Undo count correctly tracked across session

---

### Phase 4: Additional Parameters (Future)

These can be added incrementally by extending the parameter array in `config_mode.c`:

| Parameter | Config Support | Priority |
|-----------|---------------|----------|
| Verbosity | `normal_mode_set_verbosity()` ✅ | High |
| Key Timeout Duration | ❌ needs new config field | Medium |
| Freq Announce Delay | ❌ needs new config field + timing changes | Medium |
| Freq Announce on/off | ❌ needs new config field | Medium |
| Freq Plus Mode | ❌ needs new config field | Low |
| Keypad Layout | `config_get/set_keypad_layout()` ✅ | Low (requires HAL call) |

---

### HLR Traceability

| HLR | Requirement | Phase |
|-----|-------------|-------|
| HLR-029 | Config Mode SHALL allow modification of system params | Phase 1 |
| HLR-030 | Entry via hold C key | Phase 1 |
| HLR-031 | A/B step forward/backward through options | Phase 1 |
| HLR-032 | C/D increment/decrement values | Phase 1 |
| HLR-033 | Save to persistent storage (hold C) | Phase 3 |
| HLR-034 | Exit without saving (hold D) | Phase 3 |
| HLR-035 | Config file updated on save | Phase 3 |
| HLR-063 | Adjustable speech volume | Phase 1 |
| HLR-064 | Adjustable speech speed | Phase 1 |
| HLR-065 | Verbosity control | Phase 4 |
| HLR-066 | Key beep on/off | Phase 1 |
| HLR-067 | Key timeout configuration | Phase 4 |
| HLR-003 | Mode change announcements | Phase 1 |

---

### File Summary

| Phase | File | Action |
|-------|------|--------|
| 1 | `Software2/src/config_mode.c` | NEW |
| 1 | `Software2/include/config_mode.h` | NEW |
| 1 | `Software2/src/normal_mode.c` | MODIFY (add hold-C handler) |
| 1 | `Software2/src/main.c` | MODIFY (add config_mode routing) |
| 1 | `Software2/makefile` | MODIFY (add config_mode.c) |
| 2 | `Software2/src/main.c` | MODIFY (extract audio_apply_volume) |
| 2 | `Software2/src/config_mode.c` | MODIFY (call runtime appliers) |
| 3 | `Software2/src/config_mode.c` | MODIFY (undo tracking) |

---

### Verification Plan

| Phase | Test | How to Run | Success Criteria |
|-------|------|-----------|------------------|
| 1 | Build | `cd ~/HAMPOD2026/Software2 && make clean && make` | Compiles clean |
| 1 | Unit tests | `cd ~/HAMPOD2026/Software2 && make tests && ./run_all_unit_tests.sh --all` | All existing tests pass |
| 1 | Manual: entry | Hold [C] key in Normal Mode | Hear "Configuration Mode" |
| 1 | Manual: nav | Press [A] and [B] keys | Hear parameter names cycle |
| 1 | Manual: adjust | Press [C] and [D] keys | Hear values change |
| 1 | Manual: exit save | Hold [C] | Hear "Configuration saved" |
| 1 | Manual: exit discard | Hold [D] | Hear "Configuration cancelled" |
| 2 | Manual: live volume | Adjust volume in Config Mode | Hear volume change in real-time |
| 2 | Manual: live speed | Adjust speed in Config Mode | Hear TTS rate change in real-time |
| 3 | Manual: discard revert | Change volume, then discard | Volume returns to previous level |
| 3 | Inspect | `cat ~/HAMPOD2026/Software2/config/hampod.conf` | Values match expected state |

---

## 1. Physical to Logical Mappings (The HAL Layer)

The lowest level of mapping occurs in the Hardware Abstraction Layer when reading from the USB Keypad.

*   [Firmware/hal/hal_keypad_usb.c](file:///Users/amberpadgett/Developer/HAMPOD2026/Firmware/hal/hal_keypad_usb.c)
    *   **Excerpt (Layout 0: Calculator):**
        ```c
        /*
         * Calculator-style keymap: 7-8-9 on top, matches USB key labels.
         */
        static const KeymapEntry keymap_calculator[] = {
            {KEY_KP0, '0'}, {KEY_KP1, '1'}, {KEY_KP2, '2'}, {KEY_KP3, '3'},
            {KEY_KP4, '4'}, {KEY_KP5, '5'}, {KEY_KP6, '6'}, {KEY_KP7, '7'},
            {KEY_KP8, '8'}, {KEY_KP9, '9'},
            /* Function keys mapped to A-D */
            {KEY_KPSLASH, 'A'},    /* / → A */
            {KEY_KPASTERISK, 'B'}, /* * → B */
            {KEY_KPMINUS, 'C'},    /* - → C */
            {KEY_KPPLUS, 'D'},     /* + → D */
            /* Special keys */
            {KEY_KPENTER, '#'},   /* ENTER → # */
            {KEY_KPDOT, '*'},     /* . (DEL) → * */
            {KEY_NUMLOCK, 'X'},   /* NUM_LOCK → X */
            {KEY_BACKSPACE, 'Y'}, /* BACKSPACE → Y */
        };
        ```
    *   **Excerpt (Layout 1: Phone):**
        ```c
        /*
         * Phone-style keymap: positional mapping for 19-key USB calculator keypad.
         *
         * USB Physical:              HAMPOD Phone Symbol:
         * ┌────┬────┬────┬────┐     ┌────┬────┬────┬────┐
         * │NumL│ /  │ *  │ BS │     │ -- │ -- │ -- │ A  │
         * ├────┼────┼────┼────┤     ├────┼────┼────┼────┤
         * │ 7  │ 8  │ 9  │ -  │     │ 1  │ 2  │ 3  │ B  │
         * ├────┼────┼────┼────┤     ├────┼────┼────┼────┤
         * │ 4  │ 5  │ 6  │ +  │     │ 4  │ 5  │ 6  │ C  │
         * ├────┼────┼────┤    │     ├────┼────┼────┼────┤
         * │ 1  │ 2  │ 3  │Ent │     │ 7  │ 8  │ 9  │ D  │
         * ├────┼────┼────┤    │     ├────┼────┼────┼────┤
         * │ 0  │ 00 │ .  │    │     │ *  │ 0  │ #  │ D  │
         * └────┴────┴────┴────┘     └────┴────┴────┴────┘
         */
        static const KeymapEntry keymap_phone[] = {
            /* Numeric keys - swapped rows for phone-style positioning */
            {KEY_KP1, '7'}, {KEY_KP2, '8'}, {KEY_KP3, '9'},
            {KEY_KP4, '4'}, {KEY_KP5, '5'}, {KEY_KP6, '6'},
            {KEY_KP7, '1'}, {KEY_KP8, '2'}, {KEY_KP9, '3'},
            /* Right column: A, B, C, D from top to bottom */
            {KEY_BACKSPACE, 'A'}, /* Top-right → A */
            {KEY_KPMINUS, 'B'},   /* - → B */
            {KEY_KPPLUS, 'C'},    /* + → C */
            {KEY_KPENTER, 'D'},   /* Enter → D */
            /* Bottom row (non-KP0 keys) */
            {KEY_KPDOT, '#'}, /* . (DEL) → # */
        };
        ```
    *   **Excerpt ('0' / '00' Disambiguation Logic):**
        > The USB keypad's '0' and '00' keys both send `KEY_KP0`. The '00' key sends two `KEY_KP0` events within ~16-24ms. 
        > In phone mode: single '0' key → '*', double '00' key → '0'.
        
## 2. Shift States and Logical Mappings (Software Layer)

*   [Documentation/Original_Hampod_Docs/ICOMReader_Manual_v106.txt](file:///Users/amberpadgett/Developer/HAMPOD2026/Documentation/Original_Hampod_Docs/ICOMReader_Manual_v106.txt)
    *   **Excerpt (Config Mode):**
        > 7. CONFIGURATION MODE
        > This mode allows changes to the various voice settings as well as modifying the options which effect how the  HamPod operates and interacts with your ICOM transceiver. To enter Configuration Mode, press and hold the [C] key for 1 second.
        > [A]: Step forward through the configuration options.
        > [B]: Step backward through the configuration options.
        > [C]: Increment the parameter values.
        > [D]: Decrements the parameter values.

*   [Documentation/Formal_methods_tangent/High_Level_Requirements.md](file:///Users/amberpadgett/Developer/HAMPOD2026/Documentation/Formal_methods_tangent/High_Level_Requirements.md)
    *   **Excerpt (Shift Modifiers):**
        > [HLR-013] In Normal Mode, each key SHALL have up to four functions accessible via key modifiers: press, shift+press, hold, and shift+hold.
        > [HLR-032] The Shift key (A) SHALL toggle shift state on/off rather than requiring simultaneous key press.
    *   **Excerpt (DTMF Mode):**
        > [HLR-025] DTMF Mode SHALL map keys to standard DTMF tones: 0-9, *, #, A-D (16 tones total).

*   [Firmware/imitation_software.c](file:///Users/amberpadgett/Developer/HAMPOD2026/Firmware/imitation_software.c)
    *   **Excerpt (Mapping Index):**
        ```c
        int index_getter(char keypad_input){
            /* Map keypad character to array index matching keypad_names/DTMF_names layout:
             * 0:'1', 1:'2', 2:'3', 3:'A', 4:'4', 5:'5', 6:'6', 7:'B',
             * 8:'7', 9:'8', 10:'9', 11:'C', 12:'*'(POINT), 13:'0', 14:'#'(POUND), 15:'D'
             */
             // ... switch statement ...
        }
        ```

## 3. Application Routing and Global Keys (Software2/src)

*   [Software2/src/main.c](file:///Users/amberpadgett/Developer/HAMPOD2026/Software2/src/main.c)
    *   **Excerpt (Global Key Logic):**
        ```c
        // Shift state for Set Mode (toggled by [A] key)
        static bool g_shift_active = false;
        
        static void on_keypress(const KeyPressEvent *kp) {
          // ...
          // Route to set mode first (if active, it takes priority for ALL keys including [A])
          if (set_mode_is_active()) {
            if (set_mode_handle_key(kp->key, kp->isHold, was_shifted)) {
              if (was_shifted) g_shift_active = false;
              return;
            }
          }
        
          // Handle [A] key for shift toggle (only when NOT in set mode)
          if (kp->key == 'A' && !kp->isHold) {
            g_shift_active = !g_shift_active;
            speech_say_text(g_shift_active ? "Shift" : "Shift off");
            return;
          }
        
          // [B] key enters Set Mode when not active
          if (kp->key == 'B' && !kp->isHold && !set_mode_is_active()) {
            set_mode_enter();
            if (was_shifted) g_shift_active = false;
            return;
          }
          // ... Routing to frequency_mode or normal_mode
        }
        ```

## 4. Frequency Mode Key Bindings (Software2/src)

Entered by pressing `[#]` from Normal Mode. Uses a state machine: `IDLE → SELECT_VFO → ENTERING`.

*   [Software2/src/frequency_mode.c](file:///Users/amberpadgett/Developer/HAMPOD2026/Software2/src/frequency_mode.c)
    *   **Excerpt (Key handling, extracted from `frequency_mode_handle_key`):**

| State | Key | Action | Speech Feedback |
|-------|-----|--------|-----------------|
| IDLE | `[#]` | Enter Frequency Mode | "Frequency Mode" |
| SELECT_VFO | `[#]` | Cycle VFO: A → B → Current | VFO name |
| SELECT_VFO | `[0]-[9]` | Start digit entry, transition to ENTERING | Digit spoken |
| SELECT_VFO | `[*]` | Cancel, return to IDLE | "Cancelled" |
| SELECT_VFO | `[D]` | Cancel, return to IDLE | "Cancelled" |
| ENTERING | `[0]-[9]` | Accumulate digit (max 11 chars) | Digit spoken |
| ENTERING | `[*]` (first) | Insert decimal point | "point" |
| ENTERING | `[*]` (second) | Cancel, return to IDLE | "Cancelled" |
| ENTERING | `[#]` | Submit frequency to radio | Confirmed freq |
| ENTERING | `[D]` | Cancel, return to IDLE | "Cancelled" |

    *   **Excerpt (Timeout):**
        > Timeout of 10 seconds (`FREQ_MODE_TIMEOUT_SEC`). If no key is pressed within 10 seconds, the mode cancels and announces "Timeout".

*   [Documentation/Original_Hampod_Docs/ICOMReaderManual2.txt](file:///Users/amberpadgett/Developer/HAMPOD2026/Documentation/Original_Hampod_Docs/ICOMReaderManual2.txt)
    *   **Excerpt (Section 5 - Frequency Format Rules):**
        > Enter 14.250 for 14.250 MHz in standard format.
        > Enter 14 with no decimal for 14 whole MHz.
        > Leading dot for kHz entry: .250 = 0.250 MHz = 250 kHz.
        > Pressing pound with an empty buffer cycles VFO selection instead.

    *   **Excerpt (Special Frequencies):**
        > Enter 777 + [#] → Announce firmware version.
        > Enter 999 + [#] → Factory reset (immediate, no confirmation).

## 5. Set Mode Key Bindings (Software2/src)

Entered by pressing `[B]` from Normal Mode. Adjusts **radio** parameters (not HAMPOD system settings — that's Config Mode).

Uses a two-phase state machine: `OFF → IDLE (parameter selection) → EDITING (value entry)`.

*   [Software2/src/set_mode.c](file:///Users/amberpadgett/Developer/HAMPOD2026/Software2/src/set_mode.c)
    *   **Excerpt (Entry/Exit):**
        ```c
        void set_mode_enter(void) {
            g_state = SET_MODE_IDLE;
            speech_say_text("Set");
        }
        void set_mode_exit(void) {
            g_state = SET_MODE_OFF;
            speech_say_text("Set Off");
        }
        ```

    *   **Parameter Selection Keys (SET_MODE_IDLE state):**

| Key | Action | Parameter | Enum Value |
|-----|--------|-----------|------------|
| `[9]` Hold | Select | Power Level | `SET_PARAM_POWER` |
| `[8]` Hold | Select | Mic Gain | `SET_PARAM_MIC_GAIN` |
| `[Shift]+[9]` | Select | Compression | `SET_PARAM_COMPRESSION` |
| `[7]` Press | Select | Noise Blanker | `SET_PARAM_NB` |
| `[8]` Press | Select | Noise Reduction | `SET_PARAM_NR` |
| `[4]` Hold | Select | AGC | `SET_PARAM_AGC` |
| `[4]` Press | Select | PreAmp | `SET_PARAM_PREAMP` |
| `[Shift]+[4]` | Select | Attenuation | `SET_PARAM_ATTENUATION` |
| `[0]` Press | Select | Operating Mode | `SET_PARAM_MODE` |

    *   **Value Entry Keys (SET_MODE_EDITING state):**

| Key | Action | Notes |
|-----|--------|-------|
| `[0]-[9]` | Accumulate digit | Max 8 digits, digit spoken |
| `[#]` | Confirm + apply value | Calls `apply_value()`, exits to Normal |
| `[#]` (empty buffer) | Exit Set Mode | — |
| `[*]` | Cancel edit, exit Set Mode | — |
| `[D]` | Cancel edit, return to IDLE | — |
| `[B]` | Exit or disable toggle param | Context-dependent |
| `[A]` | Enable toggle param (NB/NR/Comp) | — |

    *   **Special Cases:**
        - **AGC editing**: `[1]` Hold = Fast, `[2]` Hold = Medium, `[3]` Hold = Slow
        - **Mode editing**: `[0]` cycles through available modes (USB → LSB → CW → ...)
        - **Toggle parameters (NB, NR, Compression)**: `[A]` = enable, `[B]` = disable, instead of numeric entry

*   [Documentation/Original_Hampod_Docs/ICOMReaderManual2.txt](file:///Users/amberpadgett/Developer/HAMPOD2026/Documentation/Original_Hampod_Docs/ICOMReaderManual2.txt)
    *   **Excerpt (Section 6 - Set Mode overview):**
        > Set Mode is not a separate mode but rather a modifier flag that changes how Normal Mode keys behave, specifically enabling write operations instead of just read.
    *   **Note:** The original HAMPOD treats Set Mode as a modifier on Normal Mode. Software2 implements it as a distinct state machine with `SET_MODE_OFF`, `SET_MODE_IDLE`, and `SET_MODE_EDITING` states.

## 6. Configuration Mode — Full Specification (NOT YET IMPLEMENTED)

Configuration Mode adjusts **HAMPOD system settings** (voice params, operational behavior). This is distinct from Set Mode which adjusts **radio** parameters.

### 6.1 Entry and Exit

*   [Documentation/Original_Hampod_Docs/ICOMReader_Manual_v106.txt](file:///Users/amberpadgett/Developer/HAMPOD2026/Documentation/Original_Hampod_Docs/ICOMReader_Manual_v106.txt)
    *   **Excerpt (Section 7):**
        > Hold [C] for 1 second to enter Configuration Mode at the **last-viewed option**.
        > Hold [C] for 1 second again to **save** changes to EEPROM and exit.
        > Hold [D] for 1 second to **discard** changes and exit. Changes remain until power cycle.
        > Keypad timeout exits without saving.

### 6.2 Navigation Keys (A/B/C/D only)

| Key | Function |
|-----|----------|
| `[A]` | Step **forward** through configuration options |
| `[B]` | Step **backward** through configuration options |
| `[C]` | **Increment** the current parameter value |
| `[D]` | **Decrement** the current parameter value |
| `[C]` Hold | **Save** to EEPROM and exit |
| `[D]` Hold | **Discard** changes and exit |
| All others | Error beep, ignored |

> All menus are circular and wrap around.

### 6.3 Original Configuration Parameters (Appendix B)

**Voice Parameters (DoubleTalk synth — maps to Piper TTS on HAMPOD2026):**

| # | Parameter | Range | Default | HAMPOD2026 Equivalent |
|---|-----------|-------|---------|----------------------|
| 1 | Volume | 0–9 | 5 | `config_set_volume()` (0–100 scale) |
| 2 | Speed | 0–13 | 5 | `config_set_speech_speed()` (0.5–2.0 float, Piper `--length_scale`) |
| 3 | Pitch | 0–99 | 50 | ❌ Not applicable (Piper model-dependent) |
| 4 | Articulation | 0–9 | 3 | ❌ Not applicable |
| 5 | Expression | 0–9 | 3 | ❌ Not applicable |
| 6 | Reverb | 0–9 | 0 | ❌ Not applicable |
| 7 | Formant Frequency | 0–99 | 50 | ❌ Not applicable |
| 8 | Tone | 0–2 | 1 | ❌ Not applicable |
| 9 | Voice preset | 0–10 | 0 | ❌ (could map to Piper model selection) |

**Operational Parameters:**

| # | Parameter | Range | Default | HAMPOD2026 Equivalent |
|---|-----------|-------|---------|----------------------|
| 10 | Key Beep | on/off | on | `config_set_key_beep_enabled()` ✅ |
| 11 | Key Timeout Duration | off / 5–30s | — | ❌ Not implemented |
| 12 | Verbosity | on/off | on | Partial: `normal_mode_set_verbosity()` ✅ |
| 13 | Frequency Announcement | on/off | on | Partial: tied to verbosity ✅ |
| 14 | Freq Announce Delay | 1.2–5.0s | — | ❌ Not implemented (hardcoded debounce) |
| 15 | AF/RF Announcement | on/off | — | ❌ Not applicable (no AF/RF encoders) |
| 16 | Frequency Plus Mode | on/off | off | ❌ Not implemented |
| 17 | Comms Timeout | 5–20 | — | ❌ Hardcoded in comm layer |
| 18 | ICOM ID Port 1 | 00–A0 hex | A0 | ❌ Handled by Hamlib auto-detect |
| 19 | ICOM ID Port 2 | 00–A0 hex | A0 | ❌ Single-radio design |
| 20 | DTMF Tone Duration | 100–500ms | 200 | ❌ DTMF mode not implemented |
| 21 | Power On To Port | 1–2 | 1 | ❌ Single-port design |
| 22 | Reset/Flash | special | — | ❌ Not applicable (SD-based updates) |

### 6.4 Config Mode → Piper TTS Mapping (for implementation)

The original DoubleTalk synth had 9 voice parameters. Piper TTS exposes fewer controls, but the most important ones already have backend support:

*   [Software2/include/config.h](file:///Users/amberpadgett/Developer/HAMPOD2026/Software2/include/config.h)
    *   **Speech Speed**: `config_get_speech_speed()` / `config_set_speech_speed(float)` — range 0.5–2.0, maps to Piper `--length_scale`. Currently set once at startup in `main.c` via `comm_set_speech_speed()`.
    *   **Volume**: `config_get_volume()` / `config_set_volume(int)` — range 0–100, applied via `amixer -c N sset PCM X%`. Currently set once at startup in `main.c`.
    *   **Key Beep**: `config_get_key_beep_enabled()` / `config_set_key_beep_enabled(bool)` — already queried in real-time by `keypad.c` and `set_mode.c`.

*   **What's needed for runtime adjustment:**
    - Speed: Must call `comm_set_speech_speed()` after `config_set_speech_speed()` to take effect immediately (currently only done at startup)
    - Volume: Must re-run `amixer` command after `config_set_volume()` (currently only done at startup)
    - Key Beep: Already works at runtime ✅

### 6.5 Current Code State

*   [Software2/src/normal_mode.c](file:///Users/amberpadgett/Developer/HAMPOD2026/Software2/src/normal_mode.c)
    *   **Line 263:**
        ```c
        // [C] - Toggle verbosity (press) / Config mode entry (hold, not implemented)
        ```
    *   The hold-C path is not handled — it falls through to "Key not handled by normal mode".
    *   Implementation would require a new module (e.g., `config_mode.c`) integrated into the key routing in `main.c`, similar to how `set_mode.c` is integrated.

## 7. Gap Analysis: Original Spec vs. Software2

### Normal Mode Keys

| Key | Original Function | Software2 Status |
|-----|------------------|-----------------|
| `[0]` Press | Mode query | ✅ Implemented |
| `[0]` Hold | Data Mode toggle | ❌ |
| `[1]` Press | VFO A select | ✅ Implemented |
| `[1]` Hold | VFO B select | ✅ Implemented |
| `[Shift]+[1]` | VOX query | ✅ Implemented |
| `[2]` Press | Frequency query | ✅ Implemented |
| `[2]` Hold | Memory Scan toggle | ❌ |
| `[3]` Press/Hold | Split/VFO exchange | ❌ |
| `[A]` Press | Shift toggle | ✅ Implemented |
| `[A]` Hold | Volume Up | ❌ |
| `[4]` Press | PreAmp query | ✅ Implemented |
| `[4]` Hold | AGC query | ✅ Implemented |
| `[Shift]+[4]` | Attenuation query | ✅ Implemented |
| `[5]` Press | Tones query | ❌ |
| `[6]` Press/Hold | Filter queries | ❌ |
| `[B]` Press | Set Mode entry | ✅ Implemented |
| `[B]` Hold | Volume Down | ❌ |
| `[7]` Press | NB query | ✅ Implemented |
| `[7]` Hold | Tuner query | ❌ |
| `[8]` Press | NR query | ✅ Implemented |
| `[8]` Hold | Mic Gain query | ✅ Implemented |
| `[Shift]+[9]` | Compression query | ✅ Implemented |
| `[9]` Hold | Power query | ✅ Implemented |
| `[C]` Press | Freq announce toggle | ✅ Implemented (as "Announcements") |
| `[C]` Hold | Config Mode entry | ❌ **Not implemented** |
| `[Shift]+[C]` | Memory Mode | ❌ |
| `[*]` Press | S-Meter | ✅ Implemented |
| `[*]` Hold | Power Meter | ✅ Implemented |
| `[#]` Press | Freq Mode entry | ✅ Implemented |
| `[D]` Press | Verbosity toggle | ❌ (separate from [C] toggle) |
| `[D]` Hold | Port switch | ❌ N/A |
| `[Shift]+[D]` | DStar Mode | ❌ |
| `[Shift]+[D]` Hold | DTMF Mode | ❌ |

### Modes

| Mode | Original | Software2 Status |
|------|----------|-----------------|
| Normal Mode | Full key set | 🟡 ~16 of ~40+ functions |
| Frequency Entry Mode | Complete | ✅ Core complete |
| Set Mode | All radio params | ✅ Core params (9 of ~15) |
| Memory Mode | Full | ❌ Not implemented |
| DStar Mode | Full | ❌ Not implemented |
| DTMF Mode | 16-key tones | ❌ Not implemented |
| Configuration Mode | 22 parameters | ❌ **Not implemented** |
