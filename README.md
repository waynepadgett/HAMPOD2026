# HAMPOD2026

**HAMPOD** is an accessibility interface designed to make amateur radios (like the Icom IC-7300) accessible to visually impaired operators. It uses a Raspberry Pi-based controller with a tactile keypad and text-to-speech (TTS) feedback to provide audio menus, frequency readouts, and full radio control via Hamlib.

## 🚧 Status: Active Reconstruction (Fresh Start)

This project is currently undergoing a major software rewrite ("Fresh Start"). The goal is to replace the original monolithic software layer with a modular, testable, and responsive architecture.

- **Firmware Layer:** Stable. Handles raw hardware I/O (Keypad, Audio extraction) and communicates via Named Pipes.
- **Software Layer:** Being rewritten from scratch to support robust polling, zero-latency feedback, and reliable Hamlib control.

## 📂 Repository Structure

*   **`Documentation/`** - **Start Here.** Contains build guides, architectural plans, and developer onboarding.
    *   `Project Overview and Onboarding/` - Critical reading for developers.
        *   [`fresh-start-big-plan.md`](Documentation/Project%20Overview%20and%20Onboarding/fresh-start-big-plan.md) - The master plan for the rewrite.
        *   [`fresh-start-phase-zero-plan.md`](Documentation/Project%20Overview%20and%20Onboarding/fresh-start-phase-zero-plan.md) - Immediate steps for core infrastructure.
*   **`Firmware/`** - The low-level C code that runs on the Pi. It manages the keypad hardware and separates audio output.
*   **`Software/`** - The legacy (2025 team) software implementation. It is currently serving as a reference but is slated for replacement.
*   **`Hardware Files/`** - Schematics and PCB design files for the custom keypad/hat.

## 🚀 Getting Started (Developers)

1.  **Review the Plans:** Read `Documentation/Project Overview and Onboarding/fresh-start-big-plan.md` to understand the architecture.
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
