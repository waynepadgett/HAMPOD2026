#!/bin/bash
# ============================================================================
# Regression_Frequency_Mode.sh
# HAMPOD2026 Frequency Mode Regression Test
#
# Tests the frequency mode implementation:
# 1. Unit tests for state machine
# 2. Radio module tests (if radio connected)
#
# Run on Raspberry Pi after building Software2
# ============================================================================

set -e

echo "============================================="
echo "HAMPOD2026 Frequency Mode Regression Test"
echo "============================================="
echo ""

# Configuration
PROJECT_DIR="$HOME/HAMPOD2026"
SOFTWARE2_DIR="$PROJECT_DIR/Software2"
BIN_DIR="$SOFTWARE2_DIR/bin"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Result tracking
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

pass() {
    echo -e "${GREEN}PASS${NC}: $1"
    ((TESTS_PASSED++)) || true
    ((TESTS_RUN++)) || true
}

fail() {
    echo -e "${RED}FAIL${NC}: $1"
    ((TESTS_FAILED++)) || true
    ((TESTS_RUN++)) || true
}

warn() {
    echo -e "${YELLOW}WARN${NC}: $1"
}

# ============================================================================
# Prerequisites
# ============================================================================

echo "Checking prerequisites..."

if [ ! -d "$SOFTWARE2_DIR" ]; then
    echo "ERROR: Software2 directory not found at $SOFTWARE2_DIR"
    exit 1
fi

cd "$SOFTWARE2_DIR"
echo "Working directory: $(pwd)"
echo ""

# ============================================================================
# Build Test
# ============================================================================

echo "--- Build Test ---"
echo "Running 'make clean && make'..."

if make clean && make; then
    pass "Software2 builds successfully"
else
    fail "Software2 build failed"
    exit 1
fi
echo ""

# ============================================================================
# Unit Test - Frequency Mode State Machine
# ============================================================================

echo "--- Frequency Mode Unit Test ---"

# The test_frequency_mode uses mocks, so we need to compile it standalone
# with the frequency_mode.c but mock versions of other modules

# For now, we'll compile a minimal test that just tests the state machine
TEST_FREQ_SRC="$SOFTWARE2_DIR/tests/test_frequency_mode.c"
TEST_FREQ_BIN="$BIN_DIR/test_frequency_mode_standalone"

echo "Compiling standalone frequency mode test..."
# Compile with frequency_mode.c but use mocks from test file
gcc -Wall -I./include \
    -o "$TEST_FREQ_BIN" \
    "$TEST_FREQ_SRC" \
    src/frequency_mode.c \
    2>&1

if [ $? -eq 0 ]; then
    pass "Frequency mode test compiled"
else
    fail "Frequency mode test compilation failed"
    exit 1
fi

echo "Running frequency mode unit tests..."
if "$TEST_FREQ_BIN"; then
    pass "Frequency mode unit tests passed"
else
    fail "Frequency mode unit tests failed"
    exit 1
fi
echo ""

# ============================================================================
# Radio Test (Optional - requires hardware)
# ============================================================================

echo "--- Radio Module Test (Optional) ---"

# Check if radio is connected by looking for USB device
if ls /dev/ttyUSB* 2>/dev/null | head -1 > /dev/null; then
    echo "USB serial device detected, attempting radio test..."
    
    # Build and run radio test
    if [ -f "$BIN_DIR/test_radio" ]; then
        if timeout 30 "$BIN_DIR/test_radio" 2>&1; then
            pass "Radio module test passed"
        else
            warn "Radio test failed (radio may not be connected)"
        fi
    else
        warn "test_radio binary not found, skipping"
    fi
else
    warn "No USB serial device found, skipping radio test"
fi
echo ""

# ============================================================================
# Summary
# ============================================================================

echo "============================================="
echo "RESULTS"
echo "============================================="
echo "Tests run:    $TESTS_RUN"
echo "Tests passed: $TESTS_PASSED"
echo "Tests failed: $TESTS_FAILED"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
fi
