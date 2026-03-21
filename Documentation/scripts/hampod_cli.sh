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
    echo "  update"
    echo "      Updates the HAMPOD codebase from GitHub and rebuilds firmware."
    echo ""
    echo "  clear-cache"
    echo "      Clears the TTS audio cache directory."
    echo ""
    echo "  reset [--yes|-y]"
    echo "      Performs a hard reset of system state, clearing logs and stale processes."
    echo "      Also resets the hampod.conf configuration file to factory defaults."
    echo "      --yes / -y: Skip confirmation prompt (for scripted use)."
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
    echo "  install [--verbose|-v] [--hard-reset]"
    echo "      Runs the HAMPOD installation script."
    echo "      --verbose / -v: Show detailed build output"
    echo "      --hard-reset: Overwrite hampod.conf with repo defaults"
    echo ""
    echo "  autostart [--enable|--disable|--status]"
    echo "      Configures HAMPOD to start automatically on boot."
    echo "      --enable:  Install and enable the service (default)"
    echo "      --disable: Disable and remove the service"
    echo "      --status:  Show current auto-start status"
    echo ""
    echo "  writeprotect [--enable|--disable|--status]"
    echo "      Manages SD card protection via overlay filesystem (read-only mode)."
    echo "      --enable:  Enable SD card protection (requires reboot)"
    echo "      --disable: Disable SD card protection (requires reboot)"
    echo "      --status:  Show current protection status (default)"
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
        exit 3
    fi
    
    # Pass all arguments passed to `hampod start` directly to `run_hampod.sh`
    "$run_script" "$@"
}

function cmd_update() {
    local run_script="$SCRIPT_DIR/update_hampod.sh"
    
    if [ ! -x "$run_script" ]; then
        print_error "Cannot find or execute $run_script."
        exit 3
    fi
    
    "$run_script" "$@"
}

function cmd_install() {
    local run_script="$SCRIPT_DIR/install_hampod.sh"
    
    if [ ! -x "$run_script" ]; then
        print_error "Cannot find or execute $run_script."
        exit 3
    fi
    
    "$run_script" "$@"
}

function cmd_autostart() {
    local run_script="$SCRIPT_DIR/hampod_on_powerup.sh"
    
    if [ ! -x "$run_script" ]; then
        print_error "Cannot find or execute $run_script."
        exit 3
    fi
    
    "$run_script" "$@"
}

function cmd_writeprotect() {
    local run_script="$SCRIPT_DIR/power_down_protection.sh"
    
    if [ ! -x "$run_script" ]; then
        print_error "Cannot find or execute $run_script."
        exit 3
    fi
    
    "$run_script" "$@"
}

function cmd_monitor_mem() {
    local run_script="$SCRIPT_DIR/monitor_mem.sh"
    
    if [ ! -x "$run_script" ]; then
        print_error "Cannot find or execute $run_script."
        exit 3
    fi
    
    "$run_script" "$@"
}

