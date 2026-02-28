# Config Mode Implementation Plan

This implementation plan describes the step-by-step approach to introducing Configuration Mode into the HAMPOD2026 software. The objective is to reproduce the user interface experience of the original HAMPOD (ICOM Reader) while supporting the new capabilities of the existing Linux/Piper-based system.

## User Review Required
> [!IMPORTANT]  
> Are these the correct initial four settings to include in the Config Mode?
> 1. Volume (0-100%)
> 2. Speech Speed (0.5 to 2.0x, mapped to Piper length scale)
> 3. Keypad Layout (Phone vs. Calculator)
> 4. System Shutdown (Initiate safe shutdown of Raspberry Pi)

## Proposed Changes

### Phase 1: Core Configuration Mode Logic & UI Navigation
This phase creates the state machine for Configuration Mode without actually applying the audio or system changes in real-time. It just handles the UI logic, navigating the menu, and making use of the undo history that already exists in `config.c`.

#### [NEW] `Software2/include/config_mode.h`
Create the public API for the Config Mode module:
- `void config_mode_init(void);`
- `void config_mode_enter(void);`
- `bool config_mode_is_active(void);`
- `bool config_mode_handle_key(char key, bool is_hold);`

#### [NEW] `Software2/src/config_mode.c`
Implement the state machine matching the behavior described in the ICOM Reader manual:
- **`[C]` Hold** to enter Config Mode and to save/exit.
- **`[D]` Hold** to exit without saving (discard).
- **`[A]`** to step forward through parameters: Volume -> Speech Speed -> Keypad Layout -> System Shutdown.
- **`[B]`** to step backward through parameters.
- **`[C]`** / **`[D]`** to increment / decrement parameter values.
- On entry, the module will note `config_get_undo_count()`. If the user discards changes using `[D]` hold, it will call `config_undo()` multiple times to return the configuration to its original state.

#### [MODIFY] `Software2/src/normal_mode.c`
- Update the `normal_mode_handle_key` function to detect the `[C]` Hold.
- If `key == 'C' && is_hold`, call `config_mode_enter()` and return true (so it doesn't fall through to error handling).

#### [MODIFY] `Software2/src/main.c`
- In `on_keypress`, route key events to `config_mode_handle_key` if `config_mode_is_active()` is true. Ensure this runs with higher priority than other modes so it intercepts all keys while configuring.
- Add `config_mode_init()` during startup.

#### [MODIFY] `Software2/Makefile`
- Add `src/config_mode.c` to `SW_SRCS`.

---

### Phase 2: Live Application of Settings
Make changes take effect immediately while remaining in Config Mode.

#### [MODIFY] `Software2/src/main.c`
- Extract the volume application command into a new globally accessible function `void audio_apply_volume(int volume_percent);`. 

#### [MODIFY] `Software2/include/audio_helpers.h` (or put in `comm.h`)
- Expose `audio_apply_volume` so it can be called from `config_mode.c`.

#### [MODIFY] `Software2/src/config_mode.c`
- **When Volume changes**: Call `audio_apply_volume()` immediately.
- **When Speech Speed changes**: Call `comm_set_speech_speed()` immediately.
- **When Keypad Layout changes**: Send a `CONFIG` packet down the pipe.
- **When System Shutdown is selected**: Send a command like `sudo shutdown -h now`.

---

### Phase 3: Hardware Firmware Packet Passthrough for Layout
The keypad layout switching logic requires real-time changes inside the hardware microcontroller `Firmware/`.

#### [MODIFY] `Software2/src/comm.c` and `Software2/include/comm.h`
- Define `int comm_send_config_packet(uint8_t sub_cmd, uint8_t value);`
- Use `PACKET_CONFIG`.
- Payload format: byte 0 = sub_command, byte 1 = value.
- Sub-commands: `0x01` = Layout.

#### [MODIFY] `Firmware/firmware.c`
- Add a block in the packet processing loop handling `PACKET_CONFIG`.
- Extract `sub_cmd` and `value`.
- If `sub_cmd == 0x01` (Layout): Call `hal_keypad_set_phone_layout(value == 1);` to immediately switch the polling layer's keymap. 

#### [MODIFY] `Software2/include/config.h` & `Software2/src/config.c`
- Add `void config_set_keypad_layout(const char *layout);` (already partially there, but ensure it works with auto-saving). 
- Ensure "state" variable (last viewed menu item) is added to `HampodConfig` struct so it resumes where the user left off.

## Verification Plan

### Automated Tests
1. **Unit tests**: Once Phase 1 is complete, run the existing C unit tests using `cd ~/HAMPOD2026/Software2 && make tests && ./run_all_unit_tests.sh --all` to make sure we haven't broken the existing configuration saving or history push logic.
2. **Build Test**: Run `make clean && make` on `Software2/` and `Firmware/` at the end of each phase.

### Manual Verification
1. **Navigation Test**: 
   - Start the software without radio: `./hampod --no-radio`
   - Hold `[C]` inside Normal Mode. Verify the TTS announces "Configuration Mode".
   - Press `[A]` and `[B]` to verify it cycles through the 4 options. 
   - Verify speech announces the parameters correctly.
2. **Persistence Test**: 
   - Change volume using `[C]` to increment.
   - Hold `[C]` to save and exit.
   - Look at `Software2/config/hampod.conf` to verify the new volume was saved.
3. **Discard Test**: 
   - Enter Config Mode, change volume, hold `[D]` to exit without saving.
   - Verify previous value is restored and `hampod.conf` reflects the original state.
4. **Live Settings Test**: 
   - While changing volume, verify it immediately affects system audio out.
   - While changing speech speed, verify Piper TTS immediately speaks faster/slower.
   - While changing layout, verify `7` changes to `1` instantly.
