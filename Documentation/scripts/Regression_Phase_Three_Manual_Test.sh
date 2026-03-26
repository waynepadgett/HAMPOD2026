#!/bin/bash
# =============================================================================
# HAMPOD2026 Manual Test: Phase 3 — Configuration Mode
# =============================================================================
# This script sets up and runs the full HAMPOD system for manual testing of:
#   - Entering and exiting Configuration Mode (Hold C)
#   - Parameter navigation (A/B keys)
#   - Value adjustments (C/D keys)
#   - Speech Speed limits (down to 0.1x)
#   - Keypad Layout changes (Calculator vs Phone)
#   - Save / Discard / Apply Session logic
#
# Usage: sudo ./Regression_Phase_Three_Manual_Test.sh
#
# Prerequisites:
#   - USB keypad connected
#   - USB audio device connected
#   - Piper TTS model installed in Firmware/models/
#   - (Radio connected via USB is optional but recommended)
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
echo -e "${CYAN}  HAMPOD2026 Phase 3 — Configuration Mode Manual Test ${NC}"
echo -e "${CYAN}======================================================${NC}"
echo ""

# -----------------------------------------------------------------------------
# Step 1: Clean up stale processes
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 1/7] Cleaning up stale processes...${NC}"
sudo killall -9 firmware.elf 2>/dev/null || true
sudo killall -9 hampod 2>/dev/null || true
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
sudo rm -f "$FIRMWARE_DIR/Keypad_i" "$FIRMWARE_DIR/Keypad_o" 2>/dev/null || true
rm -f /tmp/hampod_output.txt /tmp/firmware.log /tmp/hampod_debug.log 2>/dev/null || true
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
echo -e "${GREEN}Keypad Legend (Calculator Layout):${NC}"
echo "  [A] = ( / )   [B] = ( * )   [C] = ( - )   [D] = ( + )"
echo ""
echo -e "${GREEN}Instructions:${NC}"
echo ""
echo "  STARTUP:"
echo "    - You should hear \"Ready\" followed by the current frequency"
echo ""
echo -e "${CYAN}--- PART 1: Entering Config Mode & Navigation ---${NC}"
echo ""
echo "  1. Hold [C] (the minus '-' minus key on standard numpad) for ~1 sec."
echo "     - You should hear \"Configuration Mode\" then the first parameter"
echo "       (likely \"Volume: XX percent\")."
echo ""
echo "  2. Press [A] (slash '/') to navigate forward through the options:"
echo "     - Volume -> Speech Speed -> Keypad Layout -> System Reboot/Shutdown"
echo ""
echo "  3. Press [B] (asterisk '*') to navigate backward."
echo ""
echo -e "${CYAN}--- PART 2: Adjusting Settings ---${NC}"
echo ""
echo "  4. Navigate to \"Volume\"."
echo "     - Press [C] to increase volume. Verify the announcement gets louder."
echo "     - Press [D] to decrease volume."
echo ""
echo "  5. Navigate to \"Speech Speed\"."
echo "     - Press [D] to decrease the speed repeatedly."
echo "     - Verify it drops all the way down to a very fast 0.1x speed."
echo "     - Press [C] to bring it back to a comfortable level (around 1.0x)."
echo ""
echo "  6. Navigate to \"Keypad Layout\"."
echo "     - Press [C] or [D] to switch it between \"Calculator\" and \"Phone\"."
echo "     - Note: While inside the menu, the A/B/C/D navigation keys will NOT"
echo "       swap positions immediately, to prevent getting stuck or confused."
echo ""
echo -e "${CYAN}--- PART 3: Exit Strategies ---${NC}"
echo ""
echo "  7. Exiting & Saving (Hold C):"
echo "     - Change the layout to \"Phone\"."
echo "     - Hold [C] to exit. You should hear \"Configuration Saved\"."
echo "     - The keys are now in Phone Layout! Press [7], [8], and [9] (the "
echo "       next to bottom row on the keypad) and confirm what they do."
echo "     - Hold the NEW position of [C] (the third key down on the right)"
echo "       to re-enter Configuration Mode. Navigate back and set the"
echo "       layout back to \"Calculator\"."
echo "     - Hold [C] to exit again and save."
echo ""
echo "  8. Exiting & Discarding (Hold B):"
echo "     - Re-enter Config Mode (Hold C)."
echo "     - Change Volume and Speech Speed to extreme values."
echo "     - Hold [B] to Discard. You should hear \"Configuration Cancelled\"."
echo "     - Verify volume and speech speed instantly return to your old defaults."
echo ""
echo "  9. Exiting without Saving to File (Hold D):"
echo "     - Re-enter Config Mode (Hold C)."
echo "     - Change the volume."
echo "     - Hold [D] to Apply. You should hear \"Configuration applied for this session\"."
echo "     - The volume should remain at your new setting for now, but will vanish on reboot."
echo ""
echo " 10. Press Ctrl+C to exit when done"
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

# Run hampod interactively — stderr (Hamlib debug) goes to log, not the screen
sudo ./bin/hampod 2>/tmp/hampod_debug.log | tee /tmp/hampod_output.txt

echo ""
echo -e "${CYAN}======================================================${NC}"
echo -e "${CYAN}                  TEST COMPLETE                       ${NC}"
echo -e "${CYAN}======================================================${NC}"
echo ""
echo "Please verify the following:"
echo ""
echo "  1.  Did holding [C] successfully enter Configuration Mode? (Y/N)"
echo "  2.  Did [A] and [B] correctly navigate between Volume, Speed, Layout, and Shutdown? (Y/N)"
echo "  3.  Did [C] and [D] successfully increase/decrease values? (Y/N)"
echo "  4.  Were you able to lower the speech speed all the way down to 0.1? (Y/N)"
echo "  5.  Did holding [C] correctly save changes (like Layout) upon exiting? (Y/N)"
echo "  6.  Did holding [B] correctly cancel/discard changes? (Y/N)"
echo "  7.  Did holding [D] correctly exit with 'applied for this session' message? (Y/N)"
echo ""
echo "If all answers are YES, the test PASSED."
echo "If any answer is NO, investigate the issue."
echo ""
echo "Logs: /tmp/firmware.log, /tmp/hampod_debug.log, /tmp/hampod_output.txt"
