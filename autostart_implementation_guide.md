# Autostart Implementation Guide

This document explains the steps required to add the "Auto-Start on Power-Up" feature to your older copy of the HAMPOD repository.

## 1. Overview of the Architecture

To run HAMPOD automatically on boot, we use a **systemd service**. Because HAMPOD has multiple components (the firmware background process and the main software) that communicate via named pipes, simply pointing systemd to the binaries is not enough.

Instead, we use a two-part approach:
1. **A systemd service file** (`hampod.service`) that tells the OS to start our service on boot with root privileges.
2. **A wrapper script** (`run_hampod_service.sh`) that safely handles starting, synchronizing, and cleaning up the HAMPOD processes and their pipes.

## 2. The Core Problem: Root Ownership and Named Pipes

When systemd runs a service, it typically runs as the `root` user. This means that when the `firmware.elf` starts and creates its named pipes (e.g., `Firmware_o`, `Firmware_i`), those pipes will be **owned by root**.

**Why is this a problem?**
If the service stops unexpectedly, or if you disable the service and try to run HAMPOD manually as a normal user (`pi`), the normal user will not have permission to read/write to the root-owned pipes left over from the systemd run. The software will crash with "Permission Denied" errors.

**The Solution:**
The wrapper script MUST contain a robust `cleanup()` function that explicitly deletes these named pipes and kills stale processes *before* starting anything. 

## 3. Step-by-Step Implementation

You will need to create two scripts in your repository (we recommend placing them in a `scripts` or `Documentation/scripts` directory).

### Step 3A: Create the Wrapper Script (`run_hampod_service.sh`)

Create a script named `run_hampod_service.sh`. This script will be called by systemd.

```bash
#!/bin/bash
# run_hampod_service.sh

# Adjust these paths to match your repo structure
REPO_ROOT="/home/pi/HAMPOD2026"
FIRMWARE_DIR="$REPO_ROOT/Firmware"
SOFTWARE2_DIR="$REPO_ROOT/Software2"

# 1. Cleanup: This is CRITICAL to solve the root ownership pipe issues!
echo "Cleaning up stale processes and pipes..."
killall -9 firmware.elf hampod piper aplay 2>/dev/null || true

# Explicitly delete any old root-owned pipes
rm -f "$FIRMWARE_DIR/Firmware_i" "$FIRMWARE_DIR/Firmware_o" 2>/dev/null || true
rm -f "$FIRMWARE_DIR/Speaker_i" "$FIRMWARE_DIR/Speaker_o" 2>/dev/null || true
rm -f "$FIRMWARE_DIR/Keypad_i" "$FIRMWARE_DIR/Keypad_o" 2>/dev/null || true
sleep 1

# 2. Start Firmware
echo "Starting Firmware..."
cd "$FIRMWARE_DIR"
./firmware.elf > /tmp/hampod_firmware.log 2>&1 &
FIRMWARE_PID=$!

# Wait for Firmware to create its pipes before starting the main software
for i in $(seq 1 30); do
    if [ -p "$FIRMWARE_DIR/Firmware_o" ]; then
        break
    fi
    sleep 0.2
done

# 3. Start Main Software
echo "Starting Software2..."
cd "$SOFTWARE2_DIR"
./bin/hampod > /tmp/hampod_software.log 2>&1 &

# Systemd (Type=forking) expects the parent script to exit with success
exit 0
```

> **Note:** Don't forget to make it executable: `chmod +x run_hampod_service.sh`

### Step 3B: Create the Service Installer (`hampod_on_powerup.sh`)

Create a script to generate and enable the systemd service.

```bash
#!/bin/bash
# hampod_on_powerup.sh

if [ "$EUID" -ne 0 ]; then
    echo "Please run as root (sudo ./hampod_on_powerup.sh)"
    exit 1
fi

# Adjust to your absolute paths
REPO_ROOT="/home/pi/HAMPOD2026"
WRAPPER_SCRIPT="$REPO_ROOT/scripts/run_hampod_service.sh"
SERVICE_FILE="/etc/systemd/system/hampod.service"

echo "Creating systemd service..."

cat > "$SERVICE_FILE" << EOF
[Unit]
Description=HAMPOD Auto-Start Service
After=multi-user.target sound.target

[Service]
Type=forking
User=root
WorkingDirectory=${REPO_ROOT}/Firmware
ExecStartPre=/bin/sleep 3
ExecStart=${WRAPPER_SCRIPT}
ExecStop=/bin/bash -c 'killall -9 firmware.elf hampod 2>/dev/null || true'
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

echo "Enabling service to start on boot..."
systemctl daemon-reload
systemctl enable hampod

echo "Auto-start configured successfully!"
echo "To start now, run: sudo systemctl start hampod"
```

> **Note:** Don't forget to make it executable: `chmod +x hampod_on_powerup.sh`

## 4. Summary of Key Fixes
- **`Type=forking`**: This allows the wrapper script to spawn background processes and then successfully exit, telling systemd that the processes are running.
- **Pipe Deletion (`rm -f`)**: Guarantees that if a previous crash occurred as root, the leftover root-owned pipes are safely destroyed, allowing the next run to recreate them with the correct permissions and avoid "Permission Denied".
- **`ExecStartPre=/bin/sleep 3`**: Adds a short buffer delay on system boot to ensure ALSA and the sound card hardware are fully initialized before the script attempts to use text-to-speech.
