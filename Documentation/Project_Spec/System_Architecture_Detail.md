# System Architecture – Detailed Specification

## 1. Overview
The HAMPOD system is split into **two cooperative processes** that communicate via **named pipes**:

| Process | Primary Responsibilities |
|---------|---------------------------|
| **Firmware** | • Scan and debounce the USB keypad<br>• Detect and select the preferred ALSA audio device<br>• Invoke the Piper TTS engine<br>• Transmit key events to the Software process |
| **Software** | • Parse `hampod.conf` and manage runtime reloads<br>• Mode routing and dispatch (Normal, Frequency, Config, etc.)<br>• Interact with Hamlib to control the radio<br>• Generate status messages for speech synthesis<br>• Manage configuration persistence |

## 2. Process Diagram (Mermaid)

```mermaid
graph TD
    Firmware[Firmware Process] -->|Key Event| Software[Software Process]
    Software -->|Config Update| Config[Configuration Manager]
    Software -->|Radio Command| Hamlib[Hamlib Interface]
    Software -->|Audio Request| Audio[Audio Manager]
    Audio -->|TTS Command| TTS[Piper Engine]
    TTS -->|Audio Output| ALSA[ALSA Device]
    Software -->|Mode Request| ModeRouter[Mode Router (ModeRouting.c)]
    ModeRouter -->|Dynamic Load| libModes[libModes.so]
```

## 3. Component Breakdown

### 3.1 Firmware Process
- **Files**: `keypad_firmware.c`, `audio_firmware.c`, `pipe_writer.c`, `pipe_remover.sh` (pipes are created in `firmware.c`; `pipe_remover.sh` cleans up).
- **Responsibilities**:
  - Scan keypad matrix, debounce, generate `KeyPress` events.
  - Monitor ALSA device list; prioritize the user‑specified `preferred_device`.
  - Capture TTS prompts and pipe them to the Software process.
  - Cleanly handle signals for graceful shutdown.

### 3.2 Software Process
- **Core Files**:
  - `ModeRouting.c` – dynamic loading of mode implementations from `libModes.so`.
  - `StateMachine.c` – manages current operating mode (Normal, Frequency, Config, etc.).
  - `Radio.c` – abstracts Hamlib calls (frequency read/write, band switch).
  - `ConfigSettings/*` – loads/saves configuration values.
- **Responsibilities**:
  - Receive `KeyPress` events, translate to mode‑specific commands.
  - Load the appropriate mode module based on the key sequence.
  - Update radio state via Hamlib and request speech feedback.
  - Persist configuration changes to `Configs.txt`.

### 3.3 Mode Router
- **Functionality**:
  - Scans `Modes/` directory for `*_Load.c` functions.
  - Builds a hashmap mapping mode names to loaded `Mode` structs.
  - Provides `getModeByName()`, `getAllModes()`, and lookup utilities.
- **Dynamic Loading**:
  - Uses `dlopen()`/`dlsym()` to load modules at runtime.
  - Supports adding new modes without recompiling the core.

### 3.4 Configuration Manager
- **File**: `Software/ConfigSettings/ConfigLoad.c`
- **Key Functions**:
  - `enterConfigMode()` / `exitConfigMode()` – manage entry/exit of config mode.
  - `configNavigation()` – handle keypad navigation (4/6 for prev/next, 2/8 for increment/decrement).
  - `configCommandRelay()` – dispatch keypad actions to appropriate config handlers.
- **Supported Config Types**:
  - `ONOFF`, `NUMERIC`, `ONOFFNUMERIC`, `NUMPAD`, `OTHER`.

### 3.5 Audio Manager
- **Interface**: `sendSpeakerOutput()` in `KeyWatching.c`.
- **Implementation**:
  - Routes text to Piper with the configured voice model.
  - Adjusts volume and speed via command‑line options.
  - Handles fallback to a default ALSA device if the preferred one is unavailable.

### 3.6 Hamlib Interface
- **Abstraction Layer**:
  - Wraps Hamlib’s rig control functions (`rig_set_freq`, `rig_set_band`, etc.).
  - Provides a clean API for the Software process to issue commands.
  - Handles error checking and retries.

## 4. Inter‑Process Communication (IPC)
- **Named Pipes**: Created in `Firmware/pipe_writer.c` / `Firmware/pipe_remover.sh`.
- **Message Format**: JSON‑like strings (e.g., `{"type":"keypress","key":"C","shift":false,"hold":false}`).
- **Signal Flow**:
  1. Firmware writes to pipe → Software reads via `keypadInput()`.
  2. Software processes and sends status updates back on a reverse pipe for Firmware to speak.

