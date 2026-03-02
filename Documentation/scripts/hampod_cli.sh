#!/bin/bash
# =============================================================================
# HAMPOD2026 Command Line Interface
# =============================================================================
# A centralized tool to manage the HAMPOD system.
#
# Usage: hampod <command> [options]
# =============================================================================

# Define paths
SCRIPT_DIR="$( cd -P "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
SOFTWARE2_DIR="$REPO_ROOT/Software2"
FIRMWARE_DIR="$REPO_ROOT/Firmware"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

# --- Output Helpers ---
function print_header() {
    echo -e "${CYAN}======================================================${NC}"
    echo -e "${CYAN}  $1${NC}"
    echo -e "${CYAN}======================================================${NC}"
    echo ""
}

function print_error() {
    echo -e "${RED}ERROR: $1${NC}"
}

function print_success() {
    echo -e "${GREEN}SUCCESS: $1${NC}"
}

# --- Command Handlers ---

function cmd_help() {
    print_header "HAMPOD CLI - Help"
    echo "Usage: hampod <command> [options]"
    echo ""
    echo "Commands:"
    echo "  start [--no-build] [--debug]"
    echo "      Starts the HAMPOD system cleanly."
    echo "      --no-build: Skips re-compiling the C code."
    echo "      --debug:    Enables verbose Hamlib debug logging."
    echo ""
    echo "  help"
    echo "      Displays this help menu."
    echo ""
    echo "  clear-cache (Coming Soon)"
    echo "      Clears the TTS audio cache directory."
    echo ""
    echo "  reset (Coming Soon)"
    echo "      Performs a hard reset of system state, clearing logs and stale processes."
    echo ""
    echo "  backup-config (Coming Soon)"
    echo "      Backs up the current hampod.conf file."
    echo ""
    echo "  restore-config (Coming Soon)"
    echo "      Restores from an existing config backup."
    echo ""
    echo "--- Useful Scripts ---"
    echo "To run regression tests, run them directly from Documentation/scripts:"
    echo "  - Regression_Phase0_Integration.sh"
    echo "  - Regression_Phase_One_Manual_Radio_Test.sh"
    echo "  - Regression_Phase_Two_Manual_Test.sh"
    echo "  - Regression_Phase_Three_Manual_Test.sh"
    echo ""
    echo "To run the memory monitor, use:"
    echo "  - monitor_mem.sh"
    echo ""
    echo "For full installation flags, see the install_hampod.sh script header."
    echo -e "${CYAN}======================================================${NC}"
}

function cmd_start() {
    # We delegate 'start' to the existing robust run_hampod.sh script
    local run_script="$SCRIPT_DIR/run_hampod.sh"
    
    if [ ! -x "$run_script" ]; then
        print_error "Cannot find or execute $run_script."
        exit 1
    fi
    
    # Pass all arguments passed to `hampod start` directly to `run_hampod.sh`
    "$run_script" "$@"
}

# --- Main Entry Point ---

if [ $# -eq 0 ]; then
    cmd_help
    exit 0
fi

COMMAND="$1"
shift # Remove the command from the arguments list, leaving only options

case "$COMMAND" in
    start)
        cmd_start "$@"
        ;;
    help)
        cmd_help
        ;;
    clear-cache|reset|backup-config|restore-config)
        print_error "Command '$COMMAND' is coming in a future update."
        ;;
    *)
        print_error "Unknown command: $COMMAND"
        echo ""
        cmd_help
        exit 1
        ;;
esac
