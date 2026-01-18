# Raspberry Pi Development Setup Guide

This guide provides step-by-step instructions for setting up a Raspberry Pi to run the HAMPOD project.

Note: This guide focuses on installing the code and dependencies directly on the Raspberry Pi. For standard development, you will also clone the repository on your development machine (PC or Mac) to edit files locally and sync them to the Pi for testing using the remote_install.sh script.

## Prerequisites

Before you begin, ensure you have the following:

- Raspberry Pi: Recommended RPi 5 or RPi 4 (4GB+ RAM preferred, though 2GB works)
- microSD Card: 16GB or larger (Class 10 or UHS-1 recommended)
- Power Supply: Official USB-C power supply for your specific RPi model
- Computer: A Windows, Mac, or Linux computer to flash the SD card
- Internet Connection: Wi-Fi credentials or an Ethernet cable


## Step 1: Install Debian Trixie

The project is currently being developed on Debian Trixie. Install this version using the Raspberry Pi Imager.

1. Download and install Raspberry Pi Imager from raspberrypi.com/software

2. Open Raspberry Pi Imager and configure the image:
   - Choose Device: Select your RPi model (e.g., Raspberry Pi 5)
   - Choose OS: Select Raspberry Pi OS (64-bit) - debian trixie
   - Choose Storage: Select your microSD card
   - Click Next

3. Configure system settings:
   - Set hostname to: hampod (or your preference)
   - Enter your localization settings (timezone for your location)
   - Click Next

4. Configure user account:
   - Set username (you can use "hampod")
   - Set a strong password (diceware is recommended)
   - Click Next

5. Configure network:
   - Enter your Wi-Fi SSID and password (if not using Ethernet)
   - Click Next

6. Configure SSH:
   - Enable SSH
   - Use public key authentication if possible (makes passwordless login easier)
   - Otherwise use password authentication
   - Click Next

7. Disable Raspberry Pi Connect, then click Next

8. Verify you selected the correct drive. This is critical to avoid overwriting a system drive.

9. Click Write to flash the image, then confirm the write. Wait for completion (a few minutes).

10. Insert the microSD card into your Pi and power it on

11. Connect via SSH from your development machine:
    ```
    ssh hampod@hampod.local
    ```
    If prompted about authenticity, type "yes"

Troubleshooting: If hampod.local does not work, check your router's device list to find the Pi's IP address and use ssh hampod@192.168.x.x instead.


## Optional: Set up an SSH Key

To avoid typing your password every time, generate an SSH key on your computer and copy it to the Pi.

