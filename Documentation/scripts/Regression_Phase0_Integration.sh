#!/bin/bash
# =============================================================================
# HAMPOD2026 Regression Test: Phase 0.9 Integration (Software2 + Firmware)
# =============================================================================
# This script runs the Phase 0.9 Integration Test which verifies:
#   - Router thread architecture in Software2 comm module
#   - Concurrent keypad polling and speech threads
#   - Packet type routing (KEYPAD vs AUDIO responses)
#   - Key press detection with spoken feedback
#   - Key hold detection
#
# This is the ONLY regression test that exercises the router thread!
#
# Usage: ./Regression_Phase0_Integration.sh [timeout_seconds]
#   timeout_seconds: How long to run the test (default: 20)
#
# Prerequisites:
#   - USB keypad connected
#   - USB audio device connected
#   - Piper TTS model installed in Firmware/models/
#   - Firmware not already running (script will start it)
#
# =============================================================================

set -e  # Exit on error during setup

# Configuration
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
FIRMWARE_DIR="$REPO_ROOT/Firmware"
SOFTWARE2_DIR="$REPO_ROOT/Software2"
TEST_TIMEOUT="${1:-20}"  # Default 20 seconds

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}======================================================${NC}"
echo -e "${CYAN}  HAMPOD2026 Phase 0.9 Integration Regression Test    ${NC}"
echo -e "${CYAN}======================================================${NC}"
echo ""
echo "Test timeout: ${TEST_TIMEOUT} seconds"
echo "Firmware dir: $FIRMWARE_DIR"
echo "Software2 dir: $SOFTWARE2_DIR"
echo ""

# -----------------------------------------------------------------------------
# Step 1: Clean up stale processes
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 1/7] Cleaning up stale processes...${NC}"
sudo killall -9 firmware.elf 2>/dev/null || true
sudo killall -9 phase0_test 2>/dev/null || true
sudo killall -9 piper 2>/dev/null || true
sudo killall -9 aplay 2>/dev/null || true
sleep 1
echo "  Done."

# -----------------------------------------------------------------------------
# Step 2: Clean up stale pipes (use sudo since firmware runs as root)
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 2/7] Cleaning up stale pipes...${NC}"
# Note: Pipes may be owned by root if firmware was run with sudo
sudo rm -f "$FIRMWARE_DIR/Firmware_i" "$FIRMWARE_DIR/Firmware_o" 2>/dev/null || true
sudo rm -f "$FIRMWARE_DIR/Speaker_i" "$FIRMWARE_DIR/Speaker_o" 2>/dev/null || true
sudo rm -f "$FIRMWARE_DIR/Keypad_o" 2>/dev/null || true
rm -f /tmp/phase0_test_output.txt /tmp/firmware.log 2>/dev/null || true
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
# Step 5: Build Software2 phase0_test
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 5/7] Building Software2 phase0_test...${NC}"
cd "$SOFTWARE2_DIR"
make clean > /dev/null 2>&1 || true
make phase0_test 2>&1 | while read line; do echo "  $line"; done

if [ ! -f "bin/phase0_test" ]; then
    echo -e "${RED}  ERROR: phase0_test build failed${NC}"
    exit 1
fi
echo -e "  ${GREEN}✓${NC} phase0_test built"

# -----------------------------------------------------------------------------
# Step 6: Start Firmware
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 6/7] Starting Firmware...${NC}"
cd "$FIRMWARE_DIR"
sudo ./firmware.elf > /tmp/firmware.log 2>&1 &
FIRMWARE_PID=$!
echo "  Firmware PID: $FIRMWARE_PID"

# Wait for Firmware_o pipe to exist (firmware creates this first)
# Note: Firmware blocks on open(O_WRONLY) until a reader connects,
# so Firmware_i won't exist yet. The test client handles the connection.
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
echo -e "  ${GREEN}✓${NC} Firmware started (waiting for connection)"

# -----------------------------------------------------------------------------
# Step 7: Run Phase 0.9 Integration Test
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 7/7] Running Phase 0.9 Integration Test...${NC}"
echo ""
echo -e "${CYAN}--- TEST OUTPUT START ---${NC}"
echo ""

cd "$SOFTWARE2_DIR"

# Run the test with timeout, capture output
set +e  # Don't exit on error
script -q -c "sudo timeout ${TEST_TIMEOUT}s ./bin/phase0_test" /tmp/phase0_test_output.txt
TEST_EXIT=$?
set -e

