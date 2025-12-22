#!/bin/bash
# =============================================================================
# HAMPOD2026 Regression Test: Phase 1 Manual Radio Test
# =============================================================================
# This script sets up and runs the frequency mode for manual testing:
#   - Cleans up stale processes and pipes
#   - Builds Firmware and Software2
#   - Starts Firmware
#   - Starts Software2 hampod binary for manual testing
#
# Manual verification steps:
#   - Press [#] to enter frequency mode, hear "Current VFO"
#   - Enter digits (1,4,*,2,5,0) with audio echo
#   - Press [#] to submit, verify radio shows 14.250 MHz
#   - Turn VFO dial, wait 1 second, hear frequency announced
#
# Usage: ./Regression_Phase_One_Manual_Radio_Test.sh
#
# Prerequisites:
#   - USB keypad connected
#   - USB audio device connected
#   - Radio connected via USB (for frequency set/get)
#   - Piper TTS model installed in Firmware/models/
#
# =============================================================================

set -e  # Exit on error during setup

# Configuration
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
FIRMWARE_DIR="$REPO_ROOT/Firmware"
SOFTWARE2_DIR="$REPO_ROOT/Software2"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}======================================================${NC}"
echo -e "${CYAN}  HAMPOD2026 Phase 1 - Manual Radio Test              ${NC}"
echo -e "${CYAN}======================================================${NC}"
echo ""
echo "Firmware dir: $FIRMWARE_DIR"
echo "Software2 dir: $SOFTWARE2_DIR"
echo ""

# -----------------------------------------------------------------------------
# Step 1: Clean up stale processes
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 1/7] Cleaning up stale processes...${NC}"
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
echo -e "${YELLOW}[Step 2/7] Cleaning up stale pipes...${NC}"
sudo rm -f "$FIRMWARE_DIR/Firmware_i" "$FIRMWARE_DIR/Firmware_o" 2>/dev/null || true
sudo rm -f "$FIRMWARE_DIR/Speaker_i" "$FIRMWARE_DIR/Speaker_o" 2>/dev/null || true
sudo rm -f "$FIRMWARE_DIR/Keypad_o" 2>/dev/null || true
rm -f /tmp/hampod_output.txt /tmp/firmware.log 2>/dev/null || true
echo "  Done."

# -----------------------------------------------------------------------------
# Step 3: Verify hardware is connected
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 3/7] Verifying hardware...${NC}"

