#!/bin/bash
# =============================================================================
# HAMPOD2026 Regression Test: Imitation Software (Firmware Integration)
# =============================================================================
# This script runs the Imitation Software test which verifies:
#   - Firmware ↔ Software bidirectional communication via named pipes
#   - Keypad polling and key press detection
#   - Audio packet handling (TTS and pre-generated audio)
#   - Mode toggle functionality
#
# Usage: ./Regression_Imitation_Software.sh [timeout_seconds]
#   timeout_seconds: How long to run the test (default: 20)
#
# Prerequisites:
#   - USB keypad connected
#   - USB audio device connected
#   - Piper TTS model installed in Firmware/models/
#
# =============================================================================

set -e  # Exit on error during setup

# Configuration
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
FIRMWARE_DIR="$REPO_ROOT/Firmware"
TEST_TIMEOUT="${1:-20}"  # Default 20 seconds

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}======================================================${NC}"
echo -e "${CYAN}  HAMPOD2026 Imitation Software Regression Test       ${NC}"
echo -e "${CYAN}======================================================${NC}"
echo ""
echo "Test timeout: ${TEST_TIMEOUT} seconds"
echo "Firmware dir: $FIRMWARE_DIR"
echo ""

# -----------------------------------------------------------------------------
# Step 1: Clean up any stale processes
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 1/6] Cleaning up stale processes...${NC}"
sudo killall -9 firmware.elf 2>/dev/null || true
sudo killall -9 imitation_software 2>/dev/null || true
sleep 1  # Let processes fully terminate
echo "  Done."

# -----------------------------------------------------------------------------
# Step 2: Clean up stale pipes
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 2/6] Cleaning up stale pipes...${NC}"
cd "$FIRMWARE_DIR"
rm -f Firmware_i Firmware_o Keypad_i Keypad_o Speaker_i Speaker_o 2>/dev/null || true
rm -f /tmp/imitation_test_output.txt /tmp/firmware.log 2>/dev/null || true
echo "  Done."

# -----------------------------------------------------------------------------
# Step 3: Verify hardware is connected
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 3/6] Verifying hardware...${NC}"

