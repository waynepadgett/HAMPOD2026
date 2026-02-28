#!/bin/bash
# =============================================================================
# HAMPOD2026 Manual Test: Phase 2 — Normal Mode, Set Mode, Queries
# =============================================================================
# This script sets up and runs the full HAMPOD system for manual testing of:
#   - Normal mode query keys (frequency, mode, VFO, preamp, AGC, etc.)
#   - Set mode (change radio parameters like power, mic gain)
#   - Announcements toggle
#   - Speech interrupt behavior
#
# Manual verification steps:
#   - On startup, hear "Ready" followed by current frequency
#   - [0] announces current mode (e.g. "USB")
#   - [1] selects VFO A + frequency, hold [1] selects VFO B
#   - [2] announces current frequency
#   - [4] press = preamp, hold = AGC, shift+press = attenuation
#   - [7] announces noise blanker status
#   - [8] press = noise reduction, hold = mic gain
#   - [9] hold = power level, shift+press = compression
#   - [.] press = S-meter, hold = power meter
#   - [-] toggles announcements on/off
#   - [/] toggles shift on/off
#   - [*] enters set mode, then use query keys to select parameter
#   - Speech interrupt: press any key while speaking to interrupt
#
# Usage: sudo ./Regression_Phase_Two_Manual_Test.sh
#
# Prerequisites:
#   - USB keypad connected
#   - USB audio device connected
#   - Radio connected via USB
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
echo -e "${CYAN}  HAMPOD2026 Phase 2 — Normal Mode Manual Test        ${NC}"
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

# Check for USB serial (radio)
RADIO_DEVICE=$(ls /dev/ttyUSB* 2>/dev/null | head -1 || echo "")
if [ -z "$RADIO_DEVICE" ]; then
    echo -e "${YELLOW}  WARNING: No USB serial device (radio) found${NC}"
    echo -e "${YELLOW}           Normal mode queries may fail${NC}"
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
echo -e "${GREEN}Keypad Legend (current keypad → old HamPod keymap):${NC}"
echo "  [/] = Shift (was [A]),  [*] = Set (was [B])"
echo "  [-] = Toggle (was [C]), [+] = Clear (was [D])"
echo "  [.] = Decimal/Star,    [Enter] = Enter/Hash"
echo ""
echo -e "${GREEN}Instructions:${NC}"
echo ""
echo "  STARTUP:"
echo "    - You should hear \"Ready\" followed by the current frequency"
echo ""
echo -e "${CYAN}--- PART 1: Normal Mode Queries ---${NC}"
echo ""
echo "  1. Press [2] — hear current frequency"
echo "     (e.g. \"14 point 2 5 0 0 0 megahertz\")"
echo ""
echo "  2. Press [0] — hear current mode"
echo "     (e.g. \"USB\", \"LSB\", \"CW\")"
echo ""
echo "  3. Press [1] — hear \"VFO A\" then frequency"
echo "     Hold  [1] — hear \"VFO B\" then frequency"
echo ""
echo "  4. Press [4] — hear \"Pre amp off\" (or \"Pre amp 1\")"
echo "     Hold  [4] — hear \"A G C\" + level (e.g. \"A G C medium\")"
echo "     Press [/] (was [A]) — hear \"Shift\""
echo "     Then press [4] — hear \"Attenuation off\" (or \"Attenuation N D B\")"
echo ""
echo "  5. Press [7] — hear \"Noise blanker on/off, level N\""
echo ""
echo "  6. Press [8] — hear \"Noise reduction on/off, level N\""
echo "     Hold  [8] — hear \"Mic gain N percent\""
echo ""
echo "  7. Hold  [9] — hear \"Power N percent\""
echo "     [/] then [9] — hear \"Compression on/off, level N\""
echo ""
echo "  8. Press [.] — hear S-meter reading"
echo "     Hold  [.] — hear power meter reading"
echo ""
echo -e "${CYAN}--- PART 2: Announcements Toggle ---${NC}"
echo ""
echo "  9. Press [-] (was [C]) — hear \"Announcements off\""
echo "     Turn VFO dial — should hear NO frequency announcement"
echo "     Press [-] again — hear \"Announcements on\""
echo "     Turn VFO dial — frequency IS announced again"
echo ""
echo -e "${CYAN}--- PART 3: Speech Interrupt ---${NC}"
echo ""
echo " 10. Press [2] to start frequency readout, then"
echo "     press [4] WHILE it is speaking"
echo "     - Frequency should STOP, then hear preamp status instead"
echo "     NOTE: If [4] is pressed BEFORE speech starts, it will queue —"
echo "     you'll hear the full frequency, THEN preamp status. This is"
echo "     expected for now (interrupt only works mid-speech)."
echo ""
echo -e "${CYAN}--- PART 4: Set Mode (Power Level) ---${NC}"
echo ""
echo " 11. Press [*] (was [B]) — hear \"Set\""
echo "     Hold  [9] — hear current power (e.g. \"Power 45 percent\")"
echo "     Type 5, 5 then press [Enter] — hear \"Power set to 55\""
echo "       then \"Set Off\""
echo "     Hold  [9] (from normal mode) — confirm it says \"Power 55 percent\""
echo ""
echo " 12. Press Ctrl+C to exit when done"
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
echo "  1.  Did you hear \"Ready\" + frequency on startup? (Y/N)"
echo "  2.  Did [2] announce the current frequency? (Y/N)"
echo "  3.  Did [0] announce the mode (USB, LSB, etc.)? (Y/N)"
echo "  4.  Did [1] select VFO A, hold [1] select VFO B? (Y/N)"
echo "  5.  Did [4] announce preamp, hold AGC, shift attenuation? (Y/N)"
echo "  6.  Did [7] announce noise blanker status? (Y/N)"
echo "  7.  Did [8] announce noise reduction, hold mic gain? (Y/N)"
echo "  8.  Did hold [9] announce power, shift [9] compression? (Y/N)"
echo "  9.  Did [.] announce S-meter, hold [.] power meter? (Y/N)"
echo " 10.  Did [-] toggle announcements on/off correctly? (Y/N)"
echo " 11.  Did pressing a key mid-speech interrupt it? (Y/N)"
echo " 12.  Did set mode change power and announce correctly? (Y/N)"
echo ""
echo "If all answers are YES, the test PASSED."
echo "If any answer is NO, investigate the issue."
echo ""
echo "Logs: /tmp/firmware.log, /tmp/hampod_debug.log, /tmp/hampod_output.txt"
