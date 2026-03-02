#!/bin/bash
# =============================================================================
# HAMPOD2026 Update Script
# =============================================================================
# A utility to safely update the HAMPOD codebase from GitHub.
# It preserves the local configuration file during updates, cleans the build,
# pulls the latest code, and rebuilds the firmware.
#
# Usage: ./update_hampod.sh
# =============================================================================

set -e

# Resolve the real path if the script is called via a symlink (e.g., from /usr/local/bin/hampod)
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
done
SCRIPT_DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
SOFTWARE2_DIR="$REPO_ROOT/Software2"
FIRMWARE_DIR="$REPO_ROOT/Firmware"
CLI_SCRIPT="$SCRIPT_DIR/hampod_cli.sh"
CONFIG_FILE="$SOFTWARE2_DIR/config/hampod.conf"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

function print_header() {
    echo -e "${CYAN}======================================================${NC}"
    echo -e "${CYAN}  $1${NC}"
    echo -e "${CYAN}======================================================${NC}"
    echo ""
}

function print_error() {
    echo -e "${RED}ERROR: $1${NC}"
}

function print_success() {
    echo -e "${GREEN}SUCCESS: $1${NC}"
}

print_header "HAMPOD Updater"

# 1. Stash the current config if it exists
if [ -f "$CONFIG_FILE" ]; then
    echo "Backing up current configuration..."
    cp "$CONFIG_FILE" /tmp/hampod_update_backup.conf
    HAS_BACKUP=true
else
    echo "No existing configuration found. Proceeding..."
    HAS_BACKUP=false
fi

# 2. Stop running processes
echo "Stopping any running HAMPOD processes..."
"$CLI_SCRIPT" reset > /dev/null 2>&1 || true

# 3. Clean the repository and pull
echo "Cleaning and pulling latest code from GitHub..."
cd "$REPO_ROOT"
# Clean the firmware build specifically
cd "$FIRMWARE_DIR" && make clean > /dev/null 2>&1 || true
cd "$REPO_ROOT"

# Stash any other untracked/modified changes to prevent pull conflicts
git stash > /dev/null 2>&1 || true
git pull origin main || git pull origin master

# 4. Restore the configuration
if [ "$HAS_BACKUP" = true ]; then
    echo "Restoring configuration..."
    # Ensure directory exists just in case
    mkdir -p "$SOFTWARE2_DIR/config" 2>/dev/null || sudo mkdir -p "$SOFTWARE2_DIR/config"
    if cp /tmp/hampod_update_backup.conf "$CONFIG_FILE" 2>/dev/null || sudo cp /tmp/hampod_update_backup.conf "$CONFIG_FILE"; then
        sudo chown $USER:$USER "$CONFIG_FILE" 2>/dev/null || true
    fi
    rm -f /tmp/hampod_update_backup.conf
fi

# 5. Rebuild the firmware
echo "Rebuilding Firmware..."
cd "$FIRMWARE_DIR"
if make; then
    print_success "Firmware compiled successfully."
else
    print_error "Firmware compilation failed!"
    exit 1
fi

print_success "HAMPOD has been successfully updated!"
echo "You can now run 'hampod start' to launch the system."