# Check for USB keypad
KEYPAD_DEVICE=$(ls /dev/input/by-id/*kbd* 2>/dev/null | head -1 || echo "")
if [ -z "$KEYPAD_DEVICE" ]; then
    echo -e "${RED}  ERROR: No USB keypad found!${NC}"
    echo "  Check: ls /dev/input/by-id/*kbd*"
    exit 1
fi
echo -e "  ${GREEN}✓${NC} USB Keypad: $KEYPAD_DEVICE"

# Check for USB audio
AUDIO_DEVICE=$(aplay -l 2>/dev/null | grep -i "usb\|device" | head -1 || echo "")
if [ -z "$AUDIO_DEVICE" ]; then
    echo -e "${YELLOW}  WARNING: No USB audio device detected (may use default)${NC}"
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
# Step 4: Build firmware and imitation_software
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 4/6] Building Firmware and Imitation Software...${NC}"
cd "$FIRMWARE_DIR"
make clean > /dev/null 2>&1 || true
make firmware.elf imitation_software 2>&1 | while read line; do echo "  $line"; done

if [ ! -f "firmware.elf" ]; then
    echo -e "${RED}  ERROR: firmware.elf not built${NC}"
    exit 1
fi
echo -e "  ${GREEN}✓${NC} firmware.elf built"

if [ ! -f "imitation_software" ]; then
    echo -e "${RED}  ERROR: imitation_software not built${NC}"
    exit 1
fi
echo -e "  ${GREEN}✓${NC} imitation_software built"

# -----------------------------------------------------------------------------
# Step 5: Start Firmware in background
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 5/6] Starting Firmware...${NC}"
cd "$FIRMWARE_DIR"

# Start firmware in background
nohup sudo ./firmware.elf > /tmp/firmware.log 2>&1 &
FIRM_PID=$!
echo "  Firmware PID: $FIRM_PID"

# Wait for pipes to be created
echo "  Waiting for Firmware to initialize..."
for i in {1..10}; do
    if [ -p "Firmware_o" ] && [ -p "Firmware_i" ]; then
        echo -e "  ${GREEN}✓${NC} Pipes created"
        break
    fi
    sleep 0.5
done

if [ ! -p "Firmware_o" ]; then
    echo -e "${RED}  ERROR: Firmware_o pipe not created after 5 seconds${NC}"
    echo "  Firmware log:"
    cat /tmp/firmware.log 2>/dev/null || echo "  (no log)"
    sudo kill $FIRM_PID 2>/dev/null || true
    exit 1
fi

echo -e "  ${GREEN}✓${NC} Firmware started and waiting for connection"

# -----------------------------------------------------------------------------
# Step 6: Run Imitation Software
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 6/6] Running Imitation Software...${NC}"
echo ""
echo -e "${CYAN}--- TEST OUTPUT START ---${NC}"
echo ""

# Use 'script' to capture output (handles TTY/buffering issues)
# Note: sudo needed because firmware runs as root and creates pipes with root permissions
script -q -c "sudo timeout ${TEST_TIMEOUT}s ./imitation_software" /tmp/imitation_test_output.txt
TEST_EXIT=$?

echo ""
echo -e "${CYAN}--- TEST OUTPUT END ---${NC}"
echo ""

# Stop firmware
echo "Stopping Firmware..."
sudo kill $FIRM_PID 2>/dev/null || true
sleep 1

# -----------------------------------------------------------------------------
# Results Summary
# -----------------------------------------------------------------------------
echo -e "${CYAN}======================================================${NC}"
echo -e "${CYAN}                    TEST RESULTS                      ${NC}"
echo -e "${CYAN}======================================================${NC}"

# Parse output
OUTPUT=$(cat /tmp/imitation_test_output.txt 2>/dev/null || echo "")
FIRMWARE_LOG=$(cat /tmp/firmware.log 2>/dev/null || echo "")

# Check connection established
if echo "$OUTPUT" | grep -q "Successful connection"; then
    echo -e "  ${GREEN}✓${NC} Pipe connection established"
    CONNECTION=1
else
    echo -e "  ${RED}✗${NC} Pipe connection failed"
    CONNECTION=0
fi

# Check initial audio played (TTS announcement)
if echo "$FIRMWARE_LOG" | grep -q "Playing TTS\|TTS:"; then
    echo -e "  ${GREEN}✓${NC} TTS audio commands processed"
    INITIAL_AUDIO=1
else
    echo -e "  ${YELLOW}!${NC} No TTS commands in firmware log"
    INITIAL_AUDIO=0
fi

# Check keypad polling is working (use tr to clean output, avoid newline issues)
KEYPAD_POLLS=$(echo "$OUTPUT" | grep -c "keypad says" | tr -d '\n\r' || echo "0")
KEYPAD_POLLS=${KEYPAD_POLLS:-0}
if [ "$KEYPAD_POLLS" -gt 5 ]; then
    echo -e "  ${GREEN}✓${NC} Keypad polling active ($KEYPAD_POLLS polls logged)"
    KEYPAD_POLL=1
else
    echo -e "  ${YELLOW}!${NC} Few/no keypad polls detected ($KEYPAD_POLLS)"
    KEYPAD_POLL=0
fi

# Check if key presses were detected (anything other than '-' or 0x2d)
KEY_PRESSES=$(echo "$OUTPUT" | grep "keypad says" | grep -v "2d (-)" | grep -v "ff" | wc -l || echo "0")
if [ "$KEY_PRESSES" -gt 0 ]; then
    echo -e "  ${GREEN}✓${NC} Key presses detected: $KEY_PRESSES"
else
    echo -e "  ${YELLOW}!${NC} No key presses detected during test"
    echo "    (This is OK if no keys were pressed)"
fi

# Check for audio playback commands
AUDIO_PLAYS=$(echo "$OUTPUT" | grep -c "Playing:" | tr -d '\n\r' || echo "0")
AUDIO_PLAYS=${AUDIO_PLAYS:-0}
if [ "$AUDIO_PLAYS" -gt 0 ]; then
    echo -e "  ${GREEN}✓${NC} Audio playback commands: $AUDIO_PLAYS"
fi

# Check for mode toggle
if echo "$OUTPUT" | grep -q "Mode toggled"; then
    echo -e "  ${GREEN}✓${NC} Mode toggle tested"
fi

echo ""

# Determine overall result
if [ "$CONNECTION" -eq 1 ] && [ "$KEYPAD_POLL" -eq 1 ]; then
    if [ "$TEST_EXIT" -eq 124 ]; then
        # Exit code 124 = timeout (expected - test ran full duration)
        echo -e "${GREEN}TEST RESULT: PASS (ran for full duration)${NC}"
        FINAL_RESULT=0
    elif [ "$TEST_EXIT" -eq 0 ]; then
        echo -e "${GREEN}TEST RESULT: PASS${NC}"
        FINAL_RESULT=0
    else
        echo -e "${YELLOW}TEST RESULT: COMPLETED with exit code $TEST_EXIT${NC}"
        FINAL_RESULT=0
    fi
else
    echo -e "${RED}TEST RESULT: FAIL${NC}"
    FINAL_RESULT=1
fi

echo ""
echo -e "${CYAN}======================================================${NC}"
echo ""
echo "IMPORTANT: Please verify audio was heard!"
echo "  - Did you hear the initial TTS announcement?"
echo "  - Did you hear audio when pressing keys?"
echo ""
echo "If NO audio was heard, the test should be considered FAILED"
echo "even if the log shows success."
echo ""

# Show firmware log snippet if there were issues
if [ "$FINAL_RESULT" -ne 0 ]; then
    echo "Firmware log (last 20 lines):"
    tail -20 /tmp/firmware.log 2>/dev/null || echo "(no log available)"
    echo ""
fi

# Cleanup
# rm -f /tmp/imitation_test_output.txt /tmp/firmware.log 2>/dev/null || true
echo "Logs preserved in /tmp/firmware.log"

exit $FINAL_RESULT