## 5. Configuration Lifecycle
1. **Startup** – `ConfigLoad()` creates a `Mode` with `enterMode = enterConfigMode`.
2. **Configuration Reload** – `exitConfigMode()` restores previous values; changes are persisted via `writeConfigParamsToFile()`.
3. **Runtime Update** – `writeConfigParamsToFile()` persists changes; `enterConfigMode()` reloads the updated list.

## 6. Error Handling & Recovery
- **Validation** – All config entries are validated; invalid entries trigger spoken error messages.
- **Audio Hot‑Plug** – Detection of device appearance/disappearance triggers re‑initialization of the Audio Manager.
- **Watchdog** – A background watchdog process monitors the firmware and software pipes; on failure it triggers a system restart.

## 7. Extensibility
- **Adding New Radio Models** – Place a new `*_Load.c` implementation in `Modes/` and update the naming convention; the router will auto‑detect it.
- **New Config Types** – Extend `configType` enum and handle in `configNavigation()` and related switch cases.

## 8. Mapping to Hardware Constraints
| Constraint | Detail | Mitigation |
|------------|--------|------------|
| **CPU** | Raspberry Pi 3/4/5 ARM Cortex‑A53/A72; limited CPU cycles. | Keep mode implementations lightweight; avoid blocking calls; use non‑blocking I/O. |
| **Memory** | 1 GB (Pi 3), 1–8 GB (Pi 4), 4–8 GB (Pi 5). | Use static allocation where possible; free dynamically allocated structures on exit. |
| **Audio Latency** | Target ≤ 100 ms from keypad press to speech output. | Pre‑load voice model; use low‑latency ALSA parameters (`period_size=64`). |
| **USB Bandwidth** | Keypad and audio device share USB bus. | Prioritize audio device; use USB 2.0 high‑speed mode; avoid unnecessary polling. |
| **Power** | Field deployments may experience power loss. | Enable SD‑card protection mode; graceful shutdown via watchdog. |

## 9. Performance, Latency, and Reliability Targets
- **Keypad‑to‑Speech Latency**: ≤ 200 ms (95th percentile) on Raspberry Pi 5, ≤ 300 ms on Pi 3.
- **CPU Utilization**: < 30 % idle, < 70 % under full keypad activity.
- **Audio Buffer Under‑run**: < 1 % of samples lost; implement buffer monitoring.
- **Reliability**: 99.9 % uptime over 30‑day continuous run; automatic recovery from audio device hot‑plug events.

## 10. Integration, Testing, and Validation Strategy
| Test | Scope | Location | Success Criteria |
|------|-------|----------|------------------|
| **Phase 0 Integration** | Verify basic pipe communication and keypad echo | `Documentation/scripts/Regression_Phase0_Integration.sh` | All key events echoed correctly; “System Ready” announced. |
| **HAL Integration** | Test hardware abstraction layer (keypad + audio) | `Documentation/scripts/Regression_HAL_Integration.sh` | No pipe leaks; all keypad codes mapped correctly. |
| **Mode Stress Test** | Load all supported modes simultaneously | Custom script | No memory leaks; mode switching without crashes. |
| **Performance Benchmark** | Measure latency and CPU usage | Custom benchmark harness | Meets targets in Section 9. |
| **Regression Tests** | Full suite after any code change | `Regression_*.sh` scripts | All tests pass; no new failures. |

## 11. Risks & Mitigation
| Risk | Likelihood | Impact | Mitigation |
|------|------------|--------|------------|
| **Audio Device Not Found** | Medium | High (no speech) | Preference list, fallback to default ALSA, clear error message. |
| **Hamlib Version Mismatch** | Low | Medium | Pin Hamlib version in `install_hampod.sh`; provide script to install matching version. |
| **Keypad Debounce Errors** | Low | Medium | Debounce algorithm with configurable interval; extensive unit tests. |
| **Configuration Corruption** | Low | High | Write‑through to a temporary file then atomic rename; validate on load. |
| **Power Loss During Write** | Medium | High | Enable SD‑card protection; use atomic file writes. |

## 12. Future Extensibility
- **Additional Radio Models** – Add new mode files; no core changes required.
- **Advanced Speech Features** – Add SSML support, custom voice models.
- **Cloud Sync** – Optional module to sync configuration with a remote server.

---

*This detailed architecture provides a concrete blueprint for implementation, testing, and future expansion while preserving the project’s accessibility focus.*