1. Generate a key on your development machine (if you don't have one):
   On Windows PowerShell or Mac/Linux Terminal:
   ```
   ssh-keygen -t ed25519
   ```
   Press Enter to accept defaults.

2. Copy the key to the Pi:
   On Windows PowerShell:
   ```
   type $env:USERPROFILE\.ssh\id_ed25519.pub | ssh hampod@hampod.local "mkdir -p ~/.ssh && cat >> ~/.ssh/authorized_keys"
   ```
   On Mac or Linux:
   ```
   ssh-copy-id hampod@hampod.local
   ```

3. Verify by logging out and back in. You should not be asked for a password.


## Step 2: Run the Automated Installer

The install_hampod.sh script automates the complete setup process. It installs all dependencies, clones the repository, installs Piper TTS, and builds the firmware.

1. Clone the repository and run the installer:
   ```
   cd ~
   git clone https://github.com/waynepadgett/HAMPOD2026.git
   cd HAMPOD2026/Documentation/scripts
   chmod +x install_hampod.sh
   ./install_hampod.sh
   ```

   The script will:
   - Update system packages
   - Install dependencies: git, gcc, ALSA audio libraries, Hamlib
   - Clone the HAMPOD repository to ~/HAMPOD2026
   - Install Piper TTS for speech synthesis
   - Build the Firmware
   - Build the HAL Integration Tests
   - Add your user to the audio group
   - Configure HAMPOD to start automatically on boot
   - Optionally enable SD card protection (recommended for end users, not developers)

2. Wait for completion. The script shows progress and takes approximately 5-15 minutes depending on your internet speed.

3. If the script fails, see the Manual Installation section below.


## Step 3: Verify Your Setup

After installation, run the Phase 0 integration test to verify everything works.

1. Run the test:
   ```
   cd ~/HAMPOD2026/Documentation/scripts
   ./Regression_Phase0_Integration.sh
   ```

2. What to expect:
   - The system announces "System Ready"
   - Pressing keys on the keypad announces "You pressed [Key Name]"
   - Holding keys announces "You held [Key Name]"
   - Press Ctrl+C to exit

3. If the test works, you can run the full software:
   ```
   ./run_hampod.sh
   ```

4. If you only hear clicks but no speech, see the Troubleshooting section below.

### Other Available Tests

These tests help verify different aspects of the system:

- Regression_HAL_Integration.sh: Tests the hardware abstraction layer for keypad and audio
- Regression_Imitation_Software.sh: Tests low-level pipe communication between Firmware and Software


## Troubleshooting Audio and TTS

If the integration test runs but you only hear a click sound with no spoken words, the issue is likely with Piper TTS or audio configuration.

### Verify Piper is Installed

```
which piper
piper --version
```

If piper is not found, re-run the Piper installer:
```
cd ~/HAMPOD2026/Documentation/scripts
./install_piper.sh
```

### Verify Voice Model Exists

The voice model files must be in ~/HAMPOD2026/Firmware/models/:
```
ls -la ~/HAMPOD2026/Firmware/models/
```
You should see en_US-lessac-low.onnx and en_US-lessac-low.onnx.json files. If missing, re-run the Piper installer.

### Test Piper Manually

Test that Piper can generate and play speech:
```
echo "Hello World" | piper --model ~/HAMPOD2026/Firmware/models/en_US-lessac-low.onnx --output_raw | aplay -r 16000 -c 1 -f S16_LE
```

If you hear "Hello World", Piper is working. The issue may be in how the integration test invokes it.

If you hear nothing, continue to the next step.

### Test Basic Audio Output

Verify that the audio hardware is working:
```
aplay -l
speaker-test -c 2 -t wav
```

You should hear a voice saying "Front Left" and "Front Right". If no audio is heard, check:
- Is the speaker or headphones connected and powered on?
- Is the correct audio output selected? (HDMI vs. headphone jack vs. USB audio)
- Try selecting audio output: sudo raspi-config, then System Options, then Audio

### Check User Permissions

Ensure your user is in the audio group:
```
groups
```

If "audio" is not listed:
```
sudo usermod -aG audio $USER
```
Then log out and log back in for the change to take effect.


## Manual Installation (If Automated Script Fails)

If the install_hampod.sh script fails, you can perform these steps manually.

### Update System Packages

```
sudo apt update && sudo apt upgrade -y
```

### Install Dependencies

HAMPOD requires git, make, gcc, ALSA (audio), Hamlib (rig control), and supporting tools.
```
sudo apt install -y git make gcc libasound2-dev libhamlib-dev wget bc
```

### Clone the Repository

```
cd ~
git clone https://github.com/waynepadgett/HAMPOD2026.git
```

### Install Piper TTS

```
cd ~/HAMPOD2026/Documentation/scripts
chmod +x *.sh
./install_piper.sh
```

### Build the Firmware

```
cd ~/HAMPOD2026/Firmware
make clean
make
```

If successful, you will see a firmware.elf file created.

### Build the HAL Integration Tests

```
cd ~/HAMPOD2026/Firmware/hal/tests
make clean
make
```

### Add User to Audio Group

```
sudo usermod -aG audio $USER
```
Log out and back in for this to take effect.


## Tips for Developers

### Remote Install Script

For rapid development, use the remote_install.sh script from your development machine to push code and rebuild on the Pi automatically:
```
./Documentation/scripts/remote_install.sh "Update description"
```

### Cleaning Builds

If you encounter strange build issues, try cleaning:
```
make clean
make
```

### Process Cleanup After a Crash

If the firmware or software crashes, it might leave ghost processes or named pipes that prevent restarting. Run this to clean up:
```
sudo pkill -9 firmware
sudo pkill -9 imitation_software
pkill -9 Software.elf
rm -f ~/HAMPOD2026/Firmware/Firmware_i
rm -f ~/HAMPOD2026/Firmware/Firmware_o
```

### Remote Reboot

If the system becomes unresponsive or audio drivers lock up:
```
sudo reboot
```


## SD Card Protection Mode

HAMPOD supports an optional SD card protection mode that prevents filesystem corruption if the Pi loses power unexpectedly. This is recommended for end-user deployments.

### How It Works

When enabled:
- The SD card is mounted read-only
- All file changes go to RAM and are lost on reboot
- Your terminal prompt shows **[RO]** in red
- The system is protected from power-loss corruption

When disabled (normal mode):
- Standard read-write operation
- Your terminal prompt shows **[RW]** in green
- File changes persist normally

### Managing Protection Mode

```bash
# Check current status
./power_down_protection.sh --status

# Enable protection (requires reboot)
sudo ./power_down_protection.sh --enable

# Disable protection (requires reboot)
sudo ./power_down_protection.sh --disable
```

### Saving Config Changes in Protected Mode

When protection is enabled, use the persistent write helper:
```bash
hampod_persist_write /tmp/my_config.txt /home/hampod/destination.txt
```

### When to Disable Protection

| Activity | Protection Mode |
|----------|-----------------|
| Normal HAMPOD operation | ✅ Enabled |
| Changing hampod.conf settings | ✅ Enabled (use persist helper) |
| Developing/modifying code | ❌ Disabled |
| Installing new packages | ❌ Disabled |
| System updates (apt upgrade) | ❌ Disabled |


## Auto-Start Management

HAMPOD is configured to start automatically on boot by default.

### Managing Auto-Start

```bash
# Check status
./hampod_on_powerup.sh --status

# Enable auto-start
sudo ./hampod_on_powerup.sh --enable

# Disable auto-start
sudo ./hampod_on_powerup.sh --disable


# View HAMPOD logs
sudo journalctl -u hampod -f

# Manually start/stop the service
sudo systemctl start hampod
sudo systemctl stop hampod
```