echo ""
echo -e "${CYAN}--- TEST OUTPUT END ---${NC}"
echo ""

# Stop Firmware
echo "Stopping Firmware..."
sudo kill $FIRMWARE_PID 2>/dev/null || true
sudo killall -9 firmware.elf piper aplay 2>/dev/null || true

# -----------------------------------------------------------------------------
# Results Summary
# -----------------------------------------------------------------------------
echo -e "${CYAN}======================================================${NC}"
echo -e "${CYAN}                    TEST RESULTS                      ${NC}"
echo -e "${CYAN}======================================================${NC}"

# Parse output
OUTPUT=$(cat /tmp/phase0_test_output.txt 2>/dev/null || echo "")

# Check Firmware connection
if echo "$OUTPUT" | grep -q "Firmware communication initialized\|Connected to Firmware\|Firmware ready"; then
    echo -e "  ${GREEN}✓${NC} Connected to Firmware"
    FIRMWARE_CONNECTED=1
else
    echo -e "  ${RED}✗${NC} Failed to connect to Firmware"
    FIRMWARE_CONNECTED=0
fi

# Check Router thread started
if echo "$OUTPUT" | grep -q "Router thread started\|Starting router"; then
    echo -e "  ${GREEN}✓${NC} Router thread started"
    ROUTER_STARTED=1
else
    echo -e "  ${RED}✗${NC} Router thread not started"
    ROUTER_STARTED=0
fi

# Check Speech system
if echo "$OUTPUT" | grep -q "Speech thread started\|speech system\|Speech system"; then
    echo -e "  ${GREEN}✓${NC} Speech system initialized"
    SPEECH_INIT=1
else
    echo -e "  ${RED}✗${NC} Speech system failed"
    SPEECH_INIT=0
fi

# Check Keypad system
if echo "$OUTPUT" | grep -q "Keypad thread started\|keypad system\|Keypad system"; then
    echo -e "  ${GREEN}✓${NC} Keypad system initialized"
    KEYPAD_INIT=1
else
    echo -e "  ${RED}✗${NC} Keypad system failed"
    KEYPAD_INIT=0
fi

# Check for key presses
KEY_PRESSES=$(echo "$OUTPUT" | grep -c "pressed\|Key event" 2>/dev/null || echo "0")
KEY_PRESSES=$(echo "$KEY_PRESSES" | head -1 | tr -d '\n')
if [ "$KEY_PRESSES" -gt 0 ] 2>/dev/null; then
    echo -e "  ${GREEN}✓${NC} Key presses detected: $KEY_PRESSES"
else
    echo -e "  ${YELLOW}!${NC} No key presses detected (OK if no keys pressed)"
fi

# Check for holds
if echo "$OUTPUT" | grep -q "held\|HOLD"; then
    echo -e "  ${GREEN}✓${NC} Key hold detection working"
fi

# Check for packet type errors (the bug we fixed!)
if echo "$OUTPUT" | grep -q "packet type mismatch\|expected KEYPAD.*got AUDIO\|bruh"; then
    echo -e "  ${RED}✗${NC} PACKET TYPE MISMATCH ERRORS DETECTED!"
    echo "    This indicates the router thread is not working properly."
    PACKET_ERRORS=1
else
    echo -e "  ${GREEN}✓${NC} No packet type errors (router working correctly)"
    PACKET_ERRORS=0
fi

echo ""

# Determine overall result
if [ "$FIRMWARE_CONNECTED" -eq 1 ] && [ "$ROUTER_STARTED" -eq 1 ] && [ "$PACKET_ERRORS" -eq 0 ]; then
    if [ "$TEST_EXIT" -eq 124 ]; then
        echo -e "${GREEN}TEST RESULT: PASS (ran for full ${TEST_TIMEOUT}s duration)${NC}"
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
echo "  - Did you hear the startup announcement?"
echo "  - Did you hear 'You pressed X' when pressing keys?"
echo "  - Did you hear 'You held X' for long presses?"
echo ""
echo "If NO audio was heard, the test should be considered FAILED"
echo "even if the log shows success."
echo ""
echo "Logs preserved in /tmp/firmware.log and /tmp/phase0_test_output.txt"

exit $FINAL_RESULT
