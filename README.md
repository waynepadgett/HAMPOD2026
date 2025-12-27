# HAMPOD2026

**HAMPOD** is an accessibility interface designed to make amateur radios (like the Icom IC-7300) accessible to visually impaired operators. It uses a Raspberry Pi-based controller with a tactile keypad and text-to-speech (TTS) feedback to provide audio menus, frequency readouts, and full radio control via Hamlib.

## 🚧 Status: Active Reconstruction (Fresh Start)

This project is currently undergoing a major software rewrite ("Fresh Start"). The goal is to replace the original monolithic software layer with a modular, testable, and responsive architecture.

- **Firmware Layer:** Stable. Handles raw hardware I/O (Keypad, Audio extraction) and communicates via Named Pipes.
- **Software Layer:** Being rewritten from scratch to support robust polling, zero-latency feedback, and reliable Hamlib control.

## 📂 Repository Structure

*   **`Documentation/`** - **Start Here.** Contains build guides, architectural plans, and developer onboarding.
    *   `Project_Overview_and_Onboarding/` - Critical reading for developers.
        *   [`fresh-start-big-plan.md`](Documentation/Project_Overview_and_Onboarding/fresh-start-big-plan.md) - The master plan for the rewrite.
        *   [`fresh-start-phase-zero-plan.md`](Documentation/Project_Overview_and_Onboarding/fresh-start-phase-zero-plan.md) - Immediate steps for core infrastructure.
*   **`Firmware/`** - The low-level C code that runs on the Pi. It manages the keypad hardware and separates audio output.
*   **`Software/`** - The legacy (2025 team) software implementation. It is currently serving as a reference but is slated for replacement.
*   **`Hardware_Files/`** - Schematics and PCB design files for the custom keypad/hat.

## 🚀 Quick Install (Raspberry Pi)

We have an automated install script that sets up everything for you! Just flash your Pi with Debian Trixie, enable SSH, and run:

### Option 1: Feature Branch (Current - Working ✅)
```bash
# SSH into your Pi, then:
git clone -b feature/set-mode https://github.com/waynepadgett/HAMPOD2026.git
cd HAMPOD2026/Documentation/scripts
./install_hampod.sh
```

### Option 2: Main Branch One-Liner (Coming Soon ⏳)
> **Note:** This will work once the install script is merged to main.
```bash
# SSH into your Pi, then:
curl -sSL https://raw.githubusercontent.com/waynepadgett/HAMPOD2026/main/Documentation/scripts/install_hampod.sh | bash
```

The script will:
- Update system packages
- Install all dependencies (git, gcc, ALSA, Hamlib)
- Clone the repository
- Install Piper TTS for speech
- Build the Firmware
- Build integration tests
- Configure audio permissions

After completion, it will prompt you to run the first regression test to verify your setup.

📖 For detailed manual setup instructions, see [`Documentation/Project_Overview_and_Onboarding/RPi_Setup_Guide.md`](Documentation/Project_Overview_and_Onboarding/RPi_Setup_Guide.md).

---

## 🛠️ Getting Started (Developers)

1.  **Review the Plans:** Read `Documentation/Project_Overview_and_Onboarding/fresh-start-big-plan.md` to understand the architecture.
2.  **Setup Environment:**
    *   Target hardware: Raspberry Pi 5 (Debian Trixie).
    *   Dependencies: `libhamlib-dev`, `festival`, `alsa-utils`, `libasound2-dev`.
3.  **Deployment:** use the scripts in `Documentation/scripts/` to deploy code to the Pi.

## Architecture Overview

The system runs as two main processes communicating via **Named Pipes**:

1.  **Firmware Process:**
    *   Reads GPIO pins for the Keypad.
    *   Manages direct ALSA audio (or Festival TTS).
    *   Sends raw key events to the Software.
2.  **Software Process (New):**
    *   Receives key events.
    *   Manages application state (Menu, Frequency, Settings).
    *   Controls the physical radio via `hamlib`.
    *   Sends text strings back to Firmware for speech output.
