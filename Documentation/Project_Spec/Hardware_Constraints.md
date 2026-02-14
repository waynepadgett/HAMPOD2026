# Hardware Constraints and Platform Specifics

## 1. Processing Resources
- **Target Platforms**: Raspberry Pi 3 (BCM2710), Raspberry Pi 4 (BCM2711), Raspberry Pi 5 (BCM2712).
- **CPU Architecture**: ARM Cortex‑A53 (Pi 3), ARM Cortex‑A72 (Pi 4), ARM Cortex‑A76 (Pi 5).
- **Clock Speed**: Up to 1.5 GHz (Pi 3), 1.8 GHz (Pi 4), 2.4 GHz (Pi 5).
- **Core Count**: Quad‑core on Pi 3/4/5 (all use multi‑core ARM CPUs).
- **Performance Target**: Keypad‑to‑speech latency ≤ 200 ms on Pi 5, ≤ 300 ms on Pi 3 under typical load.

## 2. Memory
- **RAM**: 1 GB (Pi 3), 1–8 GB (Pi 4), 4–8 GB (Pi 5). Documented targets assume at least 1 GB.
- **Allocation Strategy**: Static allocation for core data structures; dynamic allocation only for mode‑specific buffers; all allocations freed on process shutdown.
- **Memory Protection**: Enable ARM memory protection unit (MPU) features if available; use `mmap` with `PROT_READ/WRITE` as needed.

## 3. Audio Subsystem
- **Audio Backend**: ALSA (Advanced Linux Sound Architecture).
- **Preferred Audio Device**: User‑configurable USB audio device (e.g., USB audio dongle). Fallback to default ALSA device (`hw:0,0`).
- **Latency Target**: ≤ 100 ms from TTS command to audio output; achieve via:
  - Period size = 64 frames.
  - Buffer size = 4096 bytes.
  - Use of `aplay -r 16000 -c 1 -f S16_LE` for low‑latency playback.
- **Buffer Management**: Circular buffer with double‑buffering; monitor under‑run events and request refill.

## 4. USB Connectivity
- **USB Ports**: At least two USB‑A ports required (one for keypad, one for audio device).
- **USB Speed**: USB 2.0 high‑speed (480 Mbps) sufficient; USB 3.0 on Pi 4/5 provides additional headroom.
- **Power Budget**: Each USB device draws up to 500 mA; ensure adequate power supply (2.5 A recommended for Pi 5 with multiple peripherals).

## 5. Storage
- **Root Filesystem**: Read‑write partition on microSD; recommended size ≥ 16 GB.
- **Read‑Only Mode**: Optional read‑only rootfs for field deployments; see `Documentation/scripts/power_down_protection.sh` for implementation details.
- **Persistence**: Configuration changes must be persisted (e.g. `Software2/config/hampod.conf` for the main application, or `Configs.txt` in the legacy Software tree); use atomic rename where applicable to avoid corruption on power loss.

## 6. Thermal Management
- **Thermal Limits**: SoC temperature ≤ 85 °C; throttle clock if exceeded.
- **Cooling Recommendation**: Passive heatsink for continuous operation; active fan optional for Pi 5 under heavy TTS load.

## 7. Peripheral Compatibility
- **Keypad**: USB numeric keypad with at least 12 keys (0‑9, *, #, A, B, C, D). Must support NKRO (no ghosting) for simultaneous key detection.
- **Radio Interfaces**: Hamlib‑compatible radio control (serial, USB, or network). Must support standard commands for frequency, band, and mode selection.
- **Serial Port**: Typically `/dev/ttyUSB0`; configurable via `hampod.conf`.

## 8. Operating System
- **Supported OS**: Debian Trixie (64‑bit) on ARM architecture.
- **Package Manager**: `apt`; required packages listed in `Documentation/scripts/install_hampod.sh`.
- **Dependencies**: `git`, `gcc`, `make`, `libasound2-dev`, `libhamlib-dev`, `piper`, `ffmpeg` (for audio preprocessing).

## 9. Development Toolchain
- **Compiler**: `gcc` (ARM target) with `-O2` optimization.
- **Build System**: `make` with separate targets for firmware (`firmware.elf`) and software (`hampod` binary).
- **Debugging**: `gdb` and `valgrind` for memory error detection; `strace` for system call tracing.

## 10. Deployment Constraints
- **Auto‑Start**: Systemd service `hampod.service` enabled by default; can be disabled via `Documentation/scripts/hampod_on_powerup.sh`.
- **SD‑Card Protection**: Optional read‑only mode to protect against power loss; must be disabled during active development to allow config changes.
- **Remote Access**: SSH access recommended for remote debugging; use `Documentation/scripts/remote_install.sh` for code sync.

*All constraints must be validated on the target hardware before release. Performance benchmarks should be recorded on each supported Pi model to ensure compliance with latency and reliability targets.*