# Check for USB keypad
KEYPAD_DEVICE=$(ls /dev/input/by-id/*kbd* 2>/dev/null | head -1 || echo "")
if [ -z "$KEYPAD_DEVICE" ]; then
    echo -e "${RED}  ERROR: No USB keypad found!${NC}"
    exit 1
fi
echo -e "  ${GREEN}✓${NC} USB Keypad: $KEYPAD_DEVICE"

# Check for USB audio
AUDIO_DEVICE=$(aplay -l 2>/dev/null | grep -i "usb\|device" | head -1 || echo "")
if [ -z "$AUDIO_DEVICE" ]; then
    echo -e "${YELLOW}  WARNING: No USB audio device detected${NC}"
else
    echo -e "  ${GREEN}✓${NC} Audio: $AUDIO_DEVICE"
fi

# Check for USB serial (radio)
RADIO_DEVICE=$(ls /dev/ttyUSB* 2>/dev/null | head -1 || echo "")
if [ -z "$RADIO_DEVICE" ]; then
    echo -e "${YELLOW}  WARNING: No USB serial device (radio) found${NC}"
    echo -e "${YELLOW}           Frequency mode will work but cannot control radio${NC}"
else
    echo -e "  ${GREEN}✓${NC} Radio: $RADIO_DEVICE"
fi

# Check for Piper model
PIPER_MODEL="$FIRMWARE_DIR/models/en_US-lessac-low.onnx"
if [ ! -f "$PIPER_MODEL" ]; then
    echo -e "${RED}  ERROR: Piper model not found at $PIPER_MODEL${NC}"
    exit 1
fi
echo -e "  ${GREEN}✓${NC} Piper model: $(basename $PIPER_MODEL)"

# -----------------------------------------------------------------------------
# Step 4: Build Firmware
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 4/7] Building Firmware...${NC}"
cd "$FIRMWARE_DIR"
make clean > /dev/null 2>&1 || true
make 2>&1 | while read line; do echo "  $line"; done

if [ ! -f "firmware.elf" ]; then
    echo -e "${RED}  ERROR: Firmware build failed${NC}"
    exit 1
fi
echo -e "  ${GREEN}✓${NC} firmware.elf built"

# -----------------------------------------------------------------------------
# Step 5: Build Software2
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 5/7] Building Software2...${NC}"
cd "$SOFTWARE2_DIR"
make clean > /dev/null 2>&1 || true
make 2>&1 | while read line; do echo "  $line"; done

if [ ! -f "bin/hampod" ]; then
    echo -e "${RED}  ERROR: Software2 build failed${NC}"
    exit 1
fi
echo -e "  ${GREEN}✓${NC} hampod built"

# -----------------------------------------------------------------------------
# Step 6: Start Firmware
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 6/7] Starting Firmware...${NC}"
cd "$FIRMWARE_DIR"
sudo ./firmware.elf > /tmp/firmware.log 2>&1 &
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
# Step 7: Run Software2 (hampod) for manual testing
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 7/7] Starting Software2 for manual testing...${NC}"
echo ""
echo -e "${CYAN}======================================================${NC}"
echo -e "${CYAN}              MANUAL TESTING MODE                     ${NC}"
echo -e "${CYAN}======================================================${NC}"
echo ""
echo -e "${GREEN}Instructions:${NC}"
echo "  1. Press [#] to enter frequency mode"
echo "     - You should hear 'Current VFO'"
echo "     - Press [#] again to cycle: VFO A, VFO B, Current VFO"
echo ""
echo "  2. Enter a frequency (e.g., 14.250 MHz):"
echo "     - Press: 1, 4, *, 2, 5, 0"
echo "     - You should hear each digit/point spoken"
echo ""
echo "  3. Press [#] to submit"
echo "     - You should hear 'Frequency set' and the frequency"
echo "     - Check radio display shows 14.250 MHz"
echo ""
echo "  4. Turn VFO dial on radio, wait 1 second"
echo "     - Frequency should be announced automatically"
echo ""
echo "  5. Press Ctrl+C to exit when done"
echo ""
echo -e "${CYAN}======================================================${NC}"
echo ""

cd "$SOFTWARE2_DIR"

# Trap to cleanup on exit
cleanup() {
    echo ""
    echo -e "${YELLOW}Cleaning up...${NC}"
    sudo kill $FIRMWARE_PID 2>/dev/null || true
    sudo killall -9 firmware.elf piper aplay 2>/dev/null || true
    echo "Done."
}
trap cleanup EXIT

# Run hampod interactively
sudo ./bin/hampod 2>&1 | tee /tmp/hampod_output.txt

echo ""
echo -e "${CYAN}======================================================${NC}"
echo -e "${CYAN}                  TEST COMPLETE                       ${NC}"
echo -e "${CYAN}======================================================${NC}"
echo ""
echo "Please answer the following questions:"
echo ""
echo "  1. Did you hear 'Frequency mode ready' on startup? (Y/N)"
echo "  2. Did you hear digit echoes when entering frequency? (Y/N)"
echo "  3. Did the radio display show the correct frequency? (Y/N)"
echo "  4. Did VFO dial changes get announced after 1 second? (Y/N)"
echo ""
echo "If all answers are YES, the test PASSED."
echo "If any answer is NO, investigate the issue."
echo ""
echo "Logs preserved in /tmp/firmware.log and /tmp/hampod_output.txt"
