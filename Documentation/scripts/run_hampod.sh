#!/bin/bash
# =============================================================================
# HAMPOD2026 Quick Start Script
# =============================================================================
# This script starts the full HAMPOD system for manual operation:
#   - Cleans up any stale processes and pipes
#   - Builds Firmware and Software2 (if needed)
#   - Starts both components and leaves them running
#
# Usage: ./run_hampod.sh [--no-build] [--debug]
#   --no-build: Skip the build step (faster startup if already built)
#   --debug:    Enable verbose hamlib debugging output
#
# To stop: Press Ctrl+C
#
# =============================================================================

# Refuse to run as root — causes permission issues with log files and builds
if [ "$(id -u)" -eq 0 ]; then
    echo "ERROR: Do not run this script with sudo or as root."
    echo ""
    echo "Usage: ./run_hampod.sh [--no-build] [--debug]"
    echo ""
    echo "The script will call sudo internally for steps that need it."
    exit 1
fi

set -e  # Exit on error during setup

# Configuration
# Resolve symlink so we can run from anywhere via /usr/local/bin/run_hampod
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do
  DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
done
SCRIPT_DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
FIRMWARE_DIR="$REPO_ROOT/Firmware"
SOFTWARE2_DIR="$REPO_ROOT/Software2"
SKIP_BUILD=false

# Parse arguments
for arg in "$@"; do
    if [ "$arg" = "--no-build" ]; then
        SKIP_BUILD=true
    elif [ "$arg" = "--debug" ]; then
        DEBUG_MODE=true
    fi
done

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}======================================================${NC}"
echo -e "${CYAN}  HAMPOD2026 - Starting Full System                   ${NC}"
echo -e "${CYAN}======================================================${NC}"
echo ""

# -----------------------------------------------------------------------------
# Step 1: Clean up stale processes
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 1/5] Cleaning up stale processes...${NC}"
sudo killall -9 firmware.elf 2>/dev/null || true
sudo killall -9 hampod 2>/dev/null || true
sudo killall -9 phase0_test 2>/dev/null || true
sudo killall -9 piper 2>/dev/null || true
sudo killall -9 aplay 2>/dev/null || true
sleep 1
echo "  Done."

# -----------------------------------------------------------------------------
# Step 2: Clean up stale pipes
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 2/5] Cleaning up stale pipes...${NC}"
sudo rm -f "$FIRMWARE_DIR/Firmware_i" "$FIRMWARE_DIR/Firmware_o" 2>/dev/null || true
sudo rm -f "$FIRMWARE_DIR/Speaker_i" "$FIRMWARE_DIR/Speaker_o" 2>/dev/null || true
sudo rm -f "$FIRMWARE_DIR/Keypad_i" "$FIRMWARE_DIR/Keypad_o" 2>/dev/null || true
# Clean up log files that may be owned by root from previous sudo runs
sudo rm -f /tmp/firmware.log /tmp/hampod_output.txt /tmp/hampod_debug.log 2>/dev/null || true
echo "  Done."

# -----------------------------------------------------------------------------
# Step 3: Build (optional)
# -----------------------------------------------------------------------------
if [ "$SKIP_BUILD" = true ]; then
    echo -e "${YELLOW}[Step 3/5] Skipping build (--no-build flag)${NC}"
else
    echo -e "${YELLOW}[Step 3/5] Building Firmware and Software2...${NC}"
    
    cd "$FIRMWARE_DIR"
    echo "  Building Firmware..."
    sudo make clean > /dev/null 2>&1 || true
    make > /dev/null 2>&1
    if [ ! -f "firmware.elf" ]; then
        echo -e "${RED}  ERROR: Firmware build failed${NC}"
        exit 1
    fi
    echo -e "  ${GREEN}✓${NC} firmware.elf built"
    
    cd "$SOFTWARE2_DIR"
    echo "  Building Software2..."
    sudo make clean > /dev/null 2>&1 || true
    make > /dev/null 2>&1
    if [ ! -f "bin/hampod" ]; then
        echo -e "${RED}  ERROR: Software2 build failed${NC}"
        exit 1
    fi
    echo -e "  ${GREEN}✓${NC} hampod built"
fi

# -----------------------------------------------------------------------------
# Step 4: Start Firmware
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 4/5] Starting Firmware...${NC}"
cd "$FIRMWARE_DIR"

# Read keypad layout from config file
FIRMWARE_ARGS=""
CONF_FILE="$SOFTWARE2_DIR/config/hampod.conf"
if [ -f "$CONF_FILE" ]; then
    LAYOUT=$(grep -A10 '^\[keypad\]' "$CONF_FILE" | grep '^layout' | head -1 | cut -d'=' -f2 | sed 's/#.*//' | tr -d ' ')
    if [ "$LAYOUT" = "phone" ]; then
        FIRMWARE_ARGS="--phone-layout"
        echo -e "  Keypad layout: ${GREEN}phone${NC} (positional)"
    else
        echo "  Keypad layout: calculator (default)"
    fi
fi

sudo ./firmware.elf $FIRMWARE_ARGS > /tmp/firmware.log 2>&1 &
FIRMWARE_PID=$!
echo "  Firmware PID: $FIRMWARE_PID"

# Wait for Firmware_o pipe to exist
echo "  Waiting for Firmware to create pipes..."
for i in $(seq 1 30); do
    if [ -p "Firmware_o" ]; then
        break
    fi
    sleep 0.2
done

if [ ! -p "Firmware_o" ]; then
    echo -e "${RED}  ERROR: Firmware_o pipe not created${NC}"
    echo "  Check /tmp/firmware.log for errors"
    sudo kill $FIRMWARE_PID 2>/dev/null || true
    exit 1
fi
echo -e "  ${GREEN}✓${NC} Firmware started"

# -----------------------------------------------------------------------------
# Step 5: Start Software2 (hampod)
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 5/5] Starting Software2 (hampod)...${NC}"
echo ""
echo -e "${CYAN}======================================================${NC}"
echo -e "${CYAN}  HAMPOD SYSTEM RUNNING                               ${NC}"
echo -e "${CYAN}======================================================${NC}"
echo ""
echo -e "${GREEN}The system is now running. Use the keypad to operate.${NC}"
echo ""
echo "  Press Ctrl+C to stop."
echo ""
echo -e "${CYAN}======================================================${NC}"
echo ""

cd "$SOFTWARE2_DIR"

# Trap to cleanup on exit
cleanup() {
    echo ""
    echo -e "${YELLOW}Shutting down HAMPOD...${NC}"
    sudo kill $FIRMWARE_PID 2>/dev/null || true
    sudo killall -9 firmware.elf piper aplay 2>/dev/null || true
    echo "  Firmware stopped."
    echo -e "${GREEN}Goodbye!${NC}"
}
trap cleanup EXIT

# Run hampod interactively
HAMPOD_ARGS=""
if [ "$DEBUG_MODE" = true ]; then
    HAMPOD_ARGS="--debug"
fi

sudo ./bin/hampod $HAMPOD_ARGS 2>&1 | tee /tmp/hampod_output.txt
