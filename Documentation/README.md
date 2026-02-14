# HAMPOD Documentation

This directory contains documentation for the HAMPOD project.

## Contents
- [Project Overview](../README.md)
- [**RPi Setup Guide**](Project_Overview_and_Onboarding/RPi_Setup_Guide.md) 👈 **Start Here**
- [Raspberry Pi Migration Plan](Hampod%20RPi%20change%20plan.md)

### Project specification and plan (Project_Spec)
- [Project Plan](Project_Spec/Project_Plan.md) – Integration/testing strategy, risks & mitigations, milestones, code readability
- [Refactor Todos](Project_Spec/Refactor_Todos.md) – Checklist to reach maximum clarity (docs, code, structure, tests)
- [Hardware Constraints](Project_Spec/Hardware_Constraints.md)
- [Project Objectives and Target Users](Project_Spec/Project_Objectives_and_Target_Users.md)
- [Project Functional Requirements](Project_Spec/Project_Functional_Requirements.md)
- [System Architecture](Project_Spec/System_Architecture.md)
- [System Architecture (detailed)](Project_Spec/System_Architecture_Detail.md)

## Migration Status

**Current Platform:** Raspberry Pi (migrated from NanoPi)

### Completed
- ✅ **Phase 1-3**: Hardware Abstraction Layer (HAL) implementation and integration
- ✅ USB Keypad support via HAL
- ✅ USB Audio support via HAL (auto-detects device using plughw format)
- ✅ Piper TTS integration (16kHz sample rate)
- ✅ WiringPi dependencies removed
- ✅ Firmware builds successfully on Raspberry Pi

### In Progress
- 🚧 Full system integration testing with Software layer

See [Hampod_RPi_change_plan.md](Hampod_RPi_change_plan.md) for detailed migration progress.

## Deployment

To deploy changes to the Raspberry Pi, we use a Git-based workflow (Push/Pull).

### Prerequisites

1.  **Git** must be installed on the Raspberry Pi (already verified).
2.  The repository must be cloned on the Raspberry Pi.

#### First-time Setup on Pi

SSH into the Pi and clone the repository:
```bash
ssh hampod@hampod.local
cd ~
# If the directory exists but is empty/not a repo, remove it first: rm -rf HAMPOD2026
git clone https://github.com/waynepadgett/HAMPOD2026.git
```

### Deployment Workflow

#### Step 1: Commit and Push from Local Machine

On your local machine (Windows), commit and push your changes to GitHub:
```bash
git add .
git commit -m "Your commit message"
git push origin main
```

#### Step 2: Sync to Raspberry Pi

You have two options to pull the latest changes on the Pi:

**Option A: One-liner (Recommended)**
```bash
ssh hampod@hampod.local "cd ~/HAMPOD2026 && git pull origin main"
```

**Option B: Interactive SSH**
```bash
ssh hampod@hampod.local
cd ~/HAMPOD2026
git pull origin main
exit
```

Both options achieve the same result. Option A is faster for quick syncs.

#### Option C: Automated Script (Best for frequent updates)

We have created a script to automate the entire process (Commit -> Push -> Pull -> Make).

1.  **Run the script from Git Bash or your terminal:**
    ```bash
    ./Documentation/scripts/remote_install.sh "Your commit message"
    ```

    This script will:
    1.  Add all local changes.
    2.  Commit with your message.
    3.  Push to GitHub.
    4.  SSH into the Pi, pull the changes, and run `make` in the `Software` directory.

    *Note: You may need to make the script executable first: `chmod +x Documentation/scripts/remote_install.sh`*

