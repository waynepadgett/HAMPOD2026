Future Work ideas

Simulated Radio for hampod testing (Use an RPi maybe with a small display) to respond appropriatly to hamlib commands to allow testing without a real radio

Speech input system (microphone instead of keypad) (whisper)

Testing with lower performance, lower cost RPi boards (RPi 3B?), reduced RAM

Investigate RPi speaker hat, microphone

A key for switching between devices (radio, antenna rotor, etc.) (maybe the backspace key?)

Add support for many other radios (Icom, Kenwood, Yaesu, Elecraft, etc.)

Add support for connecting to many devices at the same time

Add a system setup mode for adding devices and adding them to the config file using the keypad interface

Add a systematic way to handle key mappings for different devices so that it is easy to add new devices

Add a system for mapping the keypad keys to different layouts by editing a simple file format (so you can pick where 1,2,3 are however you like)

Add support for many other devices (antenna rotors, etc.)

Convert the OS to use a ramdisk for all operations other than configuration file so that power downs can't corrupt the OS

### Live Configuration Changes
Currently, configuration changes in `hampod.conf` require a restart to take effect. The long-term goal is to let users change settings (volume, speech speed, keypad layout, etc.) during operation via a Config Mode and have changes apply instantly.

**Recommended approach: CONFIG packets through existing pipes.**

The Software↔Firmware pipe protocol already has a `CONFIG` packet type. The flow would be:

1. User enters Config Mode and changes a setting (e.g., keypad layout)
2. Software layer updates `hampod.conf` and sends a CONFIG packet to Firmware
3. Firmware receives the packet and applies the change (e.g., calls `hal_keypad_set_phone_layout()`)
4. Change takes effect immediately on the next keypress

This approach leverages the existing pipe infrastructure and is extensible — any new setting that needs live propagation just defines a CONFIG sub-command. The runtime variables for keypad layout switching are already in place (`g_phone_layout` in `hal_keypad_usb.c`); the only remaining work is the CONFIG packet plumbing.

Settings that would benefit from live propagation:
- Keypad layout (calculator ↔ phone)
- Audio volume
- Speech speed
- Key beep on/off
- Terse/Verbose announcement mode

### User Interface Refinements
- **Conditional Status Announcements**: For queries like NB (Noise Blanker) and NR (Noise Reduction), only announce the level if the feature is enabled. This reduces verbosity.
- **VFO Mode Announcements**:
    - "Current VFO" logic is specific to Icom.
    - Other manufacturers typically use VFO A/B (or C).
    - Suggested focused announcements to minimize chatter:
        - **Icom**: "Current" / "A" / "B" (model dependent)
        - **TS-2000**: "A" / "B" / "C"
        - **Most others**: "A" / "B"
    - Consider a "Terse/Verbose" setting to optionally toggle "VFO" prefix (e.g., "VFO A" vs "A").

### Data-Driven Radio Architecture
- **Problem**: Maintenance is difficult due to feature overlap but specific differences between radios.
- **Proposal**: Express radio behaviors, announcements, Hamlib calls, and key mappings in a standardized, human-readable format (e.g., JSON, YAML, or TOML).
- **Benefits**:
    - Build code or determine runtime behavior from these files.
    - Adding a new radio becomes a configuration task (copy similar radio config, tweak) rather than a coding task.
    - Interface tweaks (like the VFO announcements above) can be made in data without rewriting logic.
