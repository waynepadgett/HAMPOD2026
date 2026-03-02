#!/bin/bash
# =============================================================================
# HAMPOD2026 Command Line Interface
# =============================================================================
# A centralized tool to manage the HAMPOD system.
#
# Usage: hampod <command> [options]
# =============================================================================

# Resolve the real path if the script is called via a symlink (e.g., from /usr/local/bin/hampod)
SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"
  SOURCE="$(readlink "$SOURCE")"
  # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
done
SCRIPT_DIR="$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )"

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
    echo "  backup-config"
    echo "      Backs up the current hampod.conf file with an interactive prompt."
    echo ""
    echo "  restore-config"
    echo "      Restores from an existing config backup interactively."
    echo ""
    echo "  monitor_mem"
    echo "      Runs the memory monitoring script."
    echo ""
    echo "--- Useful Scripts ---"
    echo "To run regression tests, run them directly from Documentation/scripts:"
    echo "  - Regression_Phase0_Integration.sh"
    echo "  - Regression_Phase_One_Manual_Radio_Test.sh"
    echo "  - Regression_Phase_Two_Manual_Test.sh"
    echo "  - Regression_Phase_Three_Manual_Test.sh"
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

function cmd_monitor_mem() {
    local run_script="$SCRIPT_DIR/monitor_mem.sh"
    
    if [ ! -x "$run_script" ]; then
        print_error "Cannot find or execute $run_script."
        exit 1
    fi
    
    "$run_script" "$@"
}

function cmd_clear_cache() {
    print_header "HAMPOD CLI - Clear Cache"
    # The Firmware TTS cache is stored in the HOME of the user running firmware.elf.
    # Since run_hampod.sh uses sudo, this is located at /root/.cache/hampod/tts
    local cache_dir="/root/.cache/hampod/tts"
    
    echo "Clearing TTS cache at: $cache_dir"
    # We use sudo because the directory is owned by root
    if sudo rm -rf "$cache_dir"/* 2>/dev/null; then
        print_success "TTS cache cleared."
    else
        print_error "Failed to clear TTS cache or it is already empty."
    fi
}

function cmd_reset() {
    print_header "HAMPOD CLI - Hard Reset"
    echo -e "${YELLOW}Warning: This will forcefully stop HAMPOD and clear all temporary system state, including configuration.${NC}"
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
    
    echo "Resetting configuration..."
    local config_file="$SOFTWARE2_DIR/config/hampod.conf"
    local factory_config="$SOFTWARE2_DIR/config/hampod.conf.factory"
    
    if [ -f "$factory_config" ]; then
        if sudo cp "$factory_config" "$config_file" 2>/dev/null; then
            sudo chown $USER:$USER "$config_file" 2>/dev/null || true
            echo "  Restored $config_file from factory default."
        else
            print_error "Failed to restore $config_file from factory default."
        fi
    else
        echo "  Notice: $factory_config not found. Skipping config reset."
    fi
    
    print_success "System state reset successfully."
}

function cmd_backup_config() {
    print_header "HAMPOD CLI - Backup Config"
    local config_file="$SOFTWARE2_DIR/config/hampod.conf"
    local backup_dir="$SOFTWARE2_DIR/config/backups"
    
    if [ ! -f "$config_file" ]; then
        print_error "Config file not found: $config_file"
        echo "Please ensure you have run the system at least once."
        exit 1
    fi
    
    mkdir -p "$backup_dir" 2>/dev/null || sudo mkdir -p "$backup_dir"
    
    echo -n "Enter a short name/description for this backup (optional): "
    read -r backup_name
    
    local timestamp=$(date +"%Y%m%d_%H%M%S")
    local final_name="hampod_${timestamp}"
    
    if [ -n "$backup_name" ]; then
        # Replace spaces with underscores and sanitize slightly
        local sanitized_name=$(echo "$backup_name" | tr ' ' '_' | tr -cd '[:alnum:]_-')
        if [ -n "$sanitized_name" ]; then
            final_name="${final_name}_${sanitized_name}"
        fi
    fi
    
    final_name="${final_name}.conf"
    local backup_path="$backup_dir/$final_name"
    
    # We use cp with permissions preservation
    if cp "$config_file" "$backup_path" 2>/dev/null || sudo cp "$config_file" "$backup_path"; then
        sudo chown $USER:$USER "$backup_path" 2>/dev/null || true
        print_success "Config backed up to: $backup_path"
    else
        print_error "Failed to copy config file to backup path!"
        exit 1
    fi
}

function cmd_restore_config() {
    print_header "HAMPOD CLI - Restore Config"
    local config_file="$SOFTWARE2_DIR/config/hampod.conf"
    local backup_dir="$SOFTWARE2_DIR/config/backups"
    
    if [ ! -d "$backup_dir" ]; then
        print_error "No backups directory found: $backup_dir"
        exit 1
    fi
    
    # Get list of config files
    # Use nullglob to prevent returning literal string if empty
    shopt -s nullglob
    local backups=("$backup_dir"/*.conf)
    shopt -u nullglob
    
    if [ ${#backups[@]} -eq 0 ]; then
        print_error "No backups found in $backup_dir"
        exit 1
    fi
    
    echo "Available backups:"
    local i=1
    for b in "${backups[@]}"; do
        echo "  $i) $(basename "$b")"
        ((i++))
    done
    
    echo ""
    echo -n "Select a backup to restore (1-$((i-1))): "
    read -r selection
    
    if [[ ! "$selection" =~ ^[0-9]+$ ]] || [ "$selection" -lt 1 ] || [ "$selection" -gt "$((i-1))" ]; then
        print_error "Invalid selection."
        exit 1
    fi
    
    local selected_backup="${backups[$((selection-1))]}"
    
    # We use cp with permissions preservation
    if cp "$selected_backup" "$config_file" 2>/dev/null || sudo cp "$selected_backup" "$config_file"; then
        sudo chown $USER:$USER "$config_file" 2>/dev/null || true
        print_success "Config restored from: $selected_backup"
        echo "Note: You may need to restart HAMPOD for all changes to take effect."
    else
        print_error "Failed to restore config file!"
        exit 1
    fi
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
    backup-config)
        cmd_backup_config
        ;;
    restore-config)
        cmd_restore_config
        ;;
    monitor_mem|monitor_mem.sh)
        cmd_monitor_mem "$@"
        ;;
    *)
        print_error "Unknown command: $COMMAND"
        echo ""
        cmd_help
        exit 1
        ;;
esac
