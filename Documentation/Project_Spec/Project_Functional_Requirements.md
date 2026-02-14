# Functional Requirements

## 1. Radio Management
- **Detection & Listing**: The system shall scan for connected radios via Hamlib and list all detected devices.
- **Configuration Capacity**: Support configuration for up to ten (10) radio entries in the central configuration file.
- **Active Radio Selection**: Only one radio may be active at any time. Activation is performed by setting `enabled = true` for the desired radio and `enabled = false` for all others in the configuration file, followed by a system restart.
- **Runtime Switching**: Provide a mechanism to switch the active radio at runtime without requiring a full system restart, using a configuration update and pipe signal.

## 2. Frequency & Band Control
- **Current Frequency Readout**: The system shall read the currently tuned frequency from the active radio and announce it via speech synthesis.
- **Manual Frequency Entry**: Users may enter a target frequency via keypad navigation; the system shall validate and apply the entry.
- **Band/Preset Switching**: Support switching between predefined bands or presets associated with each radio configuration.

## 3. Audio Feedback
- **Speech Synthesis**: All status messages, frequency readouts, and error conditions shall be conveyed through Piper text‑to‑speech with configurable speed and volume.
- **Key Beep**: Optional audible keypad feedback shall be enabled via a configuration flag.

## 4. Hardware Interaction
- **Keypad Input**: The firmware shall scan the USB numeric keypad, debounce inputs, and transmit key events to the software layer via named pipes.
- **Audio Device Management**: The system shall prioritize a user‑specified USB audio device, falling back to the default ALSA device if the preferred device is unavailable.
- **ALSA Audio Output**: Audio output shall be handled through the Advanced Linux Sound Architecture (ALSA) with appropriate buffer settings for low latency.

## 5. System Operation
- **Process Architecture**: The application shall run as two cooperating processes:
  - *Firmware Process*: Manages hardware I/O (keypad, audio device, TTS engine).
  - *Software Process*: Implements application logic, Hamlib communication, and configuration parsing.
- **Inter‑Process Communication**: Named pipes shall be used for message exchange; messages shall follow a defined schema (e.g., `{type: "keypress", key: "C", shift: false, hold: false}`).
- **Startup & Shutdown**: The system shall initialize hardware, load configuration, and register all supported modes before entering the main event loop. Shutdown shall clean up resources (named pipes, audio handles) and exit gracefully.

## 6. Configuration Interface
- **Configuration File**: `Software2/config/hampod.conf` shall be parsed on startup; it shall contain sections for audio settings, radio definitions, and operational modes.
- **Runtime Reload**: Changes to the configuration file shall be detected and reloaded without requiring a full restart, where supported.

## 7. Error Handling & Recovery
- **Validation**: Invalid configuration entries shall trigger clear error messages via speech and log output.
- **Recovery**: The system shall attempt to recover from audio device hot‑plug events by re‑initializing the audio pipeline.

---  
*All requirements are expressed in terms of observable behavior and measurable performance criteria to facilitate verification and validation.*