function cmd_clear_cache() {
    print_header "HAMPOD CLI - Clear Cache"
    local cleared=false

    # The Firmware TTS cache is stored in the HOME of the user running firmware.elf.
    # Since run_hampod.sh uses sudo, this is located at /root/.cache/hampod/tts
    local root_cache="/root/.cache/hampod/tts"
    echo "Clearing root TTS cache at: $root_cache"
    if sudo rm -rf "$root_cache"/* 2>/dev/null; then
        cleared=true
    fi

    # Also clear the non-root user cache, in case the system was tested without sudo
    local user_cache="$HOME/.cache/hampod/tts"
    if [ -d "$user_cache" ]; then
        echo "Clearing user TTS cache at: $user_cache"
        rm -rf "$user_cache"/* 2>/dev/null && cleared=true
    fi

    if [ "$cleared" = true ]; then
        print_success "TTS cache cleared."
    else
        print_error "Failed to clear TTS cache or it is already empty."
        exit 3
    fi
}

function cmd_reset() {
    print_header "HAMPOD CLI - Hard Reset"
    echo -e "${YELLOW}Warning: This will forcefully stop HAMPOD and clear all temporary system state, including configuration.${NC}"
    echo ""
    
    # Allow --yes / -y to skip confirmation (for scripted/non-interactive use)
    local skip_confirm=false
    for arg in "$@"; do
        case "$arg" in
            --yes|-y) skip_confirm=true ;;
        esac
    done

    if [ "$skip_confirm" = false ]; then
        echo -n "Are you sure you want to continue? [y/N] "
        read -r confirm
        if [[ ! "$confirm" =~ ^[Yy]$ ]]; then
            echo "Reset cancelled."
            exit 0
        fi
        echo ""
    fi
    echo "Stopping processes..."
    # We redirect both stdout and stderr to /dev/null to prevent messy "Killed" outputs
    sudo killall -9 firmware.elf > /dev/null 2>&1 || true
    
    # Carefully kill 'hampod' processes BUT exclude our own PID so the script doesn't commit suicide
    for pid in $(pgrep -x hampod 2>/dev/null); do
        if [ "$pid" != "$$" ] && [ "$pid" != "$PPID" ]; then
            sudo kill -9 "$pid" > /dev/null 2>&1 || true
        fi
    done
    
    sudo killall -9 phase0_test > /dev/null 2>&1 || true
    sudo killall -9 piper > /dev/null 2>&1 || true
    sudo killall -9 aplay > /dev/null 2>&1 || true
    
    echo "Clearing IPC pipes..."
    sudo rm -f "$FIRMWARE_DIR/Firmware_i" "$FIRMWARE_DIR/Firmware_o" 2>/dev/null || true
    sudo rm -f "$FIRMWARE_DIR/Speaker_i" "$FIRMWARE_DIR/Speaker_o" 2>/dev/null || true
    sudo rm -f "$FIRMWARE_DIR/Keypad_i" "$FIRMWARE_DIR/Keypad_o" 2>/dev/null || true
    
    echo "Clearing temporary logs..."
    sudo rm -f /tmp/firmware.log /tmp/hampod_output.txt /tmp/hampod_debug.log 2>/dev/null || true
    
    echo "Resetting configuration..."
    local config_file="$SOFTWARE2_DIR/config/hampod.conf"
    local factory_config="$SOFTWARE2_DIR/config/hampod.conf.factory"
    
    if [ -f "$factory_config" ] && [ -f "$config_file" ]; then
        # Check if the current config is different from the factory default
        if ! diff -q "$config_file" "$factory_config" > /dev/null 2>&1; then
            echo "  Differences detected in config. Creating automatic backup..."
            local backup_dir="$SOFTWARE2_DIR/config/backups"
            mkdir -p "$backup_dir" 2>/dev/null || sudo mkdir -p "$backup_dir"
            local timestamp=$(date +"%Y%m%d_%H%M%S")
            local backup_path="$backup_dir/hampod_${timestamp}_pre_reset_autobackup.conf"
            
            if cp "$config_file" "$backup_path" 2>/dev/null || sudo cp "$config_file" "$backup_path"; then
                sudo chown $USER:$USER "$backup_path" 2>/dev/null || true
                echo "  Auto-backup saved to: $backup_path"
            else
                print_error "Failed to create auto-backup!"
            fi
        fi
        
        if sudo cp "$factory_config" "$config_file" 2>/dev/null; then
            sudo chown $USER:$USER "$config_file" 2>/dev/null || true
            echo "  Restored $config_file from factory default."
        else
            print_error "Failed to restore $config_file from factory default."
        fi
    elif [ -f "$factory_config" ]; then
        # If there's no live config but there is a factory config, just create it
        if sudo cp "$factory_config" "$config_file" 2>/dev/null; then
            sudo chown $USER:$USER "$config_file" 2>/dev/null || true
            echo "  Created $config_file from factory default."
        else
            print_error "Failed to create $config_file from factory default."
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
        exit 3
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
        exit 3
    fi
}

function cmd_restore_config() {
    print_header "HAMPOD CLI - Restore Config"
    local config_file="$SOFTWARE2_DIR/config/hampod.conf"
    local backup_dir="$SOFTWARE2_DIR/config/backups"
    
    if [ ! -d "$backup_dir" ]; then
        print_error "No backups directory found: $backup_dir"
        exit 3
    fi
    
    # Get list of config files
    # Use nullglob to prevent returning literal string if empty
    shopt -s nullglob
    local backups=("$backup_dir"/*.conf)
    shopt -u nullglob
    
    if [ ${#backups[@]} -eq 0 ]; then
        print_error "No backups found in $backup_dir"
        exit 3
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
        exit 2
    fi
    
    local selected_backup="${backups[$((selection-1))]}"
    
    # We use cp with permissions preservation
    if cp "$selected_backup" "$config_file" 2>/dev/null || sudo cp "$selected_backup" "$config_file"; then
        sudo chown $USER:$USER "$config_file" 2>/dev/null || true
        
        # Gracefully handle version skew by filling in missing keys from factory default
        local factory_config="$SOFTWARE2_DIR/config/hampod.conf.factory"
        if [ -f "$factory_config" ]; then
            python3 -c "
import configparser
f, b = configparser.ConfigParser(), configparser.ConfigParser()
f.read('$factory_config')
b.read('$config_file')
for sec in f.sections():
    if not b.has_section(sec): b.add_section(sec)
    for k, v in f.items(sec):
        if not b.has_option(sec, k): b.set(sec, k, v)
with open('$config_file', 'w') as out: b.write(out)
" 2>/dev/null || true
        fi
        
        print_success "Config restored from: $selected_backup"
        echo "Note: You may need to restart HAMPOD for all changes to take effect."
    else
        print_error "Failed to restore config file!"
        exit 3
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
    update)
        cmd_update "$@"
        ;;
    install)
        cmd_install "$@"
        ;;
    autostart)
        cmd_autostart "$@"
        ;;
    writeprotect)
        cmd_writeprotect "$@"
        ;;
    help)
        cmd_help
        ;;
    clear-cache)
        cmd_clear_cache
        ;;
    reset)
        cmd_reset "$@"
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
        exit 2
        ;;
esac
