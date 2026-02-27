# HAMPOD2026

HAMPOD is an accessibility interface designed to make amateur radios accessible to visually impaired operators. It uses a Raspberry Pi-based controller with a tactile USB keypad and text-to-speech feedback to provide audio menus, frequency readouts, and radio control via Hamlib.

## Status

Most basic functions are implemented. The system can read frequency, change bands, and control the radio. Some advanced features from the original ICOMReader are still in development.

Current capabilities:
- Multi-radio configuration: Supports up to 10 radios including IC-7300, TS-570, and TS-2000
- Deterministic audio device selection: Prioritizes external USB audio devices
- Piper text-to-speech: Fast, natural-sounding speech synthesis
- USB keypad input: Standard numeric keypad for tactile operation
- Hamlib radio control: Frequency, mode, and band control

## Supported Radios

Tested radios:
- ICOM IC-7300: Hamlib model 3073, primary development radio
- Kenwood TS-570: Hamlib model 2004, tested working
- Kenwood TS-2000: Hamlib model 2014, configured but not yet tested

## Supported Platforms

- Raspberry Pi 5 with Debian Trixie: Primary development platform
- Raspberry Pi 4 with Debian Trixie: Supported
- Raspberry Pi 3 with Debian Trixie: Supported

## Repository Structure

- Documentation folder: Build guides, plans, and install scripts
  - Project_Overview_and_Onboarding subfolder: Developer onboarding documents
  - scripts subfolder: Install and run scripts
- Firmware folder: Low-level hardware code for keypad, audio, and text-to-speech
- Software2 folder: Application layer with radio control and user modes
  - config subfolder: Contains hampod.conf configuration file
  - src subfolder: Source code files
- Hardware_Files folder: Schematics and PCB designs

## Quick Start

### Prerequisites

- Raspberry Pi 3, 4, or 5 with Debian Trixie installed

If you need to set up the operating system on your Raspberry Pi and flash an SD card, see RPi_Setup_Guide.md in the Documentation/Project_Overview_and_Onboarding folder.
- USB numeric keypad
- USB audio device (such as USB2.0 Device)
- Amateur radio with USB or serial interface


### RPI zero 2 w setup

```bash
sudo fallocate -l 2G /var/temp_swap && sudo chmod 600 /var/temp_swap && sudo mkswap /var/temp_swap && sudo swapon /var/temp_swap
```

run this to add swap space for the RPI zero 2 w - prevents an OOM error during installation of the apt upgrade

```bash
free -h 
```

to confirm swap space was added

```bash
sudo apt update && sudo apt install -y git
```

(this is needed if you are setting up a new RPI zero 2 w with the lite version of the OS as it doesn't come with git installed)

proceed as normal for the rest of the install script. 

### Installation

SSH into your Raspberry Pi and run these commands:

```
git clone https://github.com/waynepadgett/HAMPOD2026.git && cd HAMPOD2026/Documentation/scripts && ./install_hampod.sh
```


The install script will update system packages, install dependencies including Hamlib and Piper text-to-speech, build the firmware and software, and configure audio permissions.


### Running HAMPOD
`
To start the system:

```
cd ~/HAMPOD2026 && ./Documentation/scripts/run_hampod.sh
```

To stop, press Control-C.

## Configuration

The configuration file is located at Software2/config/hampod.conf. Edit this file to configure your radios.

### Audio Settings

- volume: Speaker volume level, default is 25
- speech_speed: Text-to-speech speed, default is 1.0
- key_beep: Enable key beep sounds, 1 for on, 0 for off
- preferred_device: Name of preferred USB audio device

### Radio Settings

You can configure multiple radios. Only one radio can be enabled at a time. Each radio section includes:

- name: Descriptive name for the radio
- enabled: Set to true for the active radio, false for others
- model: Hamlib model number
- device: Serial port path, usually /dev/ttyUSB0
- baud: Baud rate for serial communication

### Switching Radios

To switch to a different radio:
1. Open Software2/config/hampod.conf in a text editor
2. Find the currently enabled radio and change enabled = true to enabled = false
3. Find the radio you want to use and change enabled = false to enabled = true
4. Save the file and restart HAMPOD

## Architecture

The system runs as two processes that communicate through named pipes.

### Firmware Process

The firmware handles hardware input and output:
- Reads key presses from the USB keypad
- Manages audio output through ALSA
- Runs the Piper text-to-speech engine
- Detects and selects audio devices

### Software2 Process

The software handles application logic:
- Receives key events from firmware
- Manages operating modes including Normal, Frequency, and Set modes
- Controls the physical radio through Hamlib
- Sends speech text to firmware for audio output

## Development

### Building Manually

To build the firmware:

```
cd Firmware
make clean all
```

To build the software:

```
cd Software2
make clean all
```

### Running Tests

HAL Integration Test:

```
cd Documentation/scripts
./Regression_HAL_Integration.sh
```

Phase 0 Integration Test:

```
cd Documentation/scripts
./Regression_Phase0_Integration.sh
```

## Documentation

Key documents for developers:
- [RPi Setup Guide](Documentation/Project_Overview_and_Onboarding/RPi_Setup_Guide.md) - Manual setup instructions
- [Startup Device Handling Plan](Documentation/Project_Overview_and_Onboarding/startup_device_handling_plan.md) - Audio and radio device handling
- [Currently Implemented Keys](Documentation/Project_Overview_and_Onboarding/Currently_Implemented_Keys.md) - Keypad mapping reference
- [ICOMReaderManual2](Documentation/Original_Hampod_Docs/ICOMReaderManual2.md) - Original feature specification

### Project Specification and Plan
- [Project Plan](Documentation/Project_Spec/Project_Plan.md) – Integration/testing strategy, risks & mitigations, milestones, code readability
- [Refactor Todos](Documentation/Project_Spec/Refactor_Todos.md) – Checklist to reach maximum clarity (docs, code, structure, tests)
- [Hardware Constraints](Documentation/Project_Spec/Hardware_Constraints.md)
- [Project Objectives and Target Users](Documentation/Project_Spec/Project_Objectives_and_Target_Users.md)
- [Project Functional Requirements](Documentation/Project_Spec/Project_Functional_Requirements.md)
- [System Architecture](Documentation/Project_Spec/System_Architecture.md)
- [System Architecture (detailed)](Documentation/Project_Spec/System_Architecture_Detail.md)
- [Raspberry Pi Migration Plan](Documentation/Hampod%20RPi%20change%20plan.md)

## License

This project is maintained for accessibility purposes. See repository for license details.

Last updated: February 2026
