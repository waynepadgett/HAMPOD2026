# HAMPOD Install Script Plan

**Created:** 2025-12-27  
**Status:** ✅ Implemented - Script created at `Documentation/scripts/install_hampod.sh`

---

## 🎯 Objective

Create a single install script (`install_hampod.sh`) that automates the entire Raspberry Pi setup process from the `RPi_Setup_Guide.md`. The script should:

1. Run all setup steps automatically after the user has prepared SSH access
2. Complete the entire setup in a **single SSH session** (no password re-prompts)
3. Stop before regression tests and display a success message
4. Suggest running the first regression test with the exact CLI command

---

## 📋 Prerequisites (User Must Do Manually)

These steps **cannot be automated** and must be done by the user before running the install script:

### Step 1: Flash the SD Card (Manual)
- Download and install Raspberry Pi Imager
- Flash Debian Trixie to microSD card
- Configure hostname (e.g., `hampod`)
- Set username and password
- Configure Wi-Fi if needed
- **Enable SSH** (critical!)
- Optionally configure public key authentication during flash

### Step 2: First Boot and SSH Connection (Manual)
- Insert SD card and power on RPi
- Connect from development machine:
  ```bash
  ssh hampod@hampod.local
  ```

### Step 3: Optional SSH Key Setup (Recommended, Manual)
The user should either:
- Use public key authentication during the Imager setup, **OR**
- Setup SSH key after first boot:
  ```bash
  # On development machine (Mac/Linux):
  ssh-keygen -t ed25519  # if no key exists
  ssh-copy-id hampod@hampod.local
  ```

---

## 🤖 What the Script Will Automate

Once connected via SSH, the user runs the install script and it handles everything below:

### Phase 1: System Update & Dependencies
```bash
# Update package lists and upgrade system
sudo apt update && sudo apt upgrade -y

# Install development tools & audio libraries
sudo apt install -y git make gcc libasound2-dev libhamlib-dev
```

### Phase 2: Clone Repository
```bash
# Clone HAMPOD repository to home directory
cd ~
git clone https://github.com/waynepadgett/HAMPOD2026.git
```

**Note:** Script should check if repository already exists and handle accordingly (skip or pull latest).

### Phase 3: Install Piper TTS
```bash
# Make scripts executable
cd ~/HAMPOD2026
chmod +x Documentation/scripts/*.sh

# Run Piper installer
cd ~/HAMPOD2026/Documentation/scripts
./install_piper.sh
```

### Phase 4: Build Firmware
```bash
# Build the Firmware
cd ~/HAMPOD2026/Firmware
make clean
make
```

### Phase 5: Build HAL Integration Test
```bash
# Build the HAL integration test
cd ~/HAMPOD2026/Firmware/hal/tests
make
```

### Phase 6: Add User to Audio Group
```bash
# Ensure user is in audio group
sudo usermod -aG audio $USER
```

**Note:** The group change takes effect on next login, but the current session should still work for basic audio testing.

---

## 🖥️ Script Output Design

### Progress Indicators
The script should provide clear feedback:

```
╔══════════════════════════════════════════════════════════════╗
║                 HAMPOD Installation Script                    ║
╠══════════════════════════════════════════════════════════════╣
║ This script will set up your Raspberry Pi for HAMPOD.        ║
║ Estimated time: 5-15 minutes                                  ║
╚══════════════════════════════════════════════════════════════╝

[1/6] Updating system packages...                          ✓
[2/6] Installing dependencies (git, gcc, ALSA, Hamlib)...  ✓
[3/6] Cloning HAMPOD repository...                         ✓
[4/6] Installing Piper TTS...                              ✓
[5/6] Building Firmware...                                 ✓
[6/6] Building HAL Integration Tests...                    ✓

╔══════════════════════════════════════════════════════════════╗
║                   🎉 SETUP SUCCESSFUL! 🎉                     ║
╠══════════════════════════════════════════════════════════════╣
║ Your Raspberry Pi is now configured for HAMPOD development.  ║
║                                                               ║
║ NEXT STEP: Run the first regression test to verify setup:    ║
║                                                               ║
║   cd ~/HAMPOD2026/Documentation/scripts                       ║
║   ./Regression_Phase0_Integration.sh                          ║
║                                                               ║
║ WHAT TO EXPECT:                                               ║
║   - System announces "System Ready"                           ║
║   - Pressing keys announces "You pressed [Key Name]"          ║
║   - Holding keys announces "You held [Key Name]"              ║
║   - Press Ctrl+C to exit                                      ║
║                                                               ║
║ If you only hear clicks (no speech), see troubleshooting      ║
║ in Documentation/Project_Overview_and_Onboarding/             ║
║ RPi_Setup_Guide.md                                            ║
╚══════════════════════════════════════════════════════════════╝
```

