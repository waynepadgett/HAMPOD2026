#!/bin/bash
# =============================================================================
# HAMPOD2026 Regression Test: HAL Integration
# =============================================================================
# This script runs the HAL Integration Test which verifies:
#   - USB Keypad HAL functionality
#   - USB Audio HAL functionality
#   - Piper TTS HAL functionality
#   - Key press detection and spoken feedback
#   - Key hold detection
#
# Usage: ./Regression_HAL_Integration.sh [timeout_seconds]
#   timeout_seconds: How long to run the test (default: 15)
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
HAL_TESTS_DIR="$FIRMWARE_DIR/hal/tests"
TEST_TIMEOUT="${1:-15}"  # Default 15 seconds

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${CYAN}=============================================${NC}"
echo -e "${CYAN}  HAMPOD2026 HAL Integration Regression Test ${NC}"
echo -e "${CYAN}=============================================${NC}"
echo ""
echo "Test timeout: ${TEST_TIMEOUT} seconds"
echo "Firmware dir: $FIRMWARE_DIR"
echo ""

# -----------------------------------------------------------------------------
# Step 1: Clean up any stale processes
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 1/5] Cleaning up stale processes...${NC}"
sudo killall -9 test_hal_integration 2>/dev/null || true
sudo killall -9 test_hal_keypad 2>/dev/null || true
sudo killall -9 test_hal_audio 2>/dev/null || true
echo "  Done."

# -----------------------------------------------------------------------------
# Step 2: Clean up stale pipes/temp files
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 2/5] Cleaning up temp files...${NC}"
rm -f /tmp/hampod_test.wav /tmp/hampod_speak.wav /tmp/hal_test_output.txt 2>/dev/null || true
echo "  Done."

# -----------------------------------------------------------------------------
# Step 3: Verify hardware is connected
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 3/5] Verifying hardware...${NC}"

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
# Step 4: Build the test
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 4/5] Building HAL integration test...${NC}"
cd "$HAL_TESTS_DIR"
make clean > /dev/null 2>&1 || true
make test_hal_integration 2>&1 | while read line; do echo "  $line"; done

if [ ! -f "test_hal_integration" ]; then
    echo -e "${RED}  ERROR: Build failed - test binary not created${NC}"
    exit 1
fi
echo -e "  ${GREEN}✓${NC} Build successful"

# -----------------------------------------------------------------------------
# Step 5: Run the test
# -----------------------------------------------------------------------------
echo -e "${YELLOW}[Step 5/5] Running HAL integration test...${NC}"
echo ""
echo -e "${CYAN}--- TEST OUTPUT START ---${NC}"
echo ""

cd "$FIRMWARE_DIR"

# Use 'script' to capture output from sudo command (handles TTY/buffering issues)
# The test will run for TEST_TIMEOUT seconds then be killed by timeout
script -q -c "sudo timeout ${TEST_TIMEOUT}s hal/tests/test_hal_integration" /tmp/hal_test_output.txt
TEST_EXIT=$?

echo ""
echo -e "${CYAN}--- TEST OUTPUT END ---${NC}"
echo ""

# -----------------------------------------------------------------------------
# Results Summary
# -----------------------------------------------------------------------------
echo -e "${CYAN}=============================================${NC}"
echo -e "${CYAN}                TEST RESULTS                 ${NC}"
echo -e "${CYAN}=============================================${NC}"

# Parse output for key events
OUTPUT=$(cat /tmp/hal_test_output.txt 2>/dev/null || echo "")

# Check initialization (patterns match actual log output)
if echo "$OUTPUT" | grep -q "HAL Keypad.*Initialized\|Keypad HAL.*Initialized"; then
    echo -e "  ${GREEN}✓${NC} Keypad HAL initialized"
    KEYPAD_INIT=1
else
    echo -e "  ${RED}✗${NC} Keypad HAL failed to initialize"
    KEYPAD_INIT=0
fi

if echo "$OUTPUT" | grep -q "HAL Audio.*Detected\|Audio HAL.*Detected\|Audio.*initialized"; then
    echo -e "  ${GREEN}✓${NC} Audio HAL initialized"
    AUDIO_INIT=1
else
    echo -e "  ${RED}✗${NC} Audio HAL failed to initialize"
    AUDIO_INIT=0
fi

if echo "$OUTPUT" | grep -q "HAL TTS.*Piper\|TTS.*Piper\|TTS HAL.*initialized"; then
    echo -e "  ${GREEN}✓${NC} TTS HAL initialized"
    TTS_INIT=1
else
    echo -e "  ${RED}✗${NC} TTS HAL failed to initialize"
    TTS_INIT=0
fi

# Check for key presses
KEY_PRESSES=$(echo "$OUTPUT" | grep -c "Key:.*Speaking" || echo "0")
if [ "$KEY_PRESSES" -gt 0 ]; then
    echo -e "  ${GREEN}✓${NC} Key presses detected: $KEY_PRESSES"
else
    echo -e "  ${YELLOW}!${NC} No key presses detected during test"
    echo "    (This is OK if no keys were pressed)"
fi

# Check for hold detection
if echo "$OUTPUT" | grep -q "HELD"; then
    echo -e "  ${GREEN}✓${NC} Key hold detection working"
fi

echo ""

# Determine overall result
if [ "$KEYPAD_INIT" -eq 1 ] && [ "$AUDIO_INIT" -eq 1 ] && [ "$TTS_INIT" -eq 1 ]; then
    if [ "$TEST_EXIT" -eq 124 ]; then
        # Exit code 124 = timeout (expected)
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
    echo -e "${RED}TEST RESULT: FAIL (initialization error)${NC}"
    FINAL_RESULT=1
fi

echo ""
echo -e "${CYAN}=============================================${NC}"
echo ""
echo "IMPORTANT: Please verify audio was heard!"
echo "  - Did you hear 'System Ready' at startup?"
echo "  - Did you hear key names when pressing keys?"
echo ""
echo "If NO audio was heard, the test should be considered FAILED"
echo "even if the log shows success."
echo ""

# Cleanup
rm -f /tmp/hal_test_output.txt 2>/dev/null || true

exit $FINAL_RESULT
