#!/bin/bash
# =============================================================================
# HAMPOD2026 - Run All Unit Tests
# =============================================================================
# Runs the 5 active Software2 tests:
#
#   Phase 1: Unit tests (self-contained, no hardware needed)
#     - test_compile          Build smoke test
#     - test_comm_queue       Response queue FIFO, timeout, overflow
#     - test_config           Config load/save, undo, clamping
#     - test_frequency_mode   Frequency mode state machine (mock-based)
#
#   Phase 2: Radio test (needs physical radio connected via USB)
#     - test_radio            Hamlib radio connection (no Firmware needed)
#
# Deprecated tests (in tests/deprecated/):
#   test_comm_read, test_comm_write, test_keypad_events, test_speech_queue
#   These were Phase 0 integration tests that used blocking pipe I/O to
#   communicate with Firmware. They no longer connect to the current
#   Firmware due to pipe deadlock issues. Their functionality is covered
#   by the regression scripts and manual SOP in DO_THIS_BEFORE_MERGE_TO_MAIN.md.
#
# Usage:
#   cd ~/HAMPOD2026/Software2
#   ./run_all_unit_tests.sh              # Run unit tests + prompt for radio
#   ./run_all_unit_tests.sh --all        # Run everything automatically
#   ./run_all_unit_tests.sh --unit-only  # Run only unit tests (Phase 1)
#   ./run_all_unit_tests.sh --no-build   # Skip the build step
#
# =============================================================================

set -euo pipefail

# --- Configuration -----------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SOFTWARE2_DIR="$SCRIPT_DIR"
BIN_DIR="$SOFTWARE2_DIR/bin"

UNIT_ONLY=false
RUN_ALL=false
SKIP_BUILD=false

# Parse arguments
for arg in "$@"; do
    case "$arg" in
        --unit-only)  UNIT_ONLY=true ;;
        --all)        RUN_ALL=true ;;
        --no-build)   SKIP_BUILD=true ;;
        -h|--help)
            head -30 "$0" | tail -25
            exit 0
            ;;
        *)
            echo "Unknown argument: $arg"
            echo "Use --help for usage."
            exit 1
            ;;
    esac
done

# --- Colors ------------------------------------------------------------------

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
DIM='\033[2m'
NC='\033[0m'

# --- Counters ----------------------------------------------------------------

TOTAL=0
PASSED=0
FAILED=0
SKIPPED=0
FAIL_LIST=""

# --- Helpers -----------------------------------------------------------------

run_test() {
    local name="$1"
    local description="$2"
    local timeout="${3:-0}"  # 0 = no timeout

    TOTAL=$((TOTAL + 1))
    printf "  %-28s %s ... " "$name" "$description"

    if [ ! -f "$BIN_DIR/$name" ]; then
        echo -e "${RED}NOT FOUND${NC}"
        FAILED=$((FAILED + 1))
        FAIL_LIST="$FAIL_LIST  - $name (binary not found)\n"
        return
    fi

    local exit_code=0
    local output=""

    if [ "$timeout" -gt 0 ]; then
        output=$(timeout --kill-after=3 "$timeout" "$BIN_DIR/$name" 2>&1) || exit_code=$?
        # 124 = timed out (SIGTERM), 137 = killed (SIGKILL) — expected for timed tests
        if [ "$exit_code" -eq 124 ] || [ "$exit_code" -eq 137 ]; then
            exit_code=0
        fi
    else
        output=$("$BIN_DIR/$name" 2>&1) || exit_code=$?
    fi

    if [ "$exit_code" -eq 0 ]; then
        echo -e "${GREEN}PASS${NC}"
        PASSED=$((PASSED + 1))
    else
        echo -e "${RED}FAIL${NC} (exit code $exit_code)"
        FAILED=$((FAILED + 1))
        FAIL_LIST="$FAIL_LIST  - $name (exit code $exit_code)\n"
        # Show last few lines of output on failure
        echo "$output" | tail -5 | sed 's/^/      /'
    fi
}

skip_test() {
    local name="$1"
    local reason="$2"
    TOTAL=$((TOTAL + 1))
    SKIPPED=$((SKIPPED + 1))
    printf "  %-28s %s ... " "$name" "$reason"
    echo -e "${YELLOW}SKIPPED${NC}"
}

# =============================================================================
# Main
# =============================================================================

echo ""
echo -e "${CYAN}======================================================${NC}"
echo -e "${CYAN}  HAMPOD2026 — Run All Unit Tests                     ${NC}"
echo -e "${CYAN}======================================================${NC}"
echo ""

# --- Build -------------------------------------------------------------------

if [ "$SKIP_BUILD" = true ]; then
    echo -e "${YELLOW}[Build] Skipping (--no-build)${NC}"
else
    echo -e "${YELLOW}[Build] Building Software2 and tests...${NC}"
    cd "$SOFTWARE2_DIR"
    make tests > /tmp/make_tests.log 2>&1 || {
        echo -e "  ${RED}Build failed!${NC}"
        cat /tmp/make_tests.log | tail -20
        exit 1
    }
    echo -e "  ${GREEN}✓${NC} Build succeeded"
fi

echo ""

# =============================================================================
# Phase 1: Unit Tests (self-contained, no hardware needed)
# =============================================================================

echo -e "${BOLD}Phase 1: Unit Tests${NC} (no dependencies)"
echo ""

cd "$SOFTWARE2_DIR"

run_test "test_compile"        "Build smoke test"
run_test "test_comm_queue"     "Response queue logic"
run_test "test_config"         "Config load/save/undo"
run_test "test_frequency_mode" "Frequency mode state machine"

echo ""

# =============================================================================
# Phase 2: Radio Test (needs physical radio, NO firmware needed)
# =============================================================================

echo -e "${BOLD}Phase 2: Radio Test${NC} (needs radio via USB — no Firmware needed)"
echo ""

if [ "$UNIT_ONLY" = true ]; then
    skip_test "test_radio" "(--unit-only)"
else
    if [ "$RUN_ALL" = true ]; then
        RUN_RADIO="y"
    else
        echo -e "  ${DIM}This test connects directly to the radio via Hamlib.${NC}"
        echo -e "  ${DIM}The radio must be powered on and connected via USB.${NC}"
        echo ""
        read -p "  Is the radio powered on and connected? (y/n): " RUN_RADIO
    fi
    echo ""

    if [[ "$RUN_RADIO" =~ ^[Yy] ]]; then
        run_test "test_radio" "Hamlib radio connection" 30
    else
        skip_test "test_radio" "(user skipped — radio not connected)"
    fi
fi

echo ""

# =============================================================================
# Summary
# =============================================================================

echo -e "${CYAN}======================================================${NC}"
echo -e "${CYAN}  Test Summary                                        ${NC}"
echo -e "${CYAN}======================================================${NC}"
echo ""
echo -e "  Total:   ${BOLD}$TOTAL${NC}"
echo -e "  Passed:  ${GREEN}$PASSED${NC}"
echo -e "  Failed:  ${RED}$FAILED${NC}"
echo -e "  Skipped: ${YELLOW}$SKIPPED${NC}"
echo ""

if [ "$FAILED" -gt 0 ]; then
    echo -e "${RED}Failed tests:${NC}"
    echo -e "$FAIL_LIST"
    echo -e "${RED}✗ SOME TESTS FAILED${NC}"
    exit 1
else
    echo -e "${GREEN}✓ ALL TESTS PASSED${NC}"
    exit 0
fi