### Error Handling
Each step should:
- Check for success/failure
- Display clear error messages if something fails
- Suggest how to fix the issue
- Exit gracefully (don't continue if critical step fails)

---

## 📁 File Structure

```
Documentation/
├── Project_Overview_and_Onboarding/
│   ├── RPi_Setup_Guide.md           # Existing manual guide
│   ├── Hampod_Install_Script_Plan.md # This plan document
│   └── ...
└── scripts/
    ├── install_hampod.sh            # NEW: The install script
    ├── install_piper.sh             # Existing Piper installer (called by new script)
    ├── Regression_HAL_Integration.sh
    └── ...
```

---

## 🔧 Script Features

### 1. Single SSH Session Execution
The script runs entirely in the current shell session, so:
- No need to re-authenticate for `sudo` (uses cached credentials)
- All steps complete before connection closes
- User can optionally run the regression test immediately after

### 2. Idempotency Checks
The script should be safe to run multiple times:
- Check if HAMPOD2026 directory exists before cloning
- Check if Piper is already installed
- Use `make clean` to ensure fresh builds

### 3. Minimal User Interaction
- **No prompts** during execution (fully automated)
- Uses `-y` flag for apt commands
- Provides clear output instead of requiring user input

### 4. Error Recovery Suggestions
If a step fails, provide actionable guidance:
```
[ERROR] Piper installation failed!

Try these steps:
1. Check internet connection: ping -c 3 google.com
2. Re-run the installer: ./Documentation/scripts/install_piper.sh
3. See RPi_Setup_Guide.md for manual instructions
```

---

## ✅ Implementation Checklist

When implementing the script, ensure:

- [ ] Shebang: `#!/bin/bash`
- [ ] Error handling with `set -e` (exit on error)
- [ ] Color-coded output for success/error states
- [ ] Check for existing installations (idempotent)
- [ ] Handle repository already cloned scenario
- [ ] Verify each step completed successfully
- [ ] Clear success message at the end
- [ ] Suggest exact command to run first regression test
- [ ] Include "What to Expect" section for the test
- [ ] Make script executable: `chmod +x install_hampod.sh`

---

## 🔄 Usage Flow

### Full User Journey:

1. **User flashes SD card** with Raspberry Pi Imager (sets up SSH, Wi-Fi, etc.)
2. **User boots the Pi** and waits for it to come online
3. **User connects via SSH**:
   ```bash
   ssh hampod@hampod.local
   ```
4. **User downloads and runs the install script**:
   ```bash
   # Option A: If repo is already cloned locally
   ./HAMPOD2026/Documentation/scripts/install_hampod.sh
   
   # Option B: One-liner to fetch and run (if we host it)
   curl -sSL https://raw.githubusercontent.com/waynepadgett/HAMPOD2026/main/Documentation/scripts/install_hampod.sh | bash
   ```
5. **Script runs automatically** (5-15 minutes)
6. **User sees success message** with next steps
7. **User runs suggested regression test** to verify

---

## ⚠️ Considerations

### Things to Watch Out For:

1. **Network Timeouts**: Long downloads (Piper) might fail on slow connections
   - Solution: Add retry logic for downloads

2. **Existing Installations**: User might run script twice
   - Solution: Check if components exist before installing

3. **Audio Group Changes**: `usermod -aG audio` doesn't take effect immediately
   - Solution: Note this in success message, suggest logout/login if issues

4. **ARM Architecture**: Piper binary must match Pi architecture
   - Solution: Rely on `install_piper.sh` which already handles this

5. **Disk Space**: Ensure sufficient space before starting
   - Solution: Add disk space check at beginning

---

## 📝 Updates to Existing Documentation

After script is created, update `RPi_Setup_Guide.md` to:
1. Reference the new install script as the primary method
2. Keep manual steps as backup/reference
3. Add a "Quick Start" section at the top pointing to the script

---

## 🚦 Approval Request

**Please review this plan and let me know:**

1. Does the scope look correct?
2. Any additional features needed?
3. Any steps to add or remove?
4. Preferred location for the script (`Documentation/scripts/` recommended)?
5. Should "Option B" one-liner be supported (curl from GitHub)?

Once approved, I'll implement the `install_hampod.sh` script.
