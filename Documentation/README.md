# HAMPOD Documentation

This directory contains documentation for the HAMPOD project.

## Contents
- [Project Overview](../README.md)

## Deployment

To deploy changes to the Raspberry Pi, we use a Git-based workflow (Push/Pull).

### Prerequisites

1.  **Git** must be installed on the Raspberry Pi (already verified).
2.  The repository must be cloned on the Raspberry Pi.

#### First-time Setup on Pi

SSH into the Pi and clone the repository:
```bash
ssh waynesr@HAMPOD.local
cd ~
# If the directory exists but is empty/not a repo, remove it first: rm -rf HAMPOD2026
git clone https://github.com/waynepadgett/HAMPOD2026.git
```

### Deployment Workflow

1.  **Local Machine:** Commit and push your changes to GitHub.
    ```bash
    git add .
    git commit -m "Your commit message"
    git push origin main
    ```

2.  **Raspberry Pi:** Pull the changes.
    ```bash
    ssh waynesr@HAMPOD.local
    cd ~/HAMPOD2026
    git pull origin main
    ```

