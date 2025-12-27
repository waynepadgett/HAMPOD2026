# Raspberry Pi Development Setup Guide

This guide provides detailed, step-by-step instructions for setting up a Raspberry Pi (RPi) from scratch to run the HAMPOD project.

> **Note**: This guide focuses on installing the code and dependencies **directly on the Raspberry Pi**. For a standard development workflow, you will likely also want to clone the repository on your **Development Machine** (PC/Mac) to edit files locally and then sync them to the Pi for testing (e.g., using the `remote_install.sh` script mentioned in the Tips section). Steps for setting up your local Development Machine environment are not covered in detail here.

## 📋 Prerequisites

Before you begin, ensure you have the following:

- **Raspberry Pi**: Recommended RPi 5 or RPi 4 (4GB+ RAM preferred, though 2GB works).
- **microSD Card**: 16GB or larger (Class 10 / UHS-1 recommended).
- **Power Supply**: Official USB-C power supply for your specific RPi model.
- **Computer**: A Windows, Mac, or Linux computer to flash the SD card.
- **Internet Connection**: Wi-Fi credentials or an Ethernet cable.

---

## 🚀 Step 1: Install Debian Trixie

The project is currently being developed on **Debian Trixie**. Please install this version using the Raspberry Pi Imager.

1.  **Download & Install Raspberry Pi Imager**:
    - Go to [raspberrypi.com/software](https://www.raspberrypi.com/software/) and download the imager for your computer.

2.  **Configure the Image**:
    - Open Raspberry Pi Imager.
    - **Choose Device**: Select your RPi model (e.g., Raspberry Pi 5).
    - **Choose OS**:
        - Click "Choose OS".
        - Select  **Debian** -> **Raspberry Pi OS (64-bit) - (debian trixie)** (or the specific Trixie image if listed differently).
    - **Choose Storage**: Select your microSD card. (it should be plugged in and visible in the list), if it is not visible, make sure it is plugged in and try again.
    - **Choose hostname: `hampod` (or whatever you prefer).
    - enter your localization settings. (capital city washington for USA based, new york for timezone )
    - click "Next".
    - set your username and password. (you can also use hampod for this as well)
    - set your password to something strong (diceware is a good option)
    - click "Next".
    - enter your Wi-Fi SSID and password (if not using Ethernet)
    - click "Next".
    - make sure to enable SSH
    - if you can, use public key authentication, it makes everything easier for passwordless login
    - if you can't use public key authentication, use password authentication
    - click "Next".
    - leave raspberry pi connect disabled
    - click "Next".
    - click "Write".
    - confirm the write and make absolutely sure its the correct drive - you will destroy all data on the drive when it writes
    - wait for it to complete, it will take a few minutes
    - once it's done, remove the microSD card and insert it into your RPi
    - power on the RPi
    - wait for it to boot up
    -  **(DEV:)** Connect using the hostname and username you set:
    ```bash
    ssh hampod@hampod.local
    ```
    *(If prompted about authenticity, type `yes`)*
    - if you used public key authentication, you should be able to connect without a password
    - if you used password authentication, you will be prompted for a password
    - once connected, you should see the terminal prompt for the user you created


> **Troubleshooting**: If `hampod.local` doesn't work, check your router's device list to find the IP address of the Pi and use `ssh hampod@192.168.x.x` instead.

---

TODO: fix this section 2025-12-27--12-31-PM 

### Optional: Set up an SSH Key
To avoid typing your password every time, you can generate an SSH key on your computer and copy it to the Pi.

1.  **(DEV:)** **Generate a key** (if you don't have one):
    *On Windows (PowerShell) or Mac/Linux (Terminal):*
    ```bash
    ssh-keygen -t ed25519
    ```
    *(Press Enter to accept defaults. If using Windows Command Prompt, ensure OpenSSH is installed/enabled)*

2.  **(DEV:)** **Copy the key to the Pi**:
    *On Windows (PowerShell):*
    ```powershell
    type $env:USERPROFILE\.ssh\id_ed25519.pub | ssh hampod@hampod.local "mkdir -p ~/.ssh && cat >> ~/.ssh/authorized_keys"
    ```
    *On Mac/Linux:*
    ```bash
    ssh-copy-id hampod@hampod.local
    ```

3.  **Verify**: Log out and log back in. You should not be asked for a password.

---

## 🛠️ Step 3: Install System Dependencies

Once logged in, update the system and install the required build tools and libraries.

1.  **(RPI:)** **Update Package Lists**:
    ```bash
    sudo apt update && sudo apt upgrade -y
    ```
2.  **(RPI:)** **Install Development Tools & Audio Libraries**:
    HAMPOD requires `git`, `make`, `gcc`, ALSA (audio), and Hamlib (rig control).
    ```bash
    sudo apt install -y git make gcc libasound2-dev libhamlib-dev
    ```

---

## 📂 Step 4: Clone the HAMPOD Repository

Now, download the project code to your home directory.

1.  **(RPI:)** **Clone the Repository**:
    > **Note**: The HAMPOD2026 repository is **public**, so you do **not** need a GitHub account simply to clone and run it. You only need a GitHub account if you plan to **contribute** changes back to the project. If you plan to contribute and don't have an account, you can create one for free at [github.com/join](https://github.com/join).
    ```bash
    cd ~
    git clone https://github.com/waynepadgett/HAMPOD2026.git
    ```

---

## 🗣️ Step 5: Install Piper TTS

HAMPOD uses **Piper** for high-quality, low-latency text-to-speech. We have a script to automate this installation.


1.  **(RPI:)** **Make Scripts Executable**:
    ```bash
    cd HAMPOD2026
    chmod +x Documentation/scripts/*.sh
    ```
2.  **(RPI:)** **Run the Installer**:
    ```bash
    cd ~/HAMPOD2026/Documentation/scripts
    ./install_piper.sh
    ```
    *This script downloads the Piper binary, installs it to `/usr/local/bin/piper`, and downloads the required voice model (`en_US-lessac-low.onnx`) to `Firmware/models/`.*

---

## 🏗️ Step 6: Build the Project

Currently, the primary component to build is the **Firmware** (which handles hardware, audio, and keypad input).

> **Note on Software Layer**: The high-level Software layer is currently under active development ("Fresh Start" refactoring in `Software2/`). The legacy `Software/` directory may not build cleanly on a fresh system. For now, focus on building the Firmware and running the **HAL Integration Test** (see Step 7) to verify your setup.

### Build Firmware (RPI:)
```bash
cd ~/HAMPOD2026/Firmware
make clean   # Remove any stale object files (prevents architecture mismatch errors)
make
```
*If successful, you will see a `firmware.elf` file created.*

---

## ▶️ Step 7: Verify Your Setup (HAL Integration Test)

The recommended way to verify your Raspberry Pi setup is to run the **HAL Integration Test**. This standalone test verifies that the keypad and audio/TTS subsystems are working correctly.

1.  **(RPI:)** **Build the Test**:
    ```bash
    cd ~/HAMPOD2026/Firmware/hal/tests
    make
    ```

2.  **(RPI:)** **Run the Test** (from the Firmware directory, required for model paths):
    ```bash
    cd ~/HAMPOD2026/Documentation/scripts
    ./Regression_HAL_Integration.sh
    ```

    List of tests that generate audio but do not need the radio plugged in:

    - **Regression_Phase0_Integration.sh**: Tests the router thread and basic keypad/audio interaction.
        - **What to Expect**:
            - The system should announce **"System Ready"** (or similar).
            - Pressing keys should announce **"You pressed [Key Name]"** (e.g., "You pressed One").
            - Holding keys should announce **"You held [Key Name]"**.
            - Use `Ctrl+C` to exit (or wait for timeout).

    - **Regression_Imitation_Software.sh**: Tests the low-level pipe communication between Firmware and Software.
        - **What to Expect**:
            - You should hear an initial TTS announcement.
            - Pressing keys will trigger audio feedback.
            - Use `Ctrl+C` to exit (or wait for timeout).

    - **Regression_HAL_Integration.sh**: Verifies the hardware abstraction layer for keypad and audio/TTS.
        - **What to Expect**:
            - The system should announce **"System Ready"** (or similar).
            - Pressing keys on the keypad should announce the key name (e.g., "One", "Enter").
            - Use `Ctrl+C` to exit (or wait for timeout).


> **If you only hear a click sound (no speech)**, see the **Troubleshooting Audio/TTS** section below.

### Running the Full System (Advanced)

Once the Software layer development is complete, you will run Firmware and Software in separate terminals:


## 🔧 Troubleshooting Audio/TTS

If the integration test runs but you **only hear a click sound** (no spoken words), the issue is likely with Piper TTS or the audio configuration.

### 1. Verify Piper is Installed (RPI:)
```bash
which piper
piper --version
```
If `piper` is not found, re-run the installer:
```bash
./Documentation/scripts/install_piper.sh
```

### 2. Verify Voice Model Exists (RPI:)
The voice model files must be in `~/HAMPOD2026/Firmware/models/`:
```bash
ls -la ~/HAMPOD2026/Firmware/models/
```
You should see `en_US-lessac-low.onnx` and `en_US-lessac-low.onnx.json`. If missing, re-run the installer script.

### 3. Test Piper Manually (RPI:)
Test that Piper can generate and play speech:
```bash
echo "Hello World" | piper --model ~/HAMPOD2026/Firmware/models/en_US-lessac-low.onnx --output_raw | aplay -r 16000 -c 1 -f S16_LE
```
- **If you hear "Hello World"**: Piper is working. The issue may be in how the integration test invokes it.
- **If you hear nothing**: Continue to step 4.

### 4. Test Basic Audio Output (RPI:)
Verify that the audio hardware itself is working:
```bash
# List available audio devices
aplay -l

# Play a test tone (you should hear a voice saying "Front Left", "Front Right")
speaker-test -c 2 -t wav
```
If no audio is heard, check:
- Is the speaker/headphones connected and powered on?
- Is the correct audio output selected? (HDMI vs. headphone jack vs. USB audio)
- Try forcing audio output: `sudo raspi-config` → System Options → Audio.

### 5. Check User Permissions (RPI:)
Ensure your user is in the `audio` group:
```bash
groups
```
If `audio` is not listed, add yourself:
```bash
sudo usermod -aG audio $USER
```
Then **log out and log back in** for the change to take effect.

---

## 📝 Tips for Developers

- **Remote Install Script**: **(DEV:)** For rapid development, use the `remote_install.sh` script from your local machine to push code and rebuild on the Pi automatically.
  ```bash
  # From your local machine
  ./Documentation/scripts/remote_install.sh "Update description"
  ```

- **Cleaning Builds**: **(RPI:)** If you run into weird issues, try cleaning:
  ```bash
  make clean
  make
  ```

- **Process Cleanup (After a crash)**:
  **(RPI:)** If the firmware or software crashes, it might leave "ghost" processes or named pipes that prevent restarting. Run this to clean up:
  ```bash
  # Kill lingering processes
  sudo pkill -9 firmware
  sudo pkill -9 imitation_software
  pkill -9 Software.elf

  # Remove stale pipes
  rm -f ~/HAMPOD2026/Firmware/Firmware_i
  rm -f ~/HAMPOD2026/Firmware/Firmware_o
  ```

- **Remote Reboot**:
  **(RPI:)** If the system becomes unresponsive or audio drivers lock up:
  ```bash
  sudo reboot
  ```
