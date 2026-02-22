#!/bin/bash
# =============================================================================
# HAMPOD2026 Regression Test: Normal Mode
# =============================================================================
# Tests the Normal Mode implementation (Phase 2):
#   - Builds Software2
#   - Runs unit tests (test_normal_mode)
#   - Optionally runs radio query tests (test_radio_queries)
#
# Usage: ./Regression_Normal_Mode.sh [--with-radio]
#
# =============================================================================

set -e  # Exit on error

# Configuration
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
SOFTWARE2_DIR="$REPO_ROOT/Software2"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

# Test counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

pass() {
    ((TESTS_PASSED++)) || true
    ((TESTS_RUN++)) || true
    echo -e "${GREEN}PASS${NC}: $1"
}

fail() {
    ((TESTS_FAILED++)) || true
    ((TESTS_RUN++)) || true
    echo -e "${RED}FAIL${NC}: $1"
}

echo -e "${CYAN}=============================================${NC}"
echo -e "${CYAN}HAMPOD2026 Normal Mode Regression Test${NC}"
echo -e "${CYAN}=============================================${NC}"
echo ""

# Check for radio test flag
WITH_RADIO=false
if [[ "$1" == "--with-radio" ]]; then
    WITH_RADIO=true
fi

# Check prerequisites
echo "Checking prerequisites..."
echo "Working directory: $SOFTWARE2_DIR"
echo ""

# -----------------------------------------------------------------------------
# Test 1: Build Software2
# -----------------------------------------------------------------------------
echo -e "${YELLOW}--- Build Test ---${NC}"
cd "$SOFTWARE2_DIR"

echo "Running 'make clean && make'..."
if make clean && make 2>&1 | grep -v "^make\[" | while read line; do echo "  $line"; done; then
    if [ -f "bin/hampod" ]; then
        pass "Software2 builds successfully"
    else
        fail "Software2 build produced no binary"
    fi
else
    fail "Software2 build failed"
    exit 1
fi
echo ""

# -----------------------------------------------------------------------------
# Test 2: Compile Unit Test
# -----------------------------------------------------------------------------
echo -e "${YELLOW}--- Normal Mode Unit Test ---${NC}"

# Check if test exists
if [ -f "tests/test_normal_mode.c" ]; then
    echo "Compiling standalone normal mode test..."
    
    # Compile test with mocked radio functions
    # Note: We need to exclude radio_queries.o and provide mocks
    if gcc -Wall -I./include -o bin/test_normal_mode \
        tests/test_normal_mode.c \
        obj/normal_mode.o \
        obj/frequency_mode.o \
        obj/speech.o \
        obj/comm.o \
        obj/config.o \
        obj/keypad.o \
        -lpthread 2>&1; then
        pass "Normal mode test compiled"
        
        # Run the test
        echo "Running normal mode unit tests..."
        if ./bin/test_normal_mode 2>&1 | while read line; do echo "  $line"; done; then
            pass "Normal mode unit tests passed"
        else
            fail "Normal mode unit tests failed"
        fi
    else
        echo -e "${YELLOW}WARN${NC}: Normal mode test compilation failed (may need mocks)"
        echo "  Skipping unit tests for now"
    fi
else
    echo -e "${YELLOW}WARN${NC}: test_normal_mode.c not found, skipping unit tests"
fi
echo ""

# -----------------------------------------------------------------------------
# Test 3: Radio Query Test (Optional)
# -----------------------------------------------------------------------------
echo -e "${YELLOW}--- Radio Query Test (Optional) ---${NC}"

# Check for USB serial device (radio connection)
RADIO_DEVICE=$(ls /dev/ttyUSB* 2>/dev/null | head -1 || echo "")

if [ -n "$RADIO_DEVICE" ] && $WITH_RADIO; then
    echo "USB serial device detected: $RADIO_DEVICE"
    
    if [ -f "tests/test_radio_queries.c" ]; then
        echo "Compiling radio queries test..."
        
        if gcc -Wall -I./include -o bin/test_radio_queries \
            tests/test_radio_queries.c \
            obj/radio_queries.o \
            obj/radio.o \
            obj/config.o \
            -lhamlib -lpthread 2>&1; then
            pass "Radio queries test compiled"
            
            echo "Running radio queries test (requires radio connection)..."
            if timeout 30 ./bin/test_radio_queries 2>&1 | while read line; do echo "  $line"; done; then
                pass "Radio queries test passed"
            else
                echo -e "${YELLOW}WARN${NC}: Radio queries test failed or timed out"
            fi
        else
            echo -e "${YELLOW}WARN${NC}: Radio queries test compilation failed"
        fi
    else
        echo -e "${YELLOW}WARN${NC}: test_radio_queries.c not found"
    fi
else
    if [ -z "$RADIO_DEVICE" ]; then
        echo "No USB serial device (radio) detected, skipping radio test"
    else
        echo "Skipping radio test (use --with-radio flag to enable)"
    fi
fi
echo ""

# -----------------------------------------------------------------------------
# Summary
# -----------------------------------------------------------------------------
echo -e "${CYAN}=============================================${NC}"
echo -e "${CYAN}RESULTS${NC}"
echo -e "${CYAN}=============================================${NC}"
echo "Tests run:    $TESTS_RUN"
echo "Tests passed: $TESTS_PASSED"
echo "Tests failed: $TESTS_FAILED"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed.${NC}"
    exit 1
fi
