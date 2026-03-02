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
    echo "  clear-cache"
    echo "      Clears the TTS audio cache directory."
    echo ""
    echo "  reset"
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

function cmd_clear_cache() {
    print_header "HAMPOD CLI - Clear Cache"
    local cache_dir="$SOFTWARE2_DIR/audio_cache"
    
    if [ -d "$cache_dir" ]; then
        echo "Clearing TTS cache at: $cache_dir"
        rm -rf "$cache_dir"/* 2>/dev/null || true
        print_success "TTS cache cleared."
    else
        echo "Cache directory does not exist or is already empty."
    fi
}

function cmd_reset() {
    print_header "HAMPOD CLI - Hard Reset"
    echo -e "${YELLOW}Warning: This will forcefully stop HAMPOD and clear all temporary system state.${NC}"
    echo ""
    
    echo "Stopping processes..."
    sudo killall -9 firmware.elf 2>/dev/null || true
    sudo killall -9 hampod 2>/dev/null || true
    sudo killall -9 phase0_test 2>/dev/null || true
    sudo killall -9 piper 2>/dev/null || true
    sudo killall -9 aplay 2>/dev/null || true
    
    echo "Clearing IPC pipes..."
    sudo rm -f "$FIRMWARE_DIR/Firmware_i" "$FIRMWARE_DIR/Firmware_o" 2>/dev/null || true
    sudo rm -f "$FIRMWARE_DIR/Speaker_i" "$FIRMWARE_DIR/Speaker_o" 2>/dev/null || true
    sudo rm -f "$FIRMWARE_DIR/Keypad_i" "$FIRMWARE_DIR/Keypad_o" 2>/dev/null || true
    
    echo "Clearing temporary logs..."
    sudo rm -f /tmp/firmware.log /tmp/hampod_output.txt /tmp/hampod_debug.log 2>/dev/null || true
    
    print_success "System state reset successfully."
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
    clear-cache)
        cmd_clear_cache
        ;;
    reset)
        cmd_reset
        ;;
    backup-config|restore-config)
        print_error "Command '$COMMAND' is coming in a future update."
        ;;
    *)
        print_error "Unknown command: $COMMAND"
        echo ""
        cmd_help
        exit 1
        ;;
